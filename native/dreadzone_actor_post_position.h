#ifndef DEADLOCKED_DREADZONE_ACTOR_POST_POSITION_H
#define DEADLOCKED_DREADZONE_ACTOR_POST_POSITION_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct DreadzonePostPositionRequest {
    uint8_t *storage;
    uint32_t guest_base,storage_bytes,record_guest,gp_guest;
} DreadzonePostPositionRequest;
typedef struct DreadzonePostPositionInfo {
    uint32_t orientation,visibility,half_timers,word_timers,float_timers;
    uint32_t queue_calls,input_calls,leaf_calls,height_calls;
} DreadzonePostPositionInfo;
enum { DREADZONE_POST_POSITION_APPROXIMATE=1,DREADZONE_POST_POSITION_INVALID=-1,
       DREADZONE_POST_POSITION_UNSUPPORTED=-2,DREADZONE_POST_POSITION_MEMORY=-4 };
/* Complete selected L01 684EF0 RAM returns: real orientation/visibility,
 * ordered signed timers/counters, one-shot callback enqueue, input remap,
 * reached 686420, matrix projection and aliased height offset. Counters record
 * top-level calls, including skipped effects. Queue execution is separate.
 * This adapter status is not the original path-dependent low-V0 return.
 *
 * Caller supplies one stable exclusive writable guest image <=16MiB below
 * 2GiB, outside70000000..70003FFF; naturally aligned guest loads/stores and
 * a complete31F0/16 record. Actual GP/globals, actor/class, decoded input,
 * queue/list and math tables must map on reached paths. Host backing may have
 * any byte alignment. Descriptors are captured; the request may borrow RAM.
 * Info is disjoint from request/backing. Guest aliases follow ordered shadow
 * reads/writes. Success commits the entire image and info; every refusal
 * preserves both. Original call-stack/local addresses must not alias this
 * backing; native scalar locals represent their separate temporary values.
 * Original API/storage types and higher-level ownership remain hypotheses.
 *
 * Finite normal/signed-zero arithmetic, nearest host binary32 and gradual
 * underflow. Existing bounded orientation, heading, scalar and C60 value
 * models retain their domains. Integer-to-float counters are bounded by2^24;
 * updated values must be integral in that range. Uncaptured physical FCSR
 * does not justify fractional CVT.W.S results. Height source Z<=8192 in
 * magnitude, nonzero>=2^-20; offset magnitude<=16, same nonzero floor.
 *
 * Unsupported reached callbacks, mode1 control, actor event2EE0, uncaptured
 * contact interactions, input predicate1AE0=1, airborne velocity branches
 * and A8<1 death handling refuse. No branch or callee result is fabricated.
 * Physical rounding/flags/timing, GPR/FPR/VF/VI/ACC/Q/stack output, full67C010,
 * authentic startup/resource binding and a playable frame remain excluded.
 */
int dreadzone_actor_post_position_00684EF0(
    const DreadzonePostPositionRequest *,DreadzonePostPositionInfo *);
#ifdef __cplusplus
}
#endif
#endif
