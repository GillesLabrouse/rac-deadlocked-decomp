#ifndef DREADZONE_ACTOR_ORIENTATION_H
#define DREADZONE_ACTOR_ORIENTATION_H
#include "common.h"
typedef struct DreadzoneActorOrientationRequest {
    u8 *storage;u32 guest_base,storage_bytes,record_guest,gp_guest;
    u32 incoming_vf25_29_w[5],incoming_a0_high64[2];
} DreadzoneActorOrientationRequest;
typedef struct DreadzoneActorOrientationInfo {
    u32 skips,turns,compositions,extractions;
    u32 outgoing_vf25_29_w[5],outgoing_a0_high64[2];
} DreadzoneActorOrientationInfo;
/* Complete selected L01 683310 effects in an approximate binary32 domain.
 * Support genuine state/parity/mode>=3 exits, or actual mode0 5952B8 turns
 * ending with C0=C4=zero. The latter follows 67CBC0's real yaw-only fallback;
 * no C60/vector extraction executes. Nonzero final tilt and modes1/2 refuse.
 * Read the supplied incoming GP's delay-slot globals and actual absolute
 * DDEC/DDF4 clocks. Active first turn retains the GP-relative factor; inactive
 * first turn and the second turn reload absolute DDEC. No state, frame parity,
 * clock or tilt is manufactured.
 * Selected numeric inputs are signed zero or normal; angles<=pi, prior speed
 * <=2pi, clocks/time<=16 in absolute value. Force/damping must be in0..1 and
 * maximum in0..16. Tiny nonzero clock/speed inputs below2^-40 refuse.
 * Preserve explicit raw five W lanes and A0 high64 on supported completion.
 * Info/RAM remain unchanged on invalid mapping, unsupported path/numerics or
 * allocation failure (-1/-2/-3/-4). Success is1. The full contiguous pure RAM
 * mapping is at most16MiB, below2GiB and outside scratchpad70000000..70003FFF;
 * request may alias RAM, but info must be disjoint from both.
 * No full-GPR ABI, physical FPU/VU precision, constructor or player frame is
 * claimed. Counters count early exits/scalar turns; matrix composition and
 * extraction counters are zero throughout this supported domain. */
int dreadzone_actor_orientation_00683310(const DreadzoneActorOrientationRequest *,DreadzoneActorOrientationInfo *);
#endif
