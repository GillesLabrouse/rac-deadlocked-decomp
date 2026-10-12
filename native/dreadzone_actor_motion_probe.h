#ifndef DEADLOCKED_DREADZONE_ACTOR_MOTION_PROBE_H
#define DEADLOCKED_DREADZONE_ACTOR_MOTION_PROBE_H
#include "dreadzone_actor_velocity.h"
typedef struct DreadzoneMotionProbeRequest {
    const u8 *storage;
    u32 guest_base,storage_bytes,record_guest;
} DreadzoneMotionProbeRequest;
typedef struct DreadzoneMotionProbeInfo {
    u32 return_low,length_result_bits,length_helper_calls;
} DreadzoneMotionProbeInfo;
/* Selected actual L01 6840B8 zero-planar early return only. Mode0 and signed
 * zero XY are required. Z/W are signed zero or magnitudes [2^-40,16], so the
 * reached four-lane square cannot underflow. The real 68A920 length executes.
 * One stable, exclusive, readable main-RAM range <=16MiB below10000000;
 * original guest stack/locals are separate. Record is16-aligned,31F0 mapped.
 * Request may alias RAM; info is disjoint from request/RAM. Refusal preserves
 * info/RAM; success never writes RAM. F12/F13/F14 and incoming A1 are unused
 * before this selected return. Original banks, stack, FCSR/VU status, precision
 * and timing are excluded. This RAM/low-V0 model has no matching credit. */
int dreadzone_actor_motion_probe_006840B8(const DreadzoneMotionProbeRequest *,DreadzoneMotionProbeInfo *);
#endif
