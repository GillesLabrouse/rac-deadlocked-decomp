#ifndef DEADLOCKED_DREADZONE_ANIMATION_UPDATE_H
#define DEADLOCKED_DREADZONE_ANIMATION_UPDATE_H
#include "dreadzone_actor_animation.h"

typedef struct DreadzoneAnimationUpdateRequest {
    const DreadzoneAnimationContext *memory;
    u32 actor_guest;
} DreadzoneAnimationUpdateRequest;
enum DreadzoneAnimationUpdateResult {
    DREADZONE_ANIMATION_UPDATE_INVALID = -1,
    DREADZONE_ANIMATION_UPDATE_UNSUPPORTED = -2,
    DREADZONE_ANIMATION_UPDATE_NO_MEMORY = -3,
    DREADZONE_ANIMATION_UPDATE_NEEDS_665A28 = -4,
    DREADZONE_ANIMATION_UPDATE_NEEDS_626D50 = -5,
    DREADZONE_ANIMATION_UPDATE_APPROXIMATE = 1
};
/* Complete589F98 CPU RAM closure for paths whose reached dependencies are
 * recovered: real layers589CE0, scheduler586FE0, lookup55DC00 and releases
 *6268A8/6268D8. Mode/phase, flags/rate, selected metadata/event tables and
 * class inputs live in explicit guest views; nothing selects clip0 here.
 * Reached mode1 predicate665A28 or event/allocation626D50 refuses atomically.
 * Events are never forcibly removed and scheduler calls are never skipped.
 *
 * Up to64 disjoint views/32 MiB are copied to a shadow. All original rereads
 * see preceding shadow writes. No publication follows a refusal; successful
 * changed views are published with caller storage exclusively borrowed.
 * Context/view descriptors follow the existing validation restrictions and
 * the request is captured before writes. Actor is disjoint from the canonical
 * scheduler pool (63 records at001D24C0) and its mode/phase/pointer globals.
 * Pointer global0021F710 must contain the verified retail001D24C0 binding.
 * Query accepts every original nonreserved index if its record is mapped;
 * it does not invent an occupancy check or change generation signedness.
 * Layers retain their explicit finite approximate EE arithmetic domain.
 * VFTOI4 event quantization supports finite frames in[0,256), exact *16 then
 * truncation. CPU memory effects only: no VF publication, events synthesized,
 * DMA, player689090 wrapper, display frequency or gameplay completion. */
int dreadzone_animation_update(const DreadzoneAnimationUpdateRequest *);
#endif
