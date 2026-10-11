#ifndef DEADLOCKED_DREADZONE_ACTOR_ANIMATION_SELECT_H
#define DEADLOCKED_DREADZONE_ACTOR_ANIMATION_SELECT_H
#include "dreadzone_actor_animation.h"

typedef struct DreadzoneAnimationSelectRequest {
    const DreadzoneAnimationContext *memory;
    u32 player_guest;
    s32 clip,frame,flags;
    u32 blend_duration_bits;
} DreadzoneAnimationSelectRequest;
enum DreadzoneAnimationSelectResult {
    DREADZONE_ANIMATION_SELECT_INVALID=-1,
    DREADZONE_ANIMATION_SELECT_UNSUPPORTED=-2,
    DREADZONE_ANIMATION_SELECT_NO_MEMORY=-3,
    DREADZONE_ANIMATION_SELECT_NEEDS_5A36B0=-4,
    DREADZONE_ANIMATION_SELECT_NEEDS_5919B8=-5,
    DREADZONE_ANIMATION_SELECT_NEEDS_58AA90=-6,
    DREADZONE_ANIMATION_SELECT_NEEDS_RECURSION=-7,
    DREADZONE_ANIMATION_SELECT_NEEDS_669250=-8,
    DREADZONE_ANIMATION_SELECT_APPROXIMATE=1
};
/* Actual 688C08 argument contract, complete supported CPU memory paths through
 * real 58A850 allocation, 688E60, 688A60/688B78 and 675430 cleanup gates. Selected
 * class metadata and caller arguments are borrowed, never manufactured. Up to
 * 64 disjoint views/32 MiB are shadowed and published only on complete success.
 * Player and actor are disjoint from each other and canonical blend storage.
 * Frames 0..2^24 convert exactly to binary32; class and selected frame bounds
 * are validated using actual header counts. Negative blend reads the real
 * 220798 table using player+650; arithmetic requires finite positive duration
 * only when the allocator actually supplies a node. Division/subtraction use
 * explicit chopped finite binary32 approximation. Reached unavailable calls
 * refuse atomically, preserving caller RAM and genuine branch conditions.
 * Class table words and initial duration halfwords require their real guest
 * alignment. Additional count/alignment/bounded-arithmetic checks restrict the
 * supported domain; they are validation, not recovered game branch conditions.
 * No state entry, frequency, selected clip, effect, DMA or game frame invented.
 */
int dreadzone_actor_animation_select(const DreadzoneAnimationSelectRequest *);
#endif
