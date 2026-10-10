#include "dreadzone_animation_layers.h"
#include "dreadzone_animation_clock.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

enum { LAYER_MAX_VIEWS=64, LAYER_MAX_SNAPSHOT=32*1024*1024 };
static u32 layer_word(const u8 *p) {
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}
static void layer_store(u8 *p,u32 value) {
    p[0]=(u8)value;p[1]=(u8)(value>>8);p[2]=(u8)(value>>16);p[3]=(u8)(value>>24);
}
static f32 layer_float(u32 bits) { f32 value;memcpy(&value,&bits,4);return value; }
static u32 layer_bits(f32 value) { u32 bits;memcpy(&bits,&value,4);return bits; }
static f32 layer_subtract(f32 left,f32 right) {
    double exact=(double)left-(double)right;
    f32 result=(f32)exact;
    if(fabs((double)result)>fabs(exact)) result=layer_float(layer_bits(result)-1U);
    return result;
}
static int layer_node(u32 address) {
    return address>=0x28EB40 && address-0x28EB40<0x400 &&
        ((address-0x28EB40)&31)==0;
}
static int layer_time(DreadzoneAnimationContext *memory,u32 layer_guest,u32 *next) {
    u8 local[0x56]={0};
    u8 *layer,*definition;
    u32 definition_guest,definition_bytes,count;
    DreadzoneAnimationClockRequest clock;
    int result;
    layer=dreadzone_animation_translate(memory,layer_guest,0x16);
    if(!layer || layer_guest<0x40) return DREADZONE_ANIMATION_LAYERS_INVALID;
    definition_guest=layer_word(layer);
    definition=dreadzone_animation_translate(memory,definition_guest,0x11);
    if(!definition) return DREADZONE_ANIMATION_LAYERS_INVALID;
    count=definition[0x10];definition_bytes=0x20+count*4;
    definition=dreadzone_animation_translate(memory,definition_guest,definition_bytes);
    if(!definition) return DREADZONE_ANIMATION_LAYERS_INVALID;
    memcpy(local+0x40,layer,0x16);
    /* Local component has no chain; the actual guest link remains untouched
     * and is consumed by the enclosing complete589CE0 state machine. */
    layer_store(local+0x50,0);
    clock.actor=local;clock.actor_bytes=sizeof(local);clock.actor_guest=layer_guest-0x40;
    clock.definition=definition;clock.definition_bytes=definition_bytes;
    clock.definition_guest=definition_guest;
    result=dreadzone_animation_clock_advance(&clock);
    if(result!=DREADZONE_ANIMATION_CLOCK_APPROXIMATE)
        return result==DREADZONE_ANIMATION_CLOCK_UNSUPPORTED ?
            DREADZONE_ANIMATION_LAYERS_UNSUPPORTED : DREADZONE_ANIMATION_LAYERS_INVALID;
    /* Original zero-rate path publishes no time fields. */
    if(layer_word(layer+8)!=0) {
        layer[0x15]=local[0x55];
        layer[0xC]=local[0x4C];layer[0xD]=local[0x4D];
        *next=layer_word(layer+0x10); /* Original read precedes frame store. */
        layer_store(layer+4,layer_word(local+0x44));
    } else *next=layer_word(layer+0x10);
    return DREADZONE_ANIMATION_LAYERS_APPROXIMATE;
}

static int layer_run(DreadzoneAnimationContext *memory,u32 actor_guest) {
    u8 *actor,*node,*link;
    u32 current,next,link_guest,seen=0,slot,weight_bits,decay_bits,difference_bits;
    f32 weight,decay,difference;
    int status;
    actor=dreadzone_animation_translate(memory,actor_guest,0x100);
    if(!actor || !actor_guest || (actor_guest&3) ||
       dreadzone_animation_overlap(actor_guest,0x100,0x28EB40,0x400))
        return DREADZONE_ANIMATION_LAYERS_INVALID;
    if(layer_word(actor+0x50)!=0 && ((s8)actor[0x20]<0 || (s8)actor[0x31]==0)) {
        status=dreadzone_animation_0058ACB8(memory,actor_guest);
        if(status!=0) return DREADZONE_ANIMATION_LAYERS_INVALID;
    }
    status=layer_time(memory,actor_guest+0x40,&current);
    if(status!=DREADZONE_ANIMATION_LAYERS_APPROXIMATE) return status;
    while(current!=0) {
        if(!layer_node(current)) return DREADZONE_ANIMATION_LAYERS_INVALID;
        slot=(current-0x28EB40)/32;
        if(seen&(1U<<slot)) return DREADZONE_ANIMATION_LAYERS_UNSUPPORTED;
        seen|=1U<<slot;
        status=layer_time(memory,current,&next);
        if(status!=DREADZONE_ANIMATION_LAYERS_APPROXIMATE) return status;
        node=dreadzone_animation_translate(memory,current,32);
        if(!node) return DREADZONE_ANIMATION_LAYERS_INVALID;
        current=next;
    }
    link_guest=actor_guest+0x50;
    current=layer_word(actor+0x50);
    while(current!=0) {
        node=dreadzone_animation_translate(memory,current,32);
        if(!node || !layer_node(current)) return DREADZONE_ANIMATION_LAYERS_INVALID;
        weight_bits=layer_word(node+0x18);decay_bits=layer_word(node+0x1C);
        weight=layer_float(weight_bits);decay=layer_float(decay_bits);
        if(!isfinite(weight) || !isfinite(decay) || fabs((double)weight)>1048576.0 ||
           fabs((double)decay)>1048576.0) return DREADZONE_ANIMATION_LAYERS_UNSUPPORTED;
        difference=layer_subtract(weight,decay);difference_bits=layer_bits(difference);
        next=layer_word(node+0x10);
        if((s32)decay_bits>0) {
            layer_store(node+0x18,difference_bits);
            if((s32)difference_bits<=0) {
                link=dreadzone_animation_translate(memory,link_guest,4);
                if(!link) return DREADZONE_ANIMATION_LAYERS_INVALID;
                layer_store(link,next);
                status=dreadzone_animation_0058A8D8(memory,current);
                if(status!=0) return DREADZONE_ANIMATION_LAYERS_INVALID;
                /* Real post-release reload; not the previously captured next. */
                current=layer_word(link);
                continue;
            }
        } else if(decay_bits!=0) {
            layer_store(node+0x18,difference_bits);
            if((s32)layer_bits(layer_subtract(difference,1.0f))>=0) {
                layer_store(node+0x18,0x3F800000);
                layer_store(node+0x1C,0);
            }
        }
        link_guest=current+0x10;current=next;
    }
    return DREADZONE_ANIMATION_LAYERS_APPROXIMATE;
}

int dreadzone_animation_layers_advance(const DreadzoneAnimationLayersRequest *request) {
    DreadzoneAnimationLayersRequest input;
    DreadzoneAnimationContext memory;
    DreadzoneAnimationMemory views[LAYER_MAX_VIEWS],original[LAYER_MAX_VIEWS];
    u8 *snapshot,*actor;
    u64 bytes=0;
    u32 i,offset=0;
    int status;
    if(!request || (uintptr_t)request>UINTPTR_MAX-sizeof(*request))
        return DREADZONE_ANIMATION_LAYERS_INVALID;
    input=*request;
    if(!dreadzone_animation_validate(input.memory)) return DREADZONE_ANIMATION_LAYERS_INVALID;
    memory=*input.memory;
    actor=dreadzone_animation_translate(input.memory,input.actor_guest,0x100);
    if(!actor || layer_word(actor+0x40)!=input.selected_definition_guest ||
       layer_word(actor+0x48)!=input.effective_rate_bits)
        return DREADZONE_ANIMATION_LAYERS_INVALID;
    for(i=0;i<memory.view_count;++i) {
        original[i]=memory.views[i];bytes+=original[i].bytes;
        if(bytes>LAYER_MAX_SNAPSHOT) return DREADZONE_ANIMATION_LAYERS_UNSUPPORTED;
    }
    snapshot=(u8 *)malloc((size_t)bytes);
    if(!snapshot) return DREADZONE_ANIMATION_LAYERS_NO_MEMORY;
    for(i=0;i<memory.view_count;++i) {
        views[i]=original[i];views[i].storage=snapshot+offset;
        memcpy(views[i].storage,original[i].storage,original[i].bytes);
        offset+=original[i].bytes;
    }
    memory.views=views;
    status=layer_run(&memory,input.actor_guest);
    if(status==DREADZONE_ANIMATION_LAYERS_APPROXIMATE) {
        for(i=0;i<memory.view_count;++i) {
            if(memcmp(original[i].storage,views[i].storage,original[i].bytes)!=0)
                memcpy(original[i].storage,views[i].storage,original[i].bytes);
        }
    }
    free(snapshot);
    return status;
}
