#ifndef DEADLOCKED_DREADZONE_PLAYER_STATE_ENTRY_H
#define DEADLOCKED_DREADZONE_PLAYER_STATE_ENTRY_H
#include "dreadzone_actor_animation.h"

typedef struct DreadzonePlayerStateEntryRequest {
    const DreadzoneAnimationContext *memory;
    u32 player_guest;
    s32 target_state,selection_option,force_option,t0_option;
} DreadzonePlayerStateEntryRequest;
typedef struct DreadzonePlayerStateEntryOutput { s32 entry_return; } DreadzonePlayerStateEntryOutput;
enum DreadzonePlayerStateEntryStatus {
    DREADZONE_PLAYER_ENTRY_REJECTED_EXACT=0,
    DREADZONE_PLAYER_ENTRY_APPROXIMATE=1,
    DREADZONE_PLAYER_ENTRY_INVALID=-1,
    DREADZONE_PLAYER_ENTRY_UNSUPPORTED=-2,
    DREADZONE_PLAYER_ENTRY_NO_MEMORY=-3,
    DREADZONE_PLAYER_ENTRY_NEEDS_586E80=-4,
    DREADZONE_PLAYER_ENTRY_NEEDS_VIRTUAL_ENTRY=-5,
    DREADZONE_PLAYER_ENTRY_NEEDS_6BF200=-6,
    DREADZONE_PLAYER_ENTRY_NEEDS_58AA90=-7,
    DREADZONE_PLAYER_ENTRY_NEEDS_5919B8=-8,
    DREADZONE_PLAYER_ENTRY_NEEDS_CLEANUP_RECURSION=-9,
    DREADZONE_PLAYER_ENTRY_NEEDS_GROUND_QUERY=-10,
    DREADZONE_PLAYER_ENTRY_NEEDS_TRANSPORT=-11,
    /* -100 plus the original negative animation-select status preserves its
     * actual reached dependency identity, rather than capturing success. */
    DREADZONE_PLAYER_ENTRY_SELECT_FAILURE_BASE=-100,
    DREADZONE_PLAYER_ENTRY_DIRECTION_FAILURE_BASE=-200,
    DREADZONE_PLAYER_ENTRY_FIELD_FAILURE_BASE=-300
};
/* Actual 699C70 ABI; 69A4F0 is an internal label, not an entry point.
 * Native support executes the complete selected states 0/1/2, including their
 * common prologue and suffix. The caller supplies actual guest32 histories,
 * controls, class metadata, ownership, configuration and memory backing.
 * Reached unsupported dependencies refuse; no field is cleared to avoid them.
 * All RAM and the result publish only after the complete supported operation.
 *
 * At most 64 disjoint views totaling 32 MiB are shadowed. The player must be
 * guest16 aligned with 0x31F0 bytes; the actor is distinct, aligned 256-byte
 * storage. Both are disjoint from the canonical blend and scheduler pools.
 * Relocated player/actor overlaps with fixed globals 21D000..227FFF or sparse
 * input rows 3B5050..3DCE4F refuse. This explicit native alias domain is not a
 * retail memory-layout predicate. Client fields and class/definition links may
 * alias mapped records: reads follow the original publication order, subject
 * to the recovered callees' documented bounded domains.
 *
 * Existing animation validation governs context/view descriptors and backing.
 * The result must be disjoint from mapped backing and those descriptors; the
 * request is captured before publication. Retail gate rejection returns exact
 * status0 with entry_return0 and unchanged RAM. Supported completion returns
 * status1 and actual entry_return1, with explicitly approximate finite EE/VU
 * arithmetic inherited from direction, length and field primitives. Numerical
 * bounds and refusal statuses describe this implementation's supported domain,
 * not new game predicates. No constructor, assumed wall-clock cadence,
 * collision, model transform, rendering or playable frame is supplied. */
int dreadzone_player_state_entry(const DreadzonePlayerStateEntryRequest *,DreadzonePlayerStateEntryOutput *);
#endif
