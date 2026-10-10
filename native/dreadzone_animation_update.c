#include "dreadzone_animation_update.h"
#include "dreadzone_animation_layers.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

enum { UPDATE_MAX_VIEWS=64, UPDATE_MAX_BYTES=32*1024*1024 };
static u32 update_word(const u8 *p) {
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}
static u16 update_half(const u8 *p) { return (u16)((u32)p[0]|((u32)p[1]<<8)); }
static s32 update_signed_half(const u8 *p) { return (s32)(update_half(p)^0x8000U)-0x8000; }
static void update_store(u8 *p,u32 value) {
    p[0]=(u8)value;p[1]=(u8)(value>>8);p[2]=(u8)(value>>16);p[3]=(u8)(value>>24);
}
static int update_fixed(u32 bits,s32 *result) {
    f32 value;memcpy(&value,&bits,4);
    if(!isfinite(value) || value<0 || value>=256) return 0;
    *result=(s32)(value*16.0f);return 1;
}
static int update_query(const DreadzoneAnimationContext *memory,s32 handle,u32 *result) {
    u8 *p,*record;
    u32 base,index=(u32)handle&63U;
    p=dreadzone_animation_translate(memory,0x21F710,4);
    if(!p) return DREADZONE_ANIMATION_UPDATE_INVALID;
    base=update_word(p);
    if(base!=0x1D24C0) return DREADZONE_ANIMATION_UPDATE_INVALID;
    *result=0;
    if(index==63) return 1;
    record=dreadzone_animation_translate(memory,base+(index<<7),128);
    if(!record) return DREADZONE_ANIMATION_UPDATE_INVALID;
    if((s32)(((u32)handle&0xFFFFU)>>6)==update_signed_half(record+0x58))
        *result=base+(index<<7);
    return 1;
}
static int update_release(const DreadzoneAnimationContext *memory,s32 handle) {
    u32 address;
    u8 *record;
    int status=update_query(memory,(s32)(((u32)handle&0xFFFFU)^0x8000U)-0x8000,&address);
    if(status<0 || !address) return status;
    record=dreadzone_animation_translate(memory,address,128);
    if(!record) return DREADZONE_ANIMATION_UPDATE_INVALID;
    if(record[0x10]==7) {
        record[0x10]=0;update_store(record+0x1C,0);update_store(record+0x40,0);
    } else if(record[0x10]!=0 && record[0x10]!=6) record[0x10]=4;
    return 1;
}
static int update_scheduler(DreadzoneAnimationContext *memory,u32 actor_guest,u8 *actor) {
    u8 *p,*record;
    u32 address;
    s32 suppressed,active=0;
    int status;
    p=dreadzone_animation_translate(memory,0x21E824,4);
    if(!p) return DREADZONE_ANIMATION_UPDATE_INVALID;
    if(update_word(p)==1) return DREADZONE_ANIMATION_UPDATE_NEEDS_665A28;
    suppressed=(update_half(actor+0x34)&0x40)!=0;
    status=update_query(memory,update_signed_half(actor+0x6E),&address);
    if(status<0) return status;
    if(address) {
        record=dreadzone_animation_translate(memory,address,128);
        if(!record) return DREADZONE_ANIMATION_UPDATE_INVALID;
        if(update_word(record+0x1C)==actor_guest) active=record[0x10]!=0;
    }
    if(active) {
        status=update_query(memory,update_signed_half(actor+0x6E),&address);
        if(status<0 || !address) return DREADZONE_ANIMATION_UPDATE_INVALID;
        record=dreadzone_animation_translate(memory,address,128);
        if(!record) return DREADZONE_ANIMATION_UPDATE_INVALID;
        if(update_word(record+0x1C)!=actor_guest) {
            actor[0x6E]=0xFF;actor[0x6F]=0xFF;return 1;
        }
        if(update_signed_half(record+0xA)==actor[0x6D] && !suppressed &&
           !(update_half(actor+0x74)&0x8000)) return 1;
        status=update_release(memory,update_signed_half(actor+0x6E));
        if(status<0) return status;
        actor[0x6E]=0xFF;actor[0x6F]=0xFF;
    } else if(actor[0x6D]!=0xFF && !suppressed && !(update_half(actor+0x74)&0x8000) &&
              !(update_half(actor+0x36)&4)) return DREADZONE_ANIMATION_UPDATE_NEEDS_626D50;
    return 1;
}
static int update_run(DreadzoneAnimationContext *memory,u32 actor_guest) {
    u8 *actor,*definition,*event,*phase;
    u32 old_frame,new_frame,definition_guest,event_guest,rate,count,model_count,i;
    s32 old_fixed,new_fixed,threshold;
    DreadzoneAnimationLayersRequest layers;
    int status;
    actor=dreadzone_animation_translate(memory,actor_guest,256);
    if(!actor || !actor_guest || (actor_guest&3) ||
       dreadzone_animation_overlap(actor_guest,256,0x1D24C0,63*128) ||
       dreadzone_animation_overlap(actor_guest,256,0x21E824,4) ||
       dreadzone_animation_overlap(actor_guest,256,0x21DDB8,4) ||
       dreadzone_animation_overlap(actor_guest,256,0x21F710,4)) return DREADZONE_ANIMATION_UPDATE_INVALID;
    old_frame=update_word(actor+0x44);model_count=actor[0x57];
    actor[0x55]&=0xFC;
    if(model_count) {
        layers.memory=memory;layers.actor_guest=actor_guest;
        layers.selected_definition_guest=update_word(actor+0x40);
        layers.effective_rate_bits=update_word(actor+0x48);
        status=dreadzone_animation_layers_advance(&layers);
        if(status!=DREADZONE_ANIMATION_LAYERS_APPROXIMATE) return status;
        count=actor[0x6C];rate=update_word(actor+0x48);new_frame=update_word(actor+0x44);
        /* Original conversion of old VF16 occurs even with no event scan. */
        if(!update_fixed(old_frame,&old_fixed)) return DREADZONE_ANIMATION_UPDATE_UNSUPPORTED;
        definition_guest=update_word(actor+0x40);
        if(count && rate && !(rate&0x80000000U)) {
            if(!update_fixed(new_frame,&new_fixed)) return DREADZONE_ANIMATION_UPDATE_UNSUPPORTED;
            if(definition_guest>0xFFFFFFFFU-0x17U) return DREADZONE_ANIMATION_UPDATE_INVALID;
            definition=dreadzone_animation_translate(memory,definition_guest+0x14,4);
            if(!definition) return DREADZONE_ANIMATION_UPDATE_INVALID;
            event_guest=update_word(definition);
            for(i=0;i<count;++i) {
                if(event_guest>0xFFFFFFFFU-3U) return DREADZONE_ANIMATION_UPDATE_INVALID;
                event=dreadzone_animation_translate(memory,event_guest,4);
                if(!event) return DREADZONE_ANIMATION_UPDATE_INVALID;
                threshold=(s32)update_half(event+2);
                if(threshold-old_fixed>0 && threshold-new_fixed<=0)
                    return DREADZONE_ANIMATION_UPDATE_NEEDS_626D50;
                /* Addition is actual guest32; do not wrap into a new view. */
                if(i+1<count && event_guest>0xFFFFFFFFU-4U) return DREADZONE_ANIMATION_UPDATE_INVALID;
                event_guest+=4;
            }
        }
    }
    /* Retail loads phase even when the non-FFFF handle forces dispatch. */
    phase=dreadzone_animation_translate(memory,0x21DDB8,4);
    if(!phase) return DREADZONE_ANIMATION_UPDATE_INVALID;
    if(update_signed_half(actor+0x6E)!=-1 || (update_word(phase)&3)==((actor_guest>>8)&3)) {
        status=update_scheduler(memory,actor_guest,actor);
        if(status<0) return status;
    }
    return DREADZONE_ANIMATION_UPDATE_APPROXIMATE;
}
int dreadzone_animation_update(const DreadzoneAnimationUpdateRequest *request) {
    DreadzoneAnimationUpdateRequest input;
    DreadzoneAnimationContext memory;
    DreadzoneAnimationMemory views[UPDATE_MAX_VIEWS],original[UPDATE_MAX_VIEWS];
    u8 *shadow;
    u64 bytes=0;
    u32 i,offset=0;
    int status;
    if(!request || (uintptr_t)request>UINTPTR_MAX-sizeof(*request)) return DREADZONE_ANIMATION_UPDATE_INVALID;
    input=*request;
    if(!dreadzone_animation_validate(input.memory)) return DREADZONE_ANIMATION_UPDATE_INVALID;
    memory=*input.memory;
    for(i=0;i<memory.view_count;++i) {
        original[i]=memory.views[i];bytes+=original[i].bytes;
        if(bytes>UPDATE_MAX_BYTES) return DREADZONE_ANIMATION_UPDATE_UNSUPPORTED;
    }
    if(!bytes) return DREADZONE_ANIMATION_UPDATE_INVALID;
    shadow=(u8 *)malloc((size_t)bytes);
    if(!shadow) return DREADZONE_ANIMATION_UPDATE_NO_MEMORY;
    for(i=0;i<memory.view_count;++i) {
        views[i]=original[i];views[i].storage=shadow+offset;
        memcpy(views[i].storage,original[i].storage,original[i].bytes);offset+=original[i].bytes;
    }
    memory.views=views;status=update_run(&memory,input.actor_guest);
    if(status==DREADZONE_ANIMATION_UPDATE_APPROXIMATE)
        for(i=0;i<memory.view_count;++i)
            if(memcmp(original[i].storage,views[i].storage,original[i].bytes))
                memcpy(original[i].storage,views[i].storage,original[i].bytes);
    free(shadow);return status;
}
