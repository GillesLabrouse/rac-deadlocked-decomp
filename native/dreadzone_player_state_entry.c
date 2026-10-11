#include "dreadzone_player_state_entry.h"
#include "dreadzone_actor_animation_select.h"
#include "dreadzone_actor_direction.h"
#include "dreadzone_actor_velocity.h"
#include "dreadzone_actor_fields.h"
#include "game_random.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

enum { ENTRY_MAX_VIEWS=64,ENTRY_MAX_BYTES=32*1024*1024 };
static u32 entry_word(const u8 *p) {
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}
static s32 entry_half(const u8 *p) {
    return (s32)(((u32)p[0]|((u32)p[1]<<8))^0x8000U)-0x8000;
}
static void entry_store(u8 *p,u32 value) {
    p[0]=(u8)value;p[1]=(u8)(value>>8);p[2]=(u8)(value>>16);p[3]=(u8)(value>>24);
}
static f32 entry_float(const u8 *p) { f32 value;memcpy(&value,p,4);return value; }
static u32 entry_bits(f32 value) {u32 bits;memcpy(&bits,&value,4);return bits;}
static int entry_normal(f32 value) {
    u32 bits=entry_bits(value)&0x7FFFFFFFU;
    return !bits || (bits>=0x00800000U && bits<0x7F800000U);
}
static int entry_host_overlap(const void *a,size_t an,const void *b,size_t bn) {
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
    return x>=y ? x-y<bn : y-x<an;
}
static int entry_read(const DreadzoneAnimationContext *memory,u32 address,u32 *value) {
    u8 *p;
    if(address&3) return 0;
    p=dreadzone_animation_translate(memory,address,4);
    if(!p) return 0;
    *value=entry_word(p);return 1;
}
static int entry_gate(DreadzoneAnimationContext *memory,u8 *player,s32 target,s32 option,s32 *allowed) {
    s32 state=(s32)entry_word(player+0x25CC),previous,base;
    int special=state==57 || state==142;
    u32 predicate;
    if(state==153) {
        previous=(s32)entry_word(player+0x25D8);
        special=previous==57 || previous==142;
    }
    if(special && target!=122 && target!=126 && target!=123 && target!=118 && target!=148) special=0;
    if(!entry_read(memory,0x21DDB4,&predicate)) return DREADZONE_PLAYER_ENTRY_INVALID;
    *allowed=0;
    if(!special && entry_word(player+0x25D4)==20 && (target!=99 || option!=2)) return 1;
    if((predicate==2 || predicate==6) && option!=2) return 1;
    if(special) { *allowed=1;return 1; }
    if(!entry_normal(entry_float(player+0x2E20))) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
    if(entry_float(player+0x2E20)!=0.f) { *allowed=1;return 1; }
    base=(s32)entry_word(player+0x25D4);
    *allowed=base!=2 && base!=7;
    if(!*allowed) *allowed=target==6 || target==57 || target==118 || target==122 ||
        target==123 || target==126 || target==106 || target==127 || target==129 || target==153 || target==148;
    return 1;
}
static int entry_history(u8 *player) {
    u32 count=entry_word(player+0x264C),state,base,timer,previous,previous_base;
    s32 i;
    if(count>8) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
    for(i=(s32)(count<8?count:7)-1;i>=0;--i) {
        u32 a=entry_word(player+0x25EC+i*4),b=entry_word(player+0x260C+i*4),c=entry_word(player+0x262C+i*4);
        entry_store(player+0x25F0+i*4,a);entry_store(player+0x2610+i*4,b);entry_store(player+0x2630+i*4,c);
    }
    state=entry_word(player+0x25CC);count=entry_word(player+0x264C)+1;
    base=entry_word(player+0x25D4);timer=entry_word(player+0x370);
    previous=entry_word(player+0x25D8);previous_base=entry_word(player+0x25DC);
    entry_store(player+0x25DC,base);entry_store(player+0x264C,count<9?count:8);
    entry_store(player+0x25E4,previous);entry_store(player+0x25E8,previous_base);
    entry_store(player+0x25E0,timer);entry_store(player+0x25D8,state);
    entry_store(player+0x25EC,state);entry_store(player+0x260C,base);entry_store(player+0x262C,timer);
    return 1;
}
static int entry_children(u8 *player) {
    s32 i,count;
    if(!entry_word(player+0x217C)) return 1;
    count=(s32)entry_word(player+0x2178);
    /* Bounded domain: the ten slots preceding the adjacent count field. */
    if(count>10) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
    for(i=0;i<count;++i) if(entry_word(player+0x2150+i*4)) return DREADZONE_PLAYER_ENTRY_NEEDS_586E80;
    entry_store(player+0x217C,0);return 1;
}
static int entry_input_clear(DreadzoneAnimationContext *memory,u8 *player) {
    u32 address=entry_word(player+0x2F00),row=0;
    u8 *input,*byte;
    s32 i;
    if(address) {
        if(address>0xFFFFFFFFU-0x558U) return DREADZONE_PLAYER_ENTRY_INVALID;
        input=dreadzone_animation_translate(memory,address+0x558,1);
        if(!input) return DREADZONE_PLAYER_ENTRY_INVALID;
        if(*input==0xFF) return 1;
        row=*input;input=dreadzone_animation_translate(memory,address+0x54C,1);
        if(!input) return DREADZONE_PLAYER_ENTRY_INVALID;
        row+=(u32)*input*4;
    }
    for(i=7;i>=0;--i) {
        byte=dreadzone_animation_translate(memory,0x3B5050+row*128+(u32)i*16,1);
        if(!byte) return DREADZONE_PLAYER_ENTRY_INVALID;
        *byte=0;
    }
    return 1;
}
static int entry_low_health(u8 *player,s32 *result) {
    f32 health=entry_float(player+0x2E20),maximum=entry_float(player+0x2F24),ratio;
    double exact;
    if(!entry_normal(health) || !entry_normal(maximum) || maximum<=0.f ||
       fabsf(health)>1048576.f || maximum>1048576.f || maximum<0.0009765625f)
        return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
    exact=(double)health/(double)maximum;ratio=(f32)exact;
    if(fabs((double)ratio)>fabs(exact)) {u32 bits=entry_bits(ratio)-1U;memcpy(&ratio,&bits,4);}
    if(!entry_normal(ratio)) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
    *result=ratio<=0.20000000298023223876953125f;return 1;
}
static int entry_clip(DreadzoneAnimationContext *memory,u8 *player,s32 option,s32 *clip) {
    s32 weapon,status,low;
    u32 mode;
    if(option && player[0x2658] && !player[0x2660]) {
        weapon=entry_word(player+0x2314)==2?(s32)entry_word(player+0x2318):-1;
        if(weapon==-1) { *clip=0;return 1; }
        return DREADZONE_PLAYER_ENTRY_NEEDS_6BF200;
    }
    status=entry_low_health(player,&low);if(status<0) return status;
    if(!entry_read(memory,0x21E824,&mode)) return DREADZONE_PLAYER_ENTRY_INVALID;
    if(low && mode!=1) { *clip=101;return 1; }
    weapon=(s32)entry_word(player+0x2318);
    if(weapon==5) *clip=66;
    else if(weapon==2) *clip=64;
    else if(weapon==4 || weapon==10) *clip=68;
    else if((u32)weapon-6U<2U || weapon==3 || weapon==8 || weapon==9 || weapon==13) *clip=78;
    else *clip=0;
    return 1;
}
static int entry_select(DreadzoneAnimationContext *memory,u32 player,s32 clip,s32 frame,f32 blend) {
    DreadzoneAnimationSelectRequest request={memory,player,clip,frame,0,0};
    int status;
    request.blend_duration_bits=entry_bits(blend);
    status=dreadzone_actor_animation_select(&request);
    return status<0 ? DREADZONE_PLAYER_ENTRY_SELECT_FAILURE_BASE+status : 1;
}
static int entry_query(DreadzoneAnimationContext *memory,s32 handle,u32 *address) {
    u32 base,index=(u32)handle&63U;
    u8 *record;
    if(!entry_read(memory,0x21F710,&base) || base!=0x1D24C0) return DREADZONE_PLAYER_ENTRY_INVALID;
    *address=0;if(index==63) return 1;
    record=dreadzone_animation_translate(memory,base+index*128,128);
    if(!record) return DREADZONE_PLAYER_ENTRY_INVALID;
    if((s32)(((u32)handle&0xFFFFU)>>6)==entry_half(record+0x58)) *address=base+index*128;
    return 1;
}
static int entry_release_owned(DreadzoneAnimationContext *memory,u8 *player) {
    u32 i,address,owner;
    u8 *slot,*record;
    int status;
    for(i=0;i<9;++i) {
        slot=player+0x2D30+i*4;owner=entry_word(slot+0x24);
        if(!owner) owner=entry_word(player+0x2EF0);
        status=entry_query(memory,entry_half(slot),&address);if(status<0) return status;
        if(!address) continue;
        record=dreadzone_animation_translate(memory,address,128);
        if(owner!=entry_word(record+0x1C) || !record[0x10]) continue;
        /* Actual6268A8 rereads LW handle, then its release truncates to LH. */
        status=entry_query(memory,(s32)((entry_word(slot)&0xFFFFU)^0x8000U)-0x8000,&address);
        if(status<0) return status;
        if(address) {
            record=dreadzone_animation_translate(memory,address,128);
            if(record[0x10]==7) {record[0x10]=0;entry_store(record+0x1C,0);entry_store(record+0x40,0);}
            else if(record[0x10] && record[0x10]!=6) record[0x10]=4;
        }
        entry_store(slot,0xFFFFFFFFU);
    }
    return 1;
}
static int entry_cleanup(DreadzoneAnimationContext *memory,u32 player_guest,u8 *player) {
    int alternate=player[0x265A]!=0,status;
    u32 i,actor_guest;
    u8 *actor;
    s32 clip;
    entry_store(player+0x7F4,0);player[0x265A]=0;player[0x2658]=0;
    if(!alternate || entry_word(player+0x7E8)) {
        for(i=0;i<2;++i) if(entry_word(player+0x7E8+i*4)) return DREADZONE_PLAYER_ENTRY_NEEDS_58AA90;
        if(alternate) entry_store(player+0x7F4,0);
    }
    if(!entry_word(player+0x25CC)) {
        actor_guest=entry_word(player+0x2EF0);
        if(actor_guest>0xFFFFFFFFU-0x54U) return DREADZONE_PLAYER_ENTRY_INVALID;
        actor=dreadzone_animation_translate(memory,actor_guest+0x54,1);
        if(!actor) return DREADZONE_PLAYER_ENTRY_INVALID;
        if(*actor==entry_word(player+0x2DD4)) {
            if(entry_word(player+0x2318)==3) {
                status=entry_select(memory,player_guest,62,0,-2.f);
                if(status<0) return status;
                if(!alternate) return DREADZONE_PLAYER_ENTRY_NEEDS_5919B8;
            } else {
                status=entry_clip(memory,player,0,&clip);if(status<0) return status;
                status=entry_select(memory,player_guest,clip,0,-2.f);if(status<0) return status;
            }
        }
    }
    if(alternate) {player[0x2658]=0;player[0x265A]=0;}
    return 1;
}
static int entry_direction(DreadzoneAnimationContext *memory,u32 player_guest,u8 *player) {
    DreadzoneActorInputContext context;
    DreadzoneActorInputMemory views[ENTRY_MAX_VIEWS];
    u32 i,n=0,base,end,player_end=player_guest+0x31F0;
    int status;
    for(i=0;i<memory->view_count;++i) {
        const DreadzoneAnimationMemory *view=memory->views+i;
        base=view->guest_base;
        /* Split the containing view: the recovered input context represents
         * the player separately and forbids duplicate host/guest backings. */
        if(player_guest>=base && player_guest-base<view->bytes) {
            end=player_guest-base;
            if(end) {
                if(n==ENTRY_MAX_VIEWS) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
                views[n++]=(DreadzoneActorInputMemory){view->storage,base,end,1};
            }
            end=view->bytes-(player_end-base);
            if(end) {
                if(n==ENTRY_MAX_VIEWS) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
                views[n++]=(DreadzoneActorInputMemory){view->storage+(player_end-base),player_end,end,1};
            }
        } else {
            if(n==ENTRY_MAX_VIEWS) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
            views[n++]=(DreadzoneActorInputMemory){view->storage,base,view->bytes,1};
        }
    }
    context=(DreadzoneActorInputContext){player,player_guest,0x31F0,views,n};
    status=dreadzone_actor_direction_0067C5F8(&context,0,1.f);
    return status<0?DREADZONE_PLAYER_ENTRY_DIRECTION_FAILURE_BASE+status:1;
}
static int entry_field_mode(DreadzoneAnimationContext *memory,u32 player_guest,u8 *player,s32 mode,u32 index) {
    DreadzoneActorFieldView view;
    u32 flag,client_guest=entry_word(player+0x2EFC);
    u8 *client,*modes;
    int status;
    if(index>=256 || !entry_read(memory,0x21E878,&flag)) return DREADZONE_PLAYER_ENTRY_INVALID;
    client=dreadzone_animation_translate(memory,client_guest,0x3A0);
    modes=dreadzone_animation_translate(memory,0x171DE0,index+1);
    if(!client || !modes) return DREADZONE_PLAYER_ENTRY_INVALID;
    view=(DreadzoneActorFieldView){player,0x31F0,player_guest,client,0x3A0,client_guest,modes,index+1,(s32)flag};
    status=dreadzone_actor_0068E3B0(&view,mode);
    return status<0?DREADZONE_PLAYER_ENTRY_FIELD_FAILURE_BASE+status:1;
}
static int entry_virtual_refusal(DreadzoneAnimationContext *memory,u8 *player) {
    u32 table=entry_word(player+0x14),callee;
    if(table>0xFFFFFFFFU-0x3FU || !entry_read(memory,table+0x3C,&callee)) return DREADZONE_PLAYER_ENTRY_INVALID;
    /* The actual target is read; no virtual result is captured or invented. */
    return DREADZONE_PLAYER_ENTRY_NEEDS_VIRTUAL_ENTRY;
}
static int entry_body(DreadzoneAnimationContext *memory,u32 player_guest,u8 *player,s32 target,s32 option) {
    u32 i,actor_guest,class_guest,address,frame_word;
    u8 *actor,*seed;
    f32 speed,angle,blend,first,second;
    s32 clip,low,blend_count,random_value;
    int status;
    if(target==0) {
        entry_store(player+0x25D4,0);entry_store(player+0x2DDC,0);
        if(entry_word(player+0x2318)==3) {
            status=entry_cleanup(memory,player_guest,player);if(status<0) return status;
        }
        seed=dreadzone_animation_translate(memory,0x21DC80,4);
        if(!seed) return DREADZONE_PLAYER_ENTRY_INVALID;
        {
            DeadlockedRandomState random={seed,4};
            if(deadlocked_random_0058FF38(&random,50,100,&random_value)!=DEADLOCKED_RANDOM_COMPLETED)
                return DREADZONE_PLAYER_ENTRY_INVALID;
        }
        entry_store(player+0x1870,(u32)random_value);
        entry_store(player+0x1864,0);entry_store(player+0x1868,0);
        for(i=0;i<4;++i) {entry_store(player+0x1884+i*16,0);entry_store(player+0x1888+i*16,0);}
        memcpy(player+0x1A0,player+0x160,16);
        if(player[0x438] && !entry_half(player+0x2FC)) return entry_virtual_refusal(memory,player);
        entry_store(player+0x1778,104);
        if(!option) return 1;
        status=entry_clip(memory,player,0,&clip);if(status<0) return status;
        return entry_select(memory,player_guest,clip,0,-1.f);
    }
    if(target==1) {
        entry_store(player+0x2DDC,4);entry_store(player+0x25D4,0);
        if(entry_word(player+0x25DC)==12) player[0x2ECD]=1;
        if(entry_word(player+0x1AE0)==1) speed=2.f;
        else {
            u32 state=entry_word(player+0x25CC);
            speed=(((state==4 || state==21) && entry_word(player+0x1AE0)==2) || player[0x2ECD])?.6f:1.15f;
        }
        entry_store(player+0x1AFC,entry_bits(speed));
        if(entry_word(player+0x25D4)==3) entry_store(player+0x1AFC,entry_bits(1.75f));
        if(player[0x2658]) return 1;
        status=entry_clip(memory,player,1,&clip);if(status<0) return status;
        actor_guest=entry_word(player+0x2EF0);
        if(actor_guest>0xFFFFFFFFU-0x54U) return DREADZONE_PLAYER_ENTRY_INVALID;
        actor=dreadzone_animation_translate(memory,actor_guest+0x54,1);
        if(!actor) return DREADZONE_PLAYER_ENTRY_INVALID;
        if(*actor==clip || !option) return 1;
        return entry_select(memory,player_guest,clip,0,18.f);
    }
    entry_store(player+0x25D4,1);entry_store(player+0x2DDC,0);
    if(entry_word(player+0x25DC)!=4 && entry_word(player+0x25E8)!=4) {
        VelocityVector velocity;
        if(player[0x2659]) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
        memcpy(&velocity,player+0x160,16);
        for(i=0;i<2;++i) if(!entry_normal(velocity.v[i]) || fabsf(velocity.v[i])>16.f ||
            (velocity.v[i]!=0.f && fabsf(velocity.v[i])<0x1p-40f)) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
        speed=func_L01_0068A920(player,&velocity);
        if(!entry_normal(speed)) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
        entry_store(player+0x1784,entry_bits(speed));
    }
    speed=entry_float(player+0x1784);
    if(!entry_normal(speed) || fabsf(speed)>16.f) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
    speed=dreadzone_velocity_min(speed,0.11666667461395263671875f);
    player[0x1C90]=0;player[0x1C8F]=0;
    memcpy(player+0x1C70,player+0x20,16);entry_store(player+0x1C88,entry_word(player+0xB8));
    entry_store(player+0x1784,entry_bits(speed));
    status=entry_direction(memory,player_guest,player);if(status<0) return status;
    first=entry_float(player+0xB8);second=entry_float(player+0x17A0);
    if(!entry_normal(first) || !entry_normal(second) || fabsf(first)>3.1415927410125732421875f ||
       fabsf(second)>3.1415927410125732421875f) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
    angle=func_L01_0055D910(first,second);
    if(angle>1.57079637050628662109375f && (entry_word(player+0x25DC)==4 ||
       entry_word(player+0x25D8)==3 || entry_word(player+0x25E4)==3)) entry_store(player+0x1784,0);
    entry_store(player+0x1778,104);
    if(player[0x438]) {
        if(!entry_half(player+0x2FC)) return entry_virtual_refusal(memory,player);
        first=entry_float(player+0x2CC);
        if(!entry_normal(first)) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
        if(first<0.300000011920928955078125f) return entry_virtual_refusal(memory,player);
    }
    if(player[0x2661] || entry_word(player+0x25CC)==114) {
        entry_store(player+0x25CC,114);actor_guest=entry_word(player+0x2EF0);
        if(actor_guest>0xFFFFFFFFU-0x27U || !entry_read(memory,actor_guest+0x24,&class_guest) ||
           class_guest>0xFFFFFFFFU-0x77U || !entry_read(memory,class_guest+0x74,&address))
            return DREADZONE_PLAYER_ENTRY_INVALID;
        return entry_select(memory,player_guest,address?11:6,0,8.f);
    }
    entry_store(player+0x25D0,0);player[0x1C8C]=0;player[0x1C8E]=0;
    if(!option) return 1;
    status=entry_low_health(player,&low);if(status<0) return status;
    blend_count=low?12:8;
    if(entry_word(player+0x25E4)==34 || entry_word(player+0x25D8)==34) blend_count=15;
    if(entry_word(player+0x25E4)==133 || entry_word(player+0x25D8)==133) blend_count=15;
    if(entry_word(player+0x25D8)==19 || entry_word(player+0x25D8)==145) blend_count=13;
    if(entry_word(player+0x25E4)==6 || entry_word(player+0x25E4)==44) blend_count=18;
    if(entry_word(player+0x25E8)==4 && player[0x1D0C]>=6) blend_count=13;
    first=entry_float(player+0x2E08);
    if(!entry_normal(first)) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
    if(first>0.180000007152557373046875f) entry_store(player+0x25D0,0xFFFFFFFFU);
    clip=(s32)(entry_word(player+0x25D0)+6U);
    address=0x220660+(entry_word(player+0x25D0)&0x80000000U?1U:entry_word(player+0x25D0))*4U;
    if(!entry_read(memory,address,&frame_word)) return DREADZONE_PLAYER_ENTRY_INVALID;
    blend=(f32)blend_count;
    status=entry_select(memory,player_guest,clip,(s32)frame_word,blend);if(status<0) return status;
    if(entry_word(player+0x2318)==2 && entry_word(player+0x25D0)+6U==7U) {
        if(!entry_read(memory,address,&frame_word)) return DREADZONE_PLAYER_ENTRY_INVALID;
        return entry_select(memory,player_guest,86,(s32)frame_word,blend);
    }
    return 1;
}
static int entry_suffix(DreadzoneAnimationContext *memory,u32 player_guest,u8 *player,u8 old_flag,u8 old_link_flag) {
    u32 index=entry_word(player+0x2F10),address,mode,base,state,condition;
    u8 *value,*ground;
    int status;
    f32 height,threshold;
    if(index>0xFFFFFFFFU-0x171DE0U) return DREADZONE_PLAYER_ENTRY_INVALID;
    value=dreadzone_animation_translate(memory,0x171DE0+index,1);
    if(!value) return DREADZONE_PLAYER_ENTRY_INVALID;
    mode=*value;
    if(mode==2) {
        if(entry_word(player+0x25D4)==20) {
            if(entry_word(player+0x1AE0)==2) {
                height=entry_float(player+0x1A94);
                if(!entry_normal(height)) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
                entry_store(player+0x1A94,entry_bits(dreadzone_velocity_min(height,1.5f)));
                status=entry_field_mode(memory,player_guest,player,1,index);if(status<0) return status;
            }
        } else if(entry_word(player+0x1AE0)!=2) {
            status=entry_field_mode(memory,player_guest,player,2,index);if(status<0) return status;
        }
    }
    if(old_flag && !player[0x2666]) entry_store(player+0x26BC,1);
    if(old_link_flag && !player[0x266C]) entry_store(player+0x26BC,1);
    if(entry_word(player+0x2EE8)) entry_store(player+0x26BC,0);
    if(!entry_half(player+0x2F8) && entry_word(player+0x2368)==18 && entry_word(player+0x26A8))
        entry_store(player+0x26C0,1);
    if(entry_word(player+0x25D4)==20 || entry_word(player+0x25CC)==122) {
        player[0x266C]=0;
        if(player[0x266D] && entry_word(player+0x25CC)!=153) {
            if(!entry_read(memory,0x1723B8,&condition)) return DREADZONE_PLAYER_ENTRY_INVALID;
            if(!condition) return DREADZONE_PLAYER_ENTRY_NEEDS_TRANSPORT;
        }
        status=entry_cleanup(memory,player_guest,player);if(status<0) return status;
        address=entry_word(player+0x2EC);player[0x2664]=1;
        if(address) {
            if(address>0xFFFFFFFFU-0xBDU) return DREADZONE_PLAYER_ENTRY_INVALID;
            ground=dreadzone_animation_translate(memory,address+0xBC,2);
            if(!ground) return DREADZONE_PLAYER_ENTRY_INVALID;
            if(entry_half(ground)==0x206B) return DREADZONE_PLAYER_ENTRY_NEEDS_GROUND_QUERY;
        }
        if(entry_word(player+0x2F54)) return DREADZONE_PLAYER_ENTRY_NEEDS_GROUND_QUERY;
        entry_store(player+0x2F54,0);
    }
    base=entry_word(player+0x25D4);state=entry_word(player+0x25CC);
    condition=base-17U<2U || state==106 || state==129 || state==117 || state==116;
    if(!condition || base==18) {
        height=entry_float(player+0x2E0);threshold=entry_float(player+0x2CD8);
        if(!entry_normal(height) || !entry_normal(threshold)) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
        if(height<threshold || !entry_word(player+0x25D4)) {
            address=entry_word(player+0x2EFC);
            if(address>0xFFFFFFFFU-0x463U) return DREADZONE_PLAYER_ENTRY_INVALID;
            value=dreadzone_animation_translate(memory,address+0x460,4);
            if(!value || ((address+0x460)&3)) return DREADZONE_PLAYER_ENTRY_INVALID;
            entry_store(value,0);
        }
    }
    if(entry_word(player+0x25D4)!=entry_word(player+0x25DC)) {
        status=entry_release_owned(memory,player);if(status<0) return status;
    }
    entry_store(player+0x370,0);entry_store(player+0x378,0);
    if(entry_word(player+0x25D4)!=entry_word(player+0x25DC)) {
        entry_store(player+0x374,0);
    }
    entry_store(player+0x37C,0);
    return 1;
}
static int entry_run(DreadzoneAnimationContext *memory,const DreadzonePlayerStateEntryRequest *request,s32 *result) {
    u8 *player,*body;
    u32 body_guest;
    u8 old_flag,old_link_flag;
    s32 allowed=1;
    int status;
    if(!request->player_guest || (request->player_guest&15) || request->player_guest>0xFFFFFFFFU-0x31F0U)
        return DREADZONE_PLAYER_ENTRY_INVALID;
    player=dreadzone_animation_translate(memory,request->player_guest,0x31F0);
    if(!player) return DREADZONE_PLAYER_ENTRY_INVALID;
    if(!request->force_option) {
        status=entry_gate(memory,player,request->target_state,request->selection_option,&allowed);
        if(status<0) return status;
        if(!allowed) {*result=0;return DREADZONE_PLAYER_ENTRY_REJECTED_EXACT;}
    }
    body_guest=entry_word(player+0x2EF0);
    body=dreadzone_animation_translate(memory,body_guest,256);
    if(!body || !body_guest || (body_guest&15) ||
       dreadzone_animation_overlap(request->player_guest,0x31F0,body_guest,256) ||
       dreadzone_animation_overlap(request->player_guest,0x31F0,0x28EB40,0x4C0) ||
       dreadzone_animation_overlap(request->player_guest,0x31F0,0x1D24C0,63*128) ||
       dreadzone_animation_overlap(body_guest,256,0x28EB40,0x4C0) ||
       dreadzone_animation_overlap(body_guest,256,0x1D24C0,63*128)) return DREADZONE_PLAYER_ENTRY_INVALID;
    /* Explicit adapter domain: real fixed global data and the maximum sparse
     * input rows must not be overwritten by a relocated player/body record.
     * This is a bounded refusal, not an inferred retail alias restriction. */
    if(dreadzone_animation_overlap(request->player_guest,0x31F0,0x21D000,0xB000) ||
       dreadzone_animation_overlap(body_guest,256,0x21D000,0xB000) ||
       dreadzone_animation_overlap(request->player_guest,0x31F0,0x3B5050,1276*128) ||
       dreadzone_animation_overlap(body_guest,256,0x3B5050,1276*128))
        return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
    status=entry_history(player);if(status<0) return status;
    /* The original also captures25D0 here. Selected states0/1/2 do not consume
     * that value later; the actual word is still read before its clear. */
    (void)entry_word(player+0x25D0);
    old_flag=player[0x2666];old_link_flag=player[0x266C];
    entry_store(player+0x25CC,(u32)request->target_state);entry_store(player+0x25D0,0);
    if(player[0x2662]==1) player[0x2662]=0;
    player[0x2664]=0;player[0x2ECE]=0;player[0x2ECF]=0;player[0x2ED0]=0;
    if(request->target_state!=1 && request->target_state!=29) player[0x2ECD]=0;
    if(request->target_state==102) entry_store(player+0x1C34,0);
    body_guest=entry_word(player+0x2EF0);
    if(body_guest>0xFFFFFFFFU-0x9FU) return DREADZONE_PLAYER_ENTRY_INVALID;
    body=dreadzone_animation_translate(memory,body_guest+0x9C,4);
    if(!body) return DREADZONE_PLAYER_ENTRY_INVALID;
    entry_store(body,0);player[0x2672]=0;
    player[0x2666]=0;player[0x2669]=0;player[0x2663]=0;
    if(entry_word(player+0x2F0)) player[0x266C]=0;
    status=entry_children(player);if(status<0) return status;
    if(!entry_half(player+0x3BE)) {
        status=entry_input_clear(memory,player);if(status<0) return status;
    }
    entry_store(player+0x1778,0);
    if(request->target_state<0 || request->target_state>2) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
    status=entry_body(memory,request->player_guest,player,request->target_state,request->selection_option);
    if(status<0) return status;
    status=entry_suffix(memory,request->player_guest,player,old_flag,old_link_flag);
    if(status<0) return status;
    *result=1;return DREADZONE_PLAYER_ENTRY_APPROXIMATE;
}
int dreadzone_player_state_entry(const DreadzonePlayerStateEntryRequest *request,DreadzonePlayerStateEntryOutput *output) {
    DreadzonePlayerStateEntryRequest input;
    DreadzoneAnimationContext memory;
    DreadzoneAnimationMemory views[ENTRY_MAX_VIEWS],original[ENTRY_MAX_VIEWS];
    u64 total=0;
    u32 i,offset=0;
    u8 *shadow;
    s32 result=0;
    int status;
    if(!request || !output || (uintptr_t)request>UINTPTR_MAX-sizeof(*request) ||
       (uintptr_t)output>UINTPTR_MAX-sizeof(*output)) return DREADZONE_PLAYER_ENTRY_INVALID;
    input=*request;
    if(!dreadzone_animation_validate(input.memory)) return DREADZONE_PLAYER_ENTRY_INVALID;
    memory=*input.memory;
    if(entry_host_overlap(output,sizeof(*output),input.memory,sizeof(*input.memory)) ||
       (memory.view_count && entry_host_overlap(output,sizeof(*output),memory.views,memory.view_count*sizeof(*memory.views))))
        return DREADZONE_PLAYER_ENTRY_INVALID;
    for(i=0;i<memory.view_count;++i) {
        original[i]=memory.views[i];total+=original[i].bytes;
        if(entry_host_overlap(output,sizeof(*output),original[i].storage,original[i].bytes)) return DREADZONE_PLAYER_ENTRY_INVALID;
        if(total>ENTRY_MAX_BYTES) return DREADZONE_PLAYER_ENTRY_UNSUPPORTED;
    }
    if(!total) return DREADZONE_PLAYER_ENTRY_INVALID;
    shadow=(u8 *)malloc((size_t)total);
    if(!shadow) return DREADZONE_PLAYER_ENTRY_NO_MEMORY;
    for(i=0;i<memory.view_count;++i) {
        views[i]=original[i];views[i].storage=shadow+offset;
        memcpy(views[i].storage,original[i].storage,original[i].bytes);offset+=original[i].bytes;
    }
    memory.views=views;status=entry_run(&memory,&input,&result);
    if(status>=0) {
        for(i=0;i<memory.view_count;++i) if(memcmp(original[i].storage,views[i].storage,original[i].bytes))
            memcpy(original[i].storage,views[i].storage,original[i].bytes);
        output->entry_return=result;
    }
    free(shadow);return status;
}
