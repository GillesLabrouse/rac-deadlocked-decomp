/* Actual release flag helper0058B6B0..0058B6D0,32 bytes. NONMATCHING.
 * The metadata address uses the signed halfword at node+E. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_animation.h"
static int animation_host_span(const void *storage,uintptr_t bytes) {
    return storage && (uintptr_t)storage<=UINTPTR_MAX-bytes;
}
static int animation_host_overlap(const u8 *a,u32 an,const u8 *b,u32 bn) {
    uintptr_t aa=(uintptr_t)a,bb=(uintptr_t)b;
    return aa>=bb?aa-bb<bn:bb-aa<an;
}
int dreadzone_animation_overlap(u32 a,u32 an,u32 b,u32 bn) {
    return a>=b?a-b<bn:b-a<an;
}
int dreadzone_animation_validate(const DreadzoneAnimationContext *host) {
    u32 i,j;
    /* Reject numeric wrap before reading context, descriptors or backing data.
     * Nonwrapping caller objects must still be readable host allocations. */
    if(!animation_host_span(host,sizeof(*host))) return 0;
    if(host->view_count>64 ||
       (host->view_count && !animation_host_span(host->views,host->view_count*sizeof(*host->views)))) return 0;
    if(host->record && (!animation_host_span(host->record,host->record_bytes) ||
       (host->record_bytes && host->record_guest>0xFFFFFFFFU-(host->record_bytes-1)))) return 0;
    for(i=0;i<host->view_count;i++) {
        const DreadzoneAnimationMemory *a=&host->views[i];
        if(!a->bytes || !animation_host_span(a->storage,a->bytes) ||
           a->guest_base>0xFFFFFFFFU-(a->bytes-1)) return 0;
        if(animation_host_overlap((const u8 *)host,(u32)sizeof(*host),a->storage,a->bytes) ||
           animation_host_overlap((const u8 *)host->views,host->view_count*(u32)sizeof(*host->views),a->storage,a->bytes))
            return 0;
        for(j=0;j<i;j++) {
            const DreadzoneAnimationMemory *b=&host->views[j];
            if(dreadzone_animation_overlap(a->guest_base,a->bytes,b->guest_base,b->bytes) ||
               animation_host_overlap(a->storage,a->bytes,b->storage,b->bytes)) return 0;
        }
        if(host->record && host->record_bytes &&
           (dreadzone_animation_overlap(host->record_guest,host->record_bytes,a->guest_base,a->bytes) ||
            animation_host_overlap(host->record,host->record_bytes,a->storage,a->bytes))) return 0;
    }
    return 1;
}
u8 *dreadzone_animation_translate(const DreadzoneAnimationContext *host,u32 address,u32 bytes) {
    u32 i;
    if(!dreadzone_animation_validate(host) || !bytes || address>0xFFFFFFFFU-(bytes-1)) return 0;
    for(i=0;i<host->view_count;i++) {
        const DreadzoneAnimationMemory *v=&host->views[i];
        if(address>=v->guest_base && bytes<=v->bytes && address-v->guest_base<=v->bytes-bytes)
            return v->storage+(address-v->guest_base);
    }
    return 0;
}
int dreadzone_animation_0058B6B0(const DreadzoneAnimationContext *host,u32 node_guest) {
    u8 *node,*flag;
    u32 metadata;
    s32 index;
    if(!dreadzone_animation_validate(host)) return DREADZONE_ANIMATION_INVALID_STORAGE;
    if(node_guest<0x28EB40 || node_guest-0x28EB40>=0x400 || ((node_guest-0x28EB40)&31))
        return DREADZONE_ANIMATION_NEEDS_NODE_POOL;
    node=dreadzone_animation_translate(host,node_guest,32);
    if(!node) return DREADZONE_ANIMATION_NEEDS_NODE_POOL;
    index=(s32)(((u32)node[0xE]|((u32)node[0xF]<<8))^0x8000U)-0x8000;
    metadata=0x293004U+(u32)(index*8);
    if(dreadzone_animation_overlap(metadata,1,node_guest,32)) return DREADZONE_ANIMATION_INVALID_STORAGE;
    flag=dreadzone_animation_translate(host,metadata,1);
    if(!flag) return DREADZONE_ANIMATION_NEEDS_METADATA;
    *flag=0;
    return DREADZONE_ANIMATION_COMPLETED;
}
#else
typedef struct AnimationMetadataEntry { u8 prefix[4],flag,suffix[3]; } AnimationMetadataEntry;
typedef char AnimationMetadataStride[sizeof(AnimationMetadataEntry)==8 ? 1 : -1];
extern AnimationMetadataEntry D_00293000[];
void func_L01_0058B6B0(u8 *node) {
    s32 index=*(s16 *)(node+0xE);
    D_00293000[index].flag=0;
}
#endif
