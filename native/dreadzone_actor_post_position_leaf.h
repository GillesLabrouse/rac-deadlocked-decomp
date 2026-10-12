#ifndef DEADLOCKED_DREADZONE_ACTOR_POST_POSITION_LEAF_H
#define DEADLOCKED_DREADZONE_ACTOR_POST_POSITION_LEAF_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct DreadzonePostPositionLeafRequest {
    uint8_t *storage;
    uint32_t guest_base,storage_bytes,record_guest,gp_guest;
} DreadzonePostPositionLeafRequest;
typedef struct DreadzonePostPositionLeafInfo {
    uint32_t original_low_word,stores;
} DreadzonePostPositionLeafInfo;
enum { DREADZONE_POST_POSITION_LEAF_OK=1,
       DREADZONE_POST_POSITION_LEAF_INVALID=-1,
       DREADZONE_POST_POSITION_LEAF_UNSUPPORTED=-2 };
/* Complete selected RAM/low-V0 returns of actual L01 686420..6866E8.
 * Executes the real height/clock/length/class gates, optional +3CC=5 mark,
 * zero-timer +214 clear, and second XY-length threshold. The executed first
 * length includes XYZ; the second sums XY. Both validate every executed Z/W
 * square, including unused ones. The second source W is only transported.
 * Reached angle/integrator/C60/vector branches refuse atomically; no length,
 * contact, timer or control flag is manufactured. Low-V0 may contain float
 * bits, not just integer zero. stores counts the genuine persistent stores.
 *
 * One stable exclusive writable guest range <=16MiB below2GiB, disjoint from
 * original scratch70000000..3FFF; naturally aligned guest words and a 16-byte
 * aligned complete31F0 record. Host backing may have any byte alignment.
 * GP-7B8B is read in its real delay slot even when selected height<=0;
 * absolute21DDF4 is read only after the first length. Unconsumed loaded bits
 * need not be finite. The request is captured and may alias backing, including
 * a selected store. Info is disjoint from both. Every refusal preserves all
 * RAM/info; this excludes the original's partial writes on later guest faults.
 *
 * Consumed arithmetic supports signed zero/normal finite binary32 inputs and
 * intermediates with nearest host rounding and gradual underflow. Ordered
 * host-f32/SQRT is an explicit approximation; physical FPU/VU/Q flags, timing,
 * GPR/VF/ACC/stack effects and a complete player frame remain excluded.
 * A successful selected return does not prove construction or live gameplay.
 */
int dreadzone_actor_post_position_leaf_00686420(
    const DreadzonePostPositionLeafRequest *,DreadzonePostPositionLeafInfo *);
#ifdef __cplusplus
}
#endif
#endif
