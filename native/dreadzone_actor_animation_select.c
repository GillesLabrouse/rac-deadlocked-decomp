#include "dreadzone_actor_animation_select.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

enum { SELECT_MAX_VIEWS=64, SELECT_MAX_BYTES=32*1024*1024 };
static u32 select_word(const u8 *p) {
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}
static u16 select_half(const u8 *p) { return (u16)((u32)p[0]|((u32)p[1]<<8)); }
static s32 select_signed_half(const u8 *p) { return (s32)(select_half(p)^0x8000U)-0x8000; }
static void select_store(u8 *p,u32 v) {
    p[0]=(u8)v;p[1]=(u8)(v>>8);p[2]=(u8)(v>>16);p[3]=(u8)(v>>24);
}
static void select_store_half(u8 *p,u16 v) { p[0]=(u8)v;p[1]=(u8)(v>>8); }
static f32 select_float(u32 bits) { f32 value;memcpy(&value,&bits,4);return value; }
static u32 select_bits(f32 value) { u32 bits;memcpy(&bits,&value,4);return bits; }
static f32 select_chop(double value) {
    f32 result=(f32)value;
    if(fabs((double)result)>fabs(value)) result=select_float(select_bits(result)-1U);
    return result;
}
static int select_loop(s32 clip) {
    switch(clip) {
    case 0:case 6:case 7:case 5:case 8:case 0xB:case 0x13:case 0x14:
    case 0x17:case 0x18:case 0x19:case 0x1C:case 0x1D:case 0x1E:
    case 0x69:case 0x6A:case 0x6B:case 0x6C:case 0x20:case 0x6E:
    case 0x6F:case 0x24:case 0x25:case 0x26:case 0x32:case 0x36:
    case 0x39:case 0x47:case 0x49:case 0x4A:case 0x4C:case 0x44:
    case 0x40:case 0x4E:case 0x42:case 0x56:case 0x57:case 0x65:
    case 0x5B:case 0x5C:case 0x5D:case 0x5E:case 0x5F:case 0x41:
    case 0x60:case 0x61:return 1;
    default:return 0;
    }
}
static int select_allocate(DreadzoneAnimationContext *memory,u8 *actor,u32 actor_guest,u32 *node_guest) {
    u32 slot;
    u8 *occupancy,*owner;
    *node_guest=0;
    if(actor[0x31]==0 || (actor[0x20]&0x80)) return 1;
    for(slot=0;slot<32;++slot) {
        occupancy=dreadzone_animation_translate(memory,0x28EF40+slot,1);
        if(!occupancy) return DREADZONE_ANIMATION_SELECT_INVALID;
        if(*occupancy==0) {
            owner=dreadzone_animation_translate(memory,0x28EF80+slot*4,4);
            if(!owner) return DREADZONE_ANIMATION_SELECT_INVALID;
            *occupancy=1;select_store(owner,actor_guest);
            *node_guest=0x28EB40+slot*32;return 1;
        }
    }
    return 1;
}
static int select_auxiliary(DreadzoneAnimationContext *memory,u8 *player) {
    u32 first_guest,second_guest;
    u8 *first,*second;
    s32 actual_first_output;
    if(select_word(player+0x2404)!=2) return 1;
    first_guest=select_word(player+0x23E0);
    second_guest=select_word(player+0x23E4);
    if(!first_guest || !second_guest) return 1;
    /* Complete actual688B78 writes only the first local output and returns0.
     * Its second local output is never consumed on that genuine return path. */
    actual_first_output=1;
    if(first_guest>0xFFFFFFFFU-0x54U) return DREADZONE_ANIMATION_SELECT_INVALID;
    first=dreadzone_animation_translate(memory,first_guest+0x54,1);
    if(!first) return DREADZONE_ANIMATION_SELECT_INVALID;
    if(*first!=(u8)actual_first_output) return DREADZONE_ANIMATION_SELECT_NEEDS_5919B8;
    if(second_guest>0xFFFFFFFFU-0x54U) return DREADZONE_ANIMATION_SELECT_INVALID;
    second=dreadzone_animation_translate(memory,second_guest+0x54,1);
    if(!second) return DREADZONE_ANIMATION_SELECT_INVALID;
    if(*second!=(u8)actual_first_output) return DREADZONE_ANIMATION_SELECT_NEEDS_5919B8;
    return 1;
}
static int select_cleanup(DreadzoneAnimationContext *memory,u8 *player) {
    u32 actor_guest,index;
    u8 *actor;
    int alternate=player[0x265A]!=0;
    select_store(player+0x7F4,0);player[0x265A]=0;player[0x2658]=0;
    if(!alternate || select_word(player+0x7E8)!=0) {
        for(index=0;index<2;++index)
            if(select_word(player+0x7E8+index*4)!=0) return DREADZONE_ANIMATION_SELECT_NEEDS_58AA90;
        if(alternate) select_store(player+0x7F4,0);
    }
    if(select_word(player+0x25CC)==0) {
        actor_guest=select_word(player+0x2EF0);
        if(actor_guest>0xFFFFFFFFU-0x54U) return DREADZONE_ANIMATION_SELECT_INVALID;
        actor=dreadzone_animation_translate(memory,actor_guest+0x54,1);
        if(!actor) return DREADZONE_ANIMATION_SELECT_INVALID;
        if(*actor==select_word(player+0x2DD4)) {
            if(select_word(player+0x2318)==3) return DREADZONE_ANIMATION_SELECT_NEEDS_RECURSION;
            return DREADZONE_ANIMATION_SELECT_NEEDS_669250;
        }
    }
    if(alternate) { player[0x2658]=0;player[0x265A]=0; }
    return 1;
}
static int select_run(DreadzoneAnimationContext *memory,const DreadzoneAnimationSelectRequest *input) {
    u8 *player,*actor,*node,*table,*definition,*other,*duration_pointer;
    u32 actor_guest,node_guest,class_guest,table_guest,definition_guest,address;
    f32 blend=select_float(input->blend_duration_bits),decay,weight;
    s32 index;
    int status;
    if(!input->player_guest || (input->player_guest&3) || input->clip<0 || input->clip>255 ||
       input->frame<0 || input->frame>16777216 || !isfinite(blend)) return DREADZONE_ANIMATION_SELECT_UNSUPPORTED;
    player=dreadzone_animation_translate(memory,input->player_guest,0x31F0);
    if(!player) return DREADZONE_ANIMATION_SELECT_INVALID;
    if(blend<0) {
        index=(s32)select_word(player+0x650);
        if(index<0 || index>(s32)((0xFFFFFFFFU-0x220798U)/4U)) return DREADZONE_ANIMATION_SELECT_INVALID;
        address=0x220798U+(u32)index*4U;
        table=dreadzone_animation_translate(memory,address,4);
        if(!table) return DREADZONE_ANIMATION_SELECT_INVALID;
        index=(s32)select_word(table);
        if(index<-16777216 || index>16777216) return DREADZONE_ANIMATION_SELECT_UNSUPPORTED;
        blend=(f32)index;
    }
    actor_guest=select_word(player+0x2EF0);
    actor=dreadzone_animation_translate(memory,actor_guest,256);
    if(!actor || !actor_guest || (actor_guest&3) ||
       dreadzone_animation_overlap(actor_guest,256,input->player_guest,0x31F0) ||
       dreadzone_animation_overlap(actor_guest,256,0x28EB40,0x4C0) ||
       dreadzone_animation_overlap(input->player_guest,0x31F0,0x28EB40,0x4C0))
        return DREADZONE_ANIMATION_SELECT_INVALID;
    status=select_allocate(memory,actor,actor_guest,&node_guest);
    if(status<0) return status;
    if(node_guest) {
        if(!(blend>0) || blend<0.0009765625f || blend>1048576.f) return DREADZONE_ANIMATION_SELECT_UNSUPPORTED;
        node=dreadzone_animation_translate(memory,node_guest,32);
        if(!node) return DREADZONE_ANIMATION_SELECT_INVALID;
        select_store(node+0x10,select_word(actor+0x50));select_store(actor+0x50,node_guest);
        decay=select_chop(1.0/(double)blend);weight=select_chop(1.0-(double)decay);
        select_store(node,select_word(actor+0x40));select_store(node+8,0);
        select_store(node+4,select_word(actor+0x44));select_store_half(node+0xC,select_half(actor+0x4C));
        node[0x14]=actor[0x54];node[0x17]=255;node[0x15]=actor[0x55];
        select_store(node+0x18,select_bits(weight));select_store(node+0x1C,select_bits(decay));node[0x16]=255;
    }
    class_guest=select_word(actor+0x24);
    if((class_guest&3) || class_guest>0xFFFFFFFFU-0x48U-(u32)input->clip*4U) return DREADZONE_ANIMATION_SELECT_INVALID;
    table_guest=class_guest+0x48+(u32)input->clip*4;
    table=dreadzone_animation_translate(memory,class_guest+0xC,1);
    if(!table) return DREADZONE_ANIMATION_SELECT_INVALID;
    if((u32)input->clip>=*table) return DREADZONE_ANIMATION_SELECT_UNSUPPORTED;
    actor[0x54]=(u8)input->clip;
    table=dreadzone_animation_translate(memory,table_guest,4);
    if(!table) return DREADZONE_ANIMATION_SELECT_INVALID;
    definition_guest=select_word(table);
    definition=dreadzone_animation_translate(memory,definition_guest,0x13);
    if(!definition || (definition_guest&1)) return DREADZONE_ANIMATION_SELECT_INVALID;
    if((u32)input->frame>=definition[0x10]) return DREADZONE_ANIMATION_SELECT_UNSUPPORTED;
    if(definition_guest>0xFFFFFFFFU-0x21U-(u32)input->frame*4U) return DREADZONE_ANIMATION_SELECT_INVALID;
    select_store(actor+0x44,select_bits((f32)input->frame));select_store(actor+0x40,definition_guest);select_store(actor+0x48,0x3F800000);
    duration_pointer=dreadzone_animation_translate(memory,definition_guest+0x20+(u32)input->frame*4,2);
    if(!duration_pointer) return DREADZONE_ANIMATION_SELECT_INVALID;
    select_store_half(actor+0x4C,select_half(duration_pointer));select_store(player+0x640,0x3F800000);
    select_store(player+0x644,(u32)select_signed_half(actor+0x4C));
    address=select_word(player+0x2EF0);
    if(address>0xFFFFFFFFU-0x55U) return DREADZONE_ANIMATION_SELECT_INVALID;
    other=dreadzone_animation_translate(memory,address+0x55,1);
    if(!other) return DREADZONE_ANIMATION_SELECT_INVALID;
    if(select_loop(input->clip)) { *other&=0xEF;select_store(player+0x648,select_word(player+0x648)&~0x10U); }
    else { *other|=0x10;select_store(player+0x648,select_word(player+0x648)|0x10U); }
    if((u32)(input->frame-1)<11U && !(input->flags&8)) return DREADZONE_ANIMATION_SELECT_NEEDS_5A36B0;
    class_guest=select_word(actor+0x24);
    if((class_guest&3) || class_guest>0xFFFFFFFFU-0x48U-(u32)input->clip*4U) return DREADZONE_ANIMATION_SELECT_INVALID;
    table_guest=class_guest+0x48+(u32)input->clip*4;
    table=dreadzone_animation_translate(memory,table_guest,4);
    if(!table) return DREADZONE_ANIMATION_SELECT_INVALID;
    definition_guest=select_word(table);
    if(definition_guest>0xFFFFFFFFU-0x11U) return DREADZONE_ANIMATION_SELECT_INVALID;
    definition=dreadzone_animation_translate(memory,definition_guest+0x11,1);
    if(!definition) return DREADZONE_ANIMATION_SELECT_INVALID;
    actor[0x6D]=*definition;
    definition_guest=select_word(table);
    if(definition_guest>0xFFFFFFFFU-0x12U) return DREADZONE_ANIMATION_SELECT_INVALID;
    definition=dreadzone_animation_translate(memory,definition_guest+0x12,1);
    if(!definition) return DREADZONE_ANIMATION_SELECT_INVALID;
    actor[0x6C]=*definition;
    select_store_half(actor+0x36,(u16)((select_half(actor+0x36)&0xFFFBU)|((input->flags&0x10)?4:0)));
    if(select_signed_half(player+0x2E2C)==0) {
        status=select_auxiliary(memory,player);
        if(status<0) return status;
    }
    if(player[0x2658]==0 && input->clip!=(s32)select_word(player+0x2DD4)) {
        status=select_cleanup(memory,player);
        if(status<0) return status;
    }
    return DREADZONE_ANIMATION_SELECT_APPROXIMATE;
}
int dreadzone_actor_animation_select(const DreadzoneAnimationSelectRequest *request) {
    DreadzoneAnimationSelectRequest input;
    DreadzoneAnimationContext memory;
    DreadzoneAnimationMemory views[SELECT_MAX_VIEWS],original[SELECT_MAX_VIEWS];
    u8 *shadow;
    u64 bytes=0;
    u32 i,offset=0;
    int status;
    if(!request || (uintptr_t)request>UINTPTR_MAX-sizeof(*request)) return DREADZONE_ANIMATION_SELECT_INVALID;
    input=*request;
    if(!dreadzone_animation_validate(input.memory)) return DREADZONE_ANIMATION_SELECT_INVALID;
    memory=*input.memory;
    for(i=0;i<memory.view_count;++i) {
        original[i]=memory.views[i];bytes+=original[i].bytes;
        if(bytes>SELECT_MAX_BYTES) return DREADZONE_ANIMATION_SELECT_UNSUPPORTED;
    }
    if(!bytes) return DREADZONE_ANIMATION_SELECT_INVALID;
    shadow=(u8 *)malloc((size_t)bytes);
    if(!shadow) return DREADZONE_ANIMATION_SELECT_NO_MEMORY;
    for(i=0;i<memory.view_count;++i) {
        views[i]=original[i];views[i].storage=shadow+offset;
        memcpy(views[i].storage,original[i].storage,original[i].bytes);offset+=original[i].bytes;
    }
    memory.views=views;status=select_run(&memory,&input);
    if(status==DREADZONE_ANIMATION_SELECT_APPROXIMATE)
        for(i=0;i<memory.view_count;++i)
            if(memcmp(original[i].storage,views[i].storage,original[i].bytes))
                memcpy(original[i].storage,views[i].storage,original[i].bytes);
    free(shadow);return status;
}
