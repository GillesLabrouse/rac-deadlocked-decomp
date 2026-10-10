#ifndef DEADLOCKED_DREADZONE_ANGLE_PAIR_H
#define DEADLOCKED_DREADZONE_ANGLE_PAIR_H
#include "common.h"

enum {
    DREADZONE_ANGLE_APPROXIMATE = 1,
    DREADZONE_ANGLE_INVALID = -1,
    DREADZONE_ANGLE_UNSUPPORTED = -2
};

/* Actual C60 reflection and ninth-degree polynomial, with each multiply/add
 * rounded separately to host IEEE binary32. x is angle; y is angle+pi/2.
 * Finite normal/zero inputs in [-pi,pi], nonzero magnitude at least2^-40,
 * are supported. Subnormal intermediate results flush to their signed zero
 * after each separately rounded operation; outputs must be normal/zero.
 * Actual polynomial order is retained, without shortcuts or angle clamps.
 * No output changes on refusal.
 * APPROXIMATE explicitly excludes physical VU rounding, flags and timing.
 * This does not execute the resident microprogram or modify VU registers.
 */
int dreadzone_angle_pair(float angle, float output_xy[2]);
#endif
