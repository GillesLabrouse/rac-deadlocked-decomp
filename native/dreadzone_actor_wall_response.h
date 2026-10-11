#ifndef DEADLOCKED_DREADZONE_ACTOR_WALL_RESPONSE_H
#define DEADLOCKED_DREADZONE_ACTOR_WALL_RESPONSE_H
#include "dreadzone_wall_contact.h"

typedef struct DreadzoneActorWallResponseRequest {
    DreadzoneWallRequest wall;
    u32 record_guest;
    s32 force;
    u32 incoming_vf28_29_w[2];
} DreadzoneActorWallResponseRequest;

typedef struct DreadzoneActorWallResponseInfo {
    u32 probe_calls,surface_calls,inclination_calls,projection_calls;
    u32 velocity_changed,approximate_q_exception_bits;
    u32 outgoing_vf25_29_w[5];
} DreadzoneActorWallResponseInfo;

enum DreadzoneActorWallResponseResult {
    DREADZONE_ACTOR_WALL_APPROXIMATE=1,
    DREADZONE_ACTOR_WALL_INVALID=-1,
    DREADZONE_ACTOR_WALL_UNSUPPORTED_PATH=-2,
    DREADZONE_ACTOR_WALL_UNSUPPORTED_FLOAT=-3,
    DREADZONE_ACTOR_WALL_TRANSACTION_MEMORY=-4
};

/* Complete selected mode0 CPU response0067D6E8, including both real mask2
 * probes, first-surface retention, second-packet inclination, in-place normal
 * projection and ordered inward-velocity removal. No position/frame or
 * populated-world geometry completion is implied. Genuine wall dependencies
 * and their eligible-geometry refusals execute, never a substituted miss.
 *
 * One writable, nonwrapping guest range below2GiB, at most16MiB, excludes
 * scratchpad. record_guest is16-aligned with31F0 mapped bytes. Authentic
 * packet242340/grid26FF20/actor-base222D14 bindings are required; actual
 * globals21DDF4/21DDB8/220648 and any reached atan tables must be present.
 * Every reached mode/projection and guest field is reloaded after writes.
 * Endpoint quads are actual local results, without invented guest mappings.
 * wall.start_guest/end_guest/caller_flags/ignored_actor_guest/cache_guest are
 * unused: the real body supplies its own endpoints, mask2, actor2EF0 and0.
 * Request is captured and may alias RAM; info is disjoint from request/RAM.
 * Exclusive access and readable host spans are required. All refusals leave
 * RAM/info untouched; successful real early returns still complete this body.
 *
 * Selected operations require finite signed-zero/normal binary32 inputs and
 * intermediates; underflow/exceptional values refuse atomically. Normal/Q/ACC
 * use ordered host-f32 approximations and genuine recovered dependencies,
 * not physical VU flags/rounding/timing. approximate_q_exception_bits contains
 * the declared diagnostic I/D bits only. Exactly five caller W words are
 * transported: VF25..27.W remain, VF28/29.W clear after any real wall probe.
 * No complete VF register or hardware-status publication is claimed.
 */
int dreadzone_actor_wall_response_0067D6E8(
    const DreadzoneActorWallResponseRequest *,DreadzoneActorWallResponseInfo *);
#endif
