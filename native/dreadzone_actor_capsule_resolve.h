#ifndef DEADLOCKED_DREADZONE_ACTOR_CAPSULE_RESOLVE_H
#define DEADLOCKED_DREADZONE_ACTOR_CAPSULE_RESOLVE_H
#include "common.h"

enum DreadzoneCapsuleResolveOperation {
    DREADZONE_CAPSULE_RESOLVE_EF00=0,
    DREADZONE_CAPSULE_RESOLVE_F518=1
};
typedef struct DreadzoneCapsuleResolveRequest {
    u8 *storage;
    u32 guest_base,storage_bytes,record_guest;
    u32 operation,actor_pool_count;
    s32 attempt;
    u32 incoming_vf25_29_w[5];
} DreadzoneCapsuleResolveRequest;
typedef struct DreadzoneCapsuleResolveInfo {
    s32 original_return; /* Meaningful only when has_original_return is 1. */
    u32 has_original_return,resolve_calls,capsule_calls,static_calls;
    u32 contact_iterations,history_restores;
} DreadzoneCapsuleResolveInfo;
enum DreadzoneCapsuleResolveResult {
    DREADZONE_CAPSULE_RESOLVE_APPROXIMATE=1,
    DREADZONE_CAPSULE_RESOLVE_INVALID=-1,
    DREADZONE_CAPSULE_RESOLVE_UNSUPPORTED_PATH=-2,
    DREADZONE_CAPSULE_RESOLVE_UNSUPPORTED_FLOAT=-3,
    DREADZONE_CAPSULE_RESOLVE_TRANSACTION_MEMORY=-4
};

/* Complete selected original EF00 or F518 CPU effects, not a position frame.
 * The original EF00 return may genuinely be -1; this is reported separately
 * from the API status, and F518 executes real retries/history restores.
 *
 * All actual selected L01 record/global/body/item/collision backing is in one
 * exclusive writable, naturally aligned, nonwrapping guest range below2GiB,
 * at most64MiB and disjoint from scratch70000000..70003FFF. Record stride is
 *31F0 and16-byte alignment is required. Live packet242340, grid26FF20,
 * actor-base global222D14 and static-table global222BE0 are genuine bindings,
 * not substitute caller tables. Actor-pool count is an explicit backing bound.
 * The borrowed request is captured before writes and may alias backing; info
 * must be readable/writable and disjoint from request and all backing.
 *
 * Ordinary mode-zero branches are supported only when the actual dot/basis,
 *3D6, previous-state and dependency reads select the recovered capsule/static
 * path. Reached sphere53EC88, recovery68BB60 or offset68A638 paths refuse before
 * publication; selected probe-domain refusals also return UNSUPPORTED_PATH.
 * No orientation, state, marker or object pointer is forced to
 * evade them. Real item suspension98 and restore reads retain original order.
 *
 * Capsule and supplementary static publication(a1=0) preserve all five real
 * caller VF25..29 W lanes across every iteration. Static collection is a
 * separate original mode and includes a VF26.W write; it is never generalized
 * as W-preserving. Ordered normal/zero host-f32 and existing probe Q semantics
 * are explicit approximations, not physical VU/flags/timing/DMA validation.
 * All refusals preserve backing and info. Actual misses keep genuine probe
 * generation/visitation effects. GroundDDD0/F5E8 and construction are separate.
 */
int dreadzone_actor_capsule_resolve(const DreadzoneCapsuleResolveRequest *,
                                  DreadzoneCapsuleResolveInfo *);
#endif
