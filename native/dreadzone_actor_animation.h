#ifndef DEADLOCKED_DREADZONE_ACTOR_ANIMATION_H
#define DEADLOCKED_DREADZONE_ACTOR_ANIMATION_H
#include "common.h"
typedef struct DreadzoneAnimationMemory {
    u8 *storage;
    u32 guest_base, bytes;
} DreadzoneAnimationMemory;
typedef struct DreadzoneAnimationContext {
    const u8 *record;
    u32 record_guest, record_bytes;
    const DreadzoneAnimationMemory *views;
    u32 view_count;
} DreadzoneAnimationContext;
enum DreadzoneAnimationResult {
    DREADZONE_ANIMATION_COMPLETED = 0,
    DREADZONE_ANIMATION_INVALID_STORAGE = -1,
    DREADZONE_ANIMATION_NEEDS_ACTOR = -2,
    DREADZONE_ANIMATION_NEEDS_NODE_POOL = -3,
    DREADZONE_ANIMATION_NEEDS_METADATA = -4,
    DREADZONE_ANIMATION_NEEDS_DEFINITION = -5,
    DREADZONE_ANIMATION_NEEDS_FRAME_DATA = -6,
    DREADZONE_ANIMATION_UNSUPPORTED_ARGUMENT = -7
};
/* Complete CPU animation setup00688FE0 and its real release dependencies.
 * Animation indices0..255 and frames0..2^24 are supported: these integer frames
 * convert exactly to binary32 under every rounding mode. Retail takes an s32
 * frame, also used to address data; accepting caller float bits would change
 * its ABI and calculation. Out-of-domain arguments fail before mutation.
 *
 * At most64 disjoint, nonwrapping guest/host views are accepted. Actor, record,
 * released node, release bytes and selected definition/frame reads must have
 * disjoint footprints. Dependencies are validated before any mutation.
 * Host context/view descriptors must also be outside every mapped backing
 * region; a node clear must never overwrite descriptors used by a callee.
 * Context, descriptors, record and backing spans reject numeric host wrapping
 * before access. Otherwise the caller must provide readable host allocations.
 * The actual32-byte node clear precedes reloading its next pointer, so only
 * the head is released; old tail/self pointers are not traversed or rejected.
 * No animation transform, VU/DMA effect or actor constructor is substituted.
 */
int dreadzone_animation_00688FE0(const DreadzoneAnimationContext *,s32 animation,s32 frame);
int dreadzone_animation_0058A8D8(const DreadzoneAnimationContext *,u32 node_guest);
int dreadzone_animation_0058B6B0(const DreadzoneAnimationContext *,u32 node_guest);
/* Complete58ACB8 cleanup through the genuine58A8D8 release. Next links are
 * captured before each clear, including self/cyclic links. A byte journal
 * preflights all releases and metadata aliases before the first store.
 * Metadata may alias fields of a future node or a later occupancy byte.
 * Actor/release footprints and aliases forbidden by the release are refused
 * atomically. No callback, animation selection or GPU effect is substituted. */
int dreadzone_animation_0058ACB8(const DreadzoneAnimationContext *,u32 actor_guest);
/* Shared bounded memory validation for these CPU implementations. */
int dreadzone_animation_validate(const DreadzoneAnimationContext *);
u8 *dreadzone_animation_translate(const DreadzoneAnimationContext *,u32 address,u32 bytes);
int dreadzone_animation_overlap(u32 a,u32 an,u32 b,u32 bn);
#endif
