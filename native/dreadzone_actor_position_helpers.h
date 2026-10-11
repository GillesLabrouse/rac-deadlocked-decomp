#ifndef DEADLOCKED_DREADZONE_ACTOR_POSITION_HELPERS_H
#define DEADLOCKED_DREADZONE_ACTOR_POSITION_HELPERS_H
#include "dreadzone_actor_direction.h"

enum DreadzonePositionHelperOperation {
    DREADZONE_POSITION_PROJECT = 0, /* 0068A7F0: value=output, target=input. */
    DREADZONE_POSITION_OFFSET = 1, /* 0068A3F8: value=output, target=input, scalar0=amount. */
    DREADZONE_POSITION_HEADING = 2, /* 0067EEA0: target=raw vector argument. */
    DREADZONE_POSITION_SETTLE = 3, /* 0067EA88: actual record fields. */
    DREADZONE_POSITION_SCALAR = 4, /* 00594D28: value/rate; scalar=target,force,damping,max. */
    DREADZONE_POSITION_VECTOR = 5, /* 00594DC8: value/target/rate; scalar=force,damping,max. */
    DREADZONE_POSITION_RADIUS = 6 /* 0067B9B0: actual metadata; initial is genuine a1. */
};
typedef struct DreadzonePositionHelperRequest {
    u32 operation,value,target,rate;
    s32 initial;
    f32 scalar[4];
} DreadzonePositionHelperRequest;

/* Complete selected mode-zero CPU helper effects, not the full position frame.
 * The original projection/offset/settling sources retain all PS2 modes.
 * All selected guest vectors are16-byte aligned, scalar words4-byte aligned.
 * Real record/body/metadata/global regions must be supplied; no radius, contact,
 * heading, clock or caller field is manufactured. Returned scalar is written
 * only by HEADING/SCALAR/VECTOR; other operations leave it untouched.
 *
 * Borrowed input context must have stable, exclusive, mutually disjoint host
 * and guest backings, no wraps or descriptor overlap. At most64 views/16MiB.
 * Guest aliases and ordered reloads operate on full shadow images. On refusal,
 * all RAM and returned_scalar are unchanged. Writes into read-only views refuse.
 * Numeric selected reads: signed zero or normal abs>=2^-40, positions/scalar
 * targets<=8192, vectors/rates/metadata<=16, body heading<=pi. Selected time,
 * scale, force,damping,max use0..16 and signed zero or abs>=2^-20. Original
 * operation order is retained, with approximate host binary32 and real C60
 * polynomial helper; exceptional/subnormal selected arithmetic/results refuse.
 * Physical VU/FPU precision, flags, timing, probes and completed playable frame
 * remain excluded. The true mode selector68A090 and constructor prerequisites
 * must still be supplied by a future complete caller.
 */
int dreadzone_actor_position_helper(const DreadzoneActorInputContext *,
    const DreadzonePositionHelperRequest *,f32 *returned_scalar);
#endif
