/* Complete CPU animation setup00688FE0..00689090,176 bytes. NONMATCHING.
 * The next-node pointer is read AFTER its node is cleared by0058A8D8. */
#include "common.h"
typedef char AnimationWordWidth[sizeof(u32)==4 ? 1 : -1];
typedef char AnimationHalfWidth[sizeof(s16)==2 ? 1 : -1];
typedef char AnimationFloatWidth[sizeof(f32)==4 ? 1 : -1];
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_animation.h"
static u32 animation_word(const u8 *p) {
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}
static void animation_store(u8 *p,u32 v) {
    p[0]=(u8)v;p[1]=(u8)(v>>8);p[2]=(u8)(v>>16);p[3]=(u8)(v>>24);
}
int dreadzone_animation_00688FE0(const DreadzoneAnimationContext *host,s32 animation,s32 frame) {
    u8 *actor,*node=0,*definition,*frame_data;
    u32 actor_guest,node_guest,definition_guest,data_guest,field,occupancy_guest=0,metadata_guest=0;
    s32 index,status;
    union { f32 f;u32 bits; } frame_float;
    if(animation<0 || animation>255 || frame<0 || frame>16777216) return DREADZONE_ANIMATION_UNSUPPORTED_ARGUMENT;
    if(!dreadzone_animation_validate(host) || !host->record || host->record_bytes<0x31F0 ||
       (host->record_guest&3) || host->record_guest>0xFFFFFFFFU-(host->record_bytes-1))
        return DREADZONE_ANIMATION_INVALID_STORAGE;
    actor_guest=animation_word(host->record+0x2EF0);
    if(!actor_guest || (actor_guest&15)) return DREADZONE_ANIMATION_NEEDS_ACTOR;
    actor=dreadzone_animation_translate(host,actor_guest,0x100);
    if(!actor) return DREADZONE_ANIMATION_NEEDS_ACTOR;
    node_guest=animation_word(actor+0x50);
    if(node_guest) {
        if(node_guest<0x28EB40 || node_guest-0x28EB40>=0x400 || ((node_guest-0x28EB40)&31))
            return DREADZONE_ANIMATION_NEEDS_NODE_POOL;
        node=dreadzone_animation_translate(host,node_guest,32);
        occupancy_guest=0x28EF40+(node_guest-0x28EB40)/32;
        if(!node || !dreadzone_animation_translate(host,occupancy_guest,1)) return DREADZONE_ANIMATION_NEEDS_NODE_POOL;
        if(dreadzone_animation_overlap(actor_guest,0x100,node_guest,32) ||
           dreadzone_animation_overlap(actor_guest,0x100,occupancy_guest,1)) return DREADZONE_ANIMATION_INVALID_STORAGE;
        if(node[0x15]&4) {
            index=(s32)(((u32)node[0xE]|((u32)node[0xF]<<8))^0x8000U)-0x8000;
            metadata_guest=0x293004U+(u32)(index*8);
            if(!dreadzone_animation_translate(host,metadata_guest,1)) return DREADZONE_ANIMATION_NEEDS_METADATA;
            if(dreadzone_animation_overlap(metadata_guest,1,actor_guest,0x100) ||
               dreadzone_animation_overlap(metadata_guest,1,node_guest,32) || metadata_guest==occupancy_guest)
                return DREADZONE_ANIMATION_INVALID_STORAGE;
        }
    }
    definition_guest=animation_word(actor+0x24);
    if(!definition_guest || (definition_guest&3) || definition_guest>0xFFFFFFFFU-(0x48+(u32)animation*4+3))
        return DREADZONE_ANIMATION_NEEDS_DEFINITION;
    field=definition_guest+0x48+(u32)animation*4;
    definition=dreadzone_animation_translate(host,field,4);
    if(!definition) return DREADZONE_ANIMATION_NEEDS_DEFINITION;
    if(dreadzone_animation_overlap(field,4,actor_guest,0x100) ||
       (node && (dreadzone_animation_overlap(field,4,node_guest,32) ||
                 dreadzone_animation_overlap(field,4,occupancy_guest,1) ||
                 (metadata_guest && dreadzone_animation_overlap(field,4,metadata_guest,1)))))
        return DREADZONE_ANIMATION_INVALID_STORAGE;
    data_guest=animation_word(definition);
    if(!data_guest || (data_guest&1) || data_guest>0xFFFFFFFFU-(0x20+(u32)frame*4+1))
        return DREADZONE_ANIMATION_NEEDS_FRAME_DATA;
    field=data_guest+0x20+(u32)frame*4;
    frame_data=dreadzone_animation_translate(host,field,2);
    if(!frame_data) return DREADZONE_ANIMATION_NEEDS_FRAME_DATA;
    if(dreadzone_animation_overlap(field,2,actor_guest,0x100) ||
       (node && (dreadzone_animation_overlap(field,2,node_guest,32) ||
                 dreadzone_animation_overlap(field,2,occupancy_guest,1) ||
                 (metadata_guest && dreadzone_animation_overlap(field,2,metadata_guest,1)))))
        return DREADZONE_ANIMATION_INVALID_STORAGE;
    /* Every callee's complete footprint is validated before the first store. */
    while(node_guest) {
        status=dreadzone_animation_0058A8D8(host,node_guest);
        if(status) return status;
        node_guest=animation_word(node+0x10); /* Real clear makes this zero. */
    }
    actor[0x54]=(u8)animation;
    animation_store(actor+0x50,0);
    frame_float.f=(f32)frame;
    animation_store(actor+0x44,frame_float.bits);
    animation_store(actor+0x48,0x3F800000);
    animation_store(actor+0x40,data_guest);
    actor[0x4C]=frame_data[0];actor[0x4D]=frame_data[1];
    return DREADZONE_ANIMATION_COMPLETED;
}
#else
void func_L01_0058A8D8(u8 *);
void func_L01_00688FE0(const u8 *record,s32 animation,s32 frame) {
    u8 *actor=*(u8 **)(record+0x2EF0);
    u8 *node=*(u8 **)(actor+0x50);
    u8 *definition,*data;
    while(node) {
        func_L01_0058A8D8(node);
        node=*(u8 **)(node+0x10);
    }
    definition=*(u8 **)(actor+0x24);
    actor[0x54]=(u8)animation;
    *(u32 *)(actor+0x50)=0;
    data=*(u8 **)((u32)animation*4+(u32)definition+0x48);
    *(f32 *)(actor+0x44)=(f32)frame;
    *(f32 *)(actor+0x48)=1.f;
    *(u8 **)(actor+0x40)=data;
    *(u16 *)(actor+0x4C)=*(u16 *)(data+0x20+(u32)frame*4);
}
#endif
