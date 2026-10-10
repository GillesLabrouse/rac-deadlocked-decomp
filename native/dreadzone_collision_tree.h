#ifndef DEADLOCKED_DREADZONE_COLLISION_TREE_H
#define DEADLOCKED_DREADZONE_COLLISION_TREE_H
#include "common.h"

typedef struct DreadzoneCollisionTreeRequest {
    const u8 *storage;
    u32 guest_base,bytes,root;
    s32 x,y,z;
} DreadzoneCollisionTreeRequest;
enum {DREADZONE_TREE_READ=1,DREADZONE_TREE_INVALID=-1,DREADZONE_TREE_UNSUPPORTED=-2};

/* Complete integer lookup of0053DBD0, whose internal EE ABI is t8=root,
 * v1=Y, v0=X, at=Z and return at=packed leaf. Root's Y row offsets are
 * u16 units of4; its X columns have root-relative u32 byte offsets; Z
 * cells contain the raw packed u32 leaf. Missing/outside cells yield zero.
 * One mapped, nonwrapping guest range below0x80000000 is supported. Signed
 * ADD/SUB traps and unmapped/unaligned reads are atomically refused.
 * Output cannot overlap request/storage. The table stays untouched.
 * READ means an actual table lookup, never a collision hit/miss decision.
 * The full53DC70/53EC88 probes still require geometry, VU and DMA effects.
 */
int dreadzone_collision_tree_lookup(const DreadzoneCollisionTreeRequest *request,u32 *packed_leaf);
#endif
