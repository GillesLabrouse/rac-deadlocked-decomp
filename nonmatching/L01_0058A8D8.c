/* Complete CPU node release0058A8D8..0058A940,104 bytes.
 * Exact SN comparison verified as a draft, outside accepted coverage. */
#include "common.h"
void func_L01_0055D2D0(u8 *,s32);
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_animation.h"
int dreadzone_animation_0058A8D8(const DreadzoneAnimationContext *host,u32 node_guest) {
    u8 *node,*occupancy,*metadata=0;
    u32 slot,metadata_guest=0;
    s32 index,status;
    if(!dreadzone_animation_validate(host)) return DREADZONE_ANIMATION_INVALID_STORAGE;
    if(node_guest<0x28EB40 || node_guest-0x28EB40>=0x400 || ((node_guest-0x28EB40)&31))
        return DREADZONE_ANIMATION_NEEDS_NODE_POOL;
    node=dreadzone_animation_translate(host,node_guest,32);
    if(!node) return DREADZONE_ANIMATION_NEEDS_NODE_POOL;
    slot=(node_guest-0x28EB40)/32;
    occupancy=dreadzone_animation_translate(host,0x28EF40+slot,1);
    if(!occupancy) return DREADZONE_ANIMATION_NEEDS_NODE_POOL;
    if(node[0x15]&4) {
        index=(s32)(((u32)node[0xE]|((u32)node[0xF]<<8))^0x8000U)-0x8000;
        metadata_guest=0x293004U+(u32)(index*8);
        if(dreadzone_animation_overlap(metadata_guest,1,node_guest,32) || metadata_guest==0x28EF40+slot)
            return DREADZONE_ANIMATION_INVALID_STORAGE;
        metadata=dreadzone_animation_translate(host,metadata_guest,1);
        if(!metadata) return DREADZONE_ANIMATION_NEEDS_METADATA;
        status=dreadzone_animation_0058B6B0(host,node_guest);
        if(status) return status;
    }
    *occupancy=0;
    func_L01_0055D2D0(node,32);
    return DREADZONE_ANIMATION_COMPLETED;
}
#else
extern u8 D_0028EB40[],D_0028EF40[];
void func_L01_0058B6B0(u8 *);
void func_L01_0058A8D8(u8 *node) {
    s32 slot;
    if(node[0x15]&4) func_L01_0058B6B0(node);
    slot=(s32)((u32)node-(u32)D_0028EB40)>>5;
    D_0028EF40[slot]=0;
    func_L01_0055D2D0(node,32);
}
#endif
