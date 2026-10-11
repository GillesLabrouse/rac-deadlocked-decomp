#ifndef DEADLOCKED_DREADZONE_ACTOR_SLOPE_H
#define DEADLOCKED_DREADZONE_ACTOR_SLOPE_H
#include "dreadzone_actor_input_bridge.h"

enum DreadzoneActorSlopeResult {
    DREADZONE_SLOPE_COMPLETED=0,
    DREADZONE_SLOPE_APPROXIMATE=1,
    DREADZONE_SLOPE_INVALID=-1,
    DREADZONE_SLOPE_NEEDS_MEMORY=-2,
    DREADZONE_SLOPE_UNSUPPORTED_FLOAT=-3,
    DREADZONE_SLOPE_TRANSACTION_MEMORY=-4,
    DREADZONE_SLOPE_UNSUPPORTED_TABLES=-5
};
/* Complete selected0067D4E8 slope response. The real signed half2FE and
 *2CC/158/226/1D8 gates select the recovered55D708 and planar normalization.
 * Only an inward velocity component is corrected. Original velocity W is
 * retained; time0021DDF8 is reloaded after the first velocity store, while
 * scale25C4 is retained before it. No mode, contact, timing or normal is forced.
 *
 * Context/backings follow the stable, exclusive input-bridge contract with
 * up to64 disjoint host/guest views and16MiB total. Selected writes occur on
 * full copied RAM and commit only after supported completion, including aliases
 * of record/global reads. Context/descriptors are disjoint from backing.
 * Raw terrain normal components are zero or abs[2^-40,2^30]; they are not
 * replaced by unit normals. Velocity components use abs[2^-40,16]; time/scale use
 * zero or abs[2^-20,16]. Predicate-only floats accept finite normal/zero.
 * Subnormal/exceptional selected inputs/intermediates/results refuse atomically.
 * Selected angle calculation requires mapped original numeric coefficients
 * at003B9540/003B9580, checked before using the recovered immutable polynomial.
 * A changed table refuses. Its nonzero ratio must have abs>=2^-7, ensuring
 * the selected ratio powers stay normal; zero ratio remains supported.
 * Raw unused W transport is preserved without evaluating it. Ordered host
 * binary32 and saturated zero-Q normalization are approximations; unused VU
 * lanes/flags, physical rounding/ACC precision and timing are not reproduced.
 * This response neither probes the ground nor integrates actor position.
 */
int dreadzone_actor_slope_0067D4E8(const DreadzoneActorInputContext *);
/* Requires an already validated shadow context, matching record backing. */
int dreadzone_actor_slope_bytes(const DreadzoneActorInputContext *,u8 *record);
#endif
