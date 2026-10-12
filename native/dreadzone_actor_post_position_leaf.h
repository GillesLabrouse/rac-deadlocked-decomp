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
 * The active branch runs the actual XY heading, conditional B8 selection,
 * damped scalar progression and snap test, then the bounded C60 angle-pair
 * value model. It stores +214 and +140.XYZ, retaining +140.W verbatim.
 * Pending +3CC writes affect later aliased clock/table reads. Low-V0 carries
 * distance or cos(angle)*rate bits; stores counts genuine persistent stores,
 * including the scalar callee's intermediate stores, committed on success.
 *
 * One stable exclusive writable guest range <=16MiB below2GiB, disjoint from
 * original scratch70000000..3FFF; naturally aligned guest words and a 16-byte
 * aligned complete31F0 record. Host backing may have any byte alignment.
 * GP-7B8B is read in its real delay slot even when selected height<=0;
 * absolute21DDF4 is read after the first length or on the active scalar path;
 * that path also reads21DDEC/25C4/214. B8 is read only on its taken likely
 * branch, after the heading helper runs. Unconsumed loaded bits need not be
 * finite. The request is captured and may alias backing, including
 * a selected store. Info is disjoint from both. Every refusal preserves all
 * RAM/info; this excludes the original's partial writes on later guest faults.
 *
 * Consumed scalar arithmetic supports signed zero/normal finite binary32
 * inputs/intermediates with nearest host rounding and gradual underflow.
 * Nonzero XY headings require the verified coefficient/quadrant tables at
 * 3B9540/3B9580; all table words are checked. Axes are bounded by2^30, and
 * nonzero atan ratio magnitude must be >=1/64. Final angle is in[-pi,pi],
 * nonzero magnitude>=2^-40. C60 uses the existing angle-pair approximation:
 * separate host-f32 operations and signed-zero flushing of its subnormal
 * intermediates. Broader angles do not inherit the selected exact-witness
 * software replay proof. Record backing must not alias the original caller's
 * stack locals; their original address/ownership is not established by this
 * request. Native scalar locals represent those separate temporary values.
 * Ordered host-f32/SQRT is approximate; physical FPU/VU/Q flags, timing,
 * GPR/VF/ACC/stack effects and a complete player frame remain excluded.
 * A successful selected return does not prove construction or live gameplay.
 */
int dreadzone_actor_post_position_leaf_00686420(
    const DreadzonePostPositionLeafRequest *,DreadzonePostPositionLeafInfo *);
#ifdef __cplusplus
}
#endif
#endif
