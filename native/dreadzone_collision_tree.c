#include "dreadzone_collision_tree.h"
#include <stdint.h>
#include <string.h>

static int overlap(uintptr_t a,size_t an,uintptr_t b,size_t bn) {
    return a<b+bn && b<a+an;
}
static int checked(s64 value,s32 *out) {
    if(value<(-2147483647LL-1) || value>2147483647LL) return 0;
    *out=(s32)value;return 1;
}
static int read_word(const DreadzoneCollisionTreeRequest *r,u32 address,u32 bytes,u32 *value) {
    u32 offset;
    if((address&(bytes-1)) || address<r->guest_base) return 0;
    offset=address-r->guest_base;
    if(offset>r->bytes || bytes>r->bytes-offset) return 0;
    *value=0;memcpy(value,r->storage+offset,bytes);return 1;
}
static int at_offset(const DreadzoneCollisionTreeRequest *r,u32 node,u32 offset,u32 bytes,u32 *value) {
    s32 address;
    if(!checked((s64)(s32)node+(s64)offset,&address)) return 0;
    return read_word(r,(u32)address,bytes,value);
}
static int index(const DreadzoneCollisionTreeRequest *r,u32 node,s32 coordinate,u32 *scaled,u32 shift) {
    u32 start,count;s32 delta,remain;
    if(!at_offset(r,node,0,2,&start) || !at_offset(r,node,2,2,&count) ||
       !checked((s64)coordinate-start,&delta) || !checked((s64)count-delta,&remain)) return -1;
    /* The real BLTZ delay executes count-delta before either early return. */
    if(delta<0 || remain<=0) return 0;
    *scaled=(u32)delta<<shift;return 1;
}
int dreadzone_collision_tree_lookup(const DreadzoneCollisionTreeRequest *request,u32 *packed_leaf) {
    DreadzoneCollisionTreeRequest r;
    u32 scaled,offset,value=0,node;s32 address;int found;
    if(request==NULL || packed_leaf==NULL ||
       (uintptr_t)request>UINTPTR_MAX-sizeof(r) ||
       (uintptr_t)packed_leaf>UINTPTR_MAX-sizeof(*packed_leaf)) return DREADZONE_TREE_INVALID;
    memcpy(&r,request,sizeof(r));
    if(r.storage==NULL || r.bytes==0 || (uintptr_t)r.storage>UINTPTR_MAX-r.bytes ||
       overlap((uintptr_t)packed_leaf,4,(uintptr_t)request,sizeof(r)) ||
       overlap((uintptr_t)packed_leaf,4,(uintptr_t)r.storage,r.bytes)) return DREADZONE_TREE_INVALID;
    if(r.guest_base>=0x80000000U || r.bytes>0x80000000U-r.guest_base)
        return DREADZONE_TREE_UNSUPPORTED;
    found=index(&r,r.root,r.y,&scaled,1);
    if(found<0) return DREADZONE_TREE_UNSUPPORTED;
    if(!found) goto done;
    if(!at_offset(&r,r.root,scaled+4,2,&offset)) return DREADZONE_TREE_UNSUPPORTED;
    offset<<=2;
    if(!offset) goto done;
    if(!checked((s64)(s32)r.root+offset,&address)) return DREADZONE_TREE_UNSUPPORTED;
    node=(u32)address;
    found=index(&r,node,r.x,&scaled,2);
    if(found<0) return DREADZONE_TREE_UNSUPPORTED;
    if(!found) goto done;
    if(!at_offset(&r,node,scaled+4,4,&offset)) return DREADZONE_TREE_UNSUPPORTED;
    if(!offset) goto done;
    if(!checked((s64)(s32)r.root+(s64)(s32)offset,&address)) return DREADZONE_TREE_UNSUPPORTED;
    node=(u32)address;
    found=index(&r,node,r.z,&scaled,2);
    if(found<0) return DREADZONE_TREE_UNSUPPORTED;
    if(!found) goto done;
    if(!at_offset(&r,node,scaled+4,4,&value)) return DREADZONE_TREE_UNSUPPORTED;
done:
    memcpy(packed_leaf,&value,4);return DREADZONE_TREE_READ;
}
