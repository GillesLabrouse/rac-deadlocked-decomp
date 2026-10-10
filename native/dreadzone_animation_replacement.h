#ifndef DEADLOCKED_ANIMATION_REPLACEMENT_H
#define DEADLOCKED_ANIMATION_REPLACEMENT_H
#include "common.h"

typedef struct DreadzoneAnimationReplacementView {
    u8 *storage;
    u32 guest_base;
    u32 bytes;
} DreadzoneAnimationReplacementView;
typedef struct DreadzoneAnimationReplacement {
    const DreadzoneAnimationReplacementView *views;
    u32 view_count;
    u32 destination;
    u32 source;
    u32 actors_begin;
    u32 actors_end;
} DreadzoneAnimationReplacement;
enum {
    DREADZONE_ANIMATION_REPLACED = 1,
    DREADZONE_ANIMATION_REPLACEMENT_INVALID = -1,
    DREADZONE_ANIMATION_REPLACEMENT_CAPACITY = -2
};
/* Complete CPU memory effects of actual L01 00575A18. Views use numeric guest
 * pointers, not widened records. Reads observe earlier planned writes, including
 * aliases. All accesses are checked before any write. Actor bounds describe a
 * whole number of 256-byte records; at most 32768 records are supported.
 * Writes aliasing the original bound globals 00222D14/00222D30 are refused;
 * source/destination/actor field aliases retain their ordered retail effects.
 * This does not load an archive, select a state, decode a pose, or run a GPU. */
int dreadzone_animation_replace(const DreadzoneAnimationReplacement *request);
#endif
