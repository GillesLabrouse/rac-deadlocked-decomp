#ifndef DEADLOCKED_DREADZONE_ACTOR_CONSTRUCTOR_H
#define DEADLOCKED_DREADZONE_ACTOR_CONSTRUCTOR_H
#include "common.h"

/* Supported complete L01 00668070 ordinary null-actor/index-zero RAM path.
 * The occupied pool must contain class23F7; class171C conversion, allocation,
 * nonzero mode/height/index and nested unsupported paths refuse atomically.
 * Record, pool and global ranges obey the component layout contracts. Finite
 * arithmetic is the established ordered binary32 host model, not hardware
 * VU flags, Q precision or timing equivalence. This adds no PS2 matching credit.
 *
 * Caller-owned W/side-local W and initial SPR/VU data are explicit inputs.
 * They are retained before writes and may borrow readable RAM backing. On
 * success complete RAM publishes, including borrowed input backing; disjoint
 * input banks remain unchanged. No SPR/VU/register/stack outputs are promised.
 * A selected startup, later model allocation and a repeated frame need their
 * own validation. The request itself may reside in readable RAM backing.
 */
typedef struct DreadzoneActorConstructor {
    u8 *storage;
    u32 guest_base, bytes, record, actor_pool_count, grid_block_count;
    const u32 *incoming_vf25_27_w;
    const u32 *incoming_side_output_w;
    const u8 *initial_scratch;
    u32 scratch_bytes;
    const u8 *initial_vu_data;
    u32 vu_data_bytes;
} DreadzoneActorConstructor;
enum DreadzoneActorConstructorResult {
    DREADZONE_CONSTRUCTOR_COMPLETED=1,
    DREADZONE_CONSTRUCTOR_INVALID=-1,
    DREADZONE_CONSTRUCTOR_UNSUPPORTED=-2,
    DREADZONE_CONSTRUCTOR_ALLOCATION_FAILED=-3
};
int dreadzone_actor_construct(const DreadzoneActorConstructor *);
#endif
