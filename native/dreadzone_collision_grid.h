#ifndef DEADLOCKED_DREADZONE_COLLISION_GRID_H
#define DEADLOCKED_DREADZONE_COLLISION_GRID_H
#include "common.h"
#define DREADZONE_COLLISION_GRID_MAX_RECORDS 239U
typedef struct DreadzoneCollisionGridRequest {
    float start_scaled[3],end_scaled[3];
} DreadzoneCollisionGridRequest;
typedef struct DreadzoneCollisionGridRecord {
    u32 fraction_bits,packed_cell;
} DreadzoneCollisionGridRecord;
typedef struct DreadzoneCollisionGridInfo {
    u32 record_count,zero_q_division_mask,has_integer_displacement;
} DreadzoneCollisionGridInfo;
enum {
    DREADZONE_GRID_APPROXIMATE=1,
    DREADZONE_GRID_INVALID=-1,
    DREADZONE_GRID_UNSUPPORTED=-2,
    DREADZONE_GRID_CAPACITY=-3
};
/* Actual axis-list production/stable merge0053DDE0..0053DF84, as a bounded
 * memory algorithm. Endpoints are the original VF30/31 world*1024 floats.
 * Coordinates must be nonnegative finite normal/positive-zero and <1048576;
 * each axis may cross at most79 cells, avoiding the actual80th-entry cap path
 * which counts an entry without storing its fraction. Numeric intermediates
 * must be finite normal/zero. All unsupported/capacity/alias failures preserve
 * outputs. Input/output overlap is allowed by snapshot; the two outputs must
 * be disjoint. Only record_count records are written; spare slots untouched.
 * Records begin with fraction0/start cell, follow the actual X,Y,Z tie order,
 * and end with the actual fraction1/cell0 sentinel. If integer endpoints are
 * identical, the original exits before writing that sentinel: only the first
 * record is supplied. Fraction words retain raw
 * IEEE bit patterns after the original signed-word clamp. Axis reciprocals
 * use ordered host f32, with the actual1/0 Q saturation represented explicitly
 * by zero_q_division_mask. No physical VU flags/rounding/timing are claimed.
 * has_integer_displacement exposes the later original integer-endpoint test;
 * it does not claim a complete probe, triangle result or player update.
 */
int dreadzone_collision_grid(const DreadzoneCollisionGridRequest *request,
                            DreadzoneCollisionGridRecord *records,u32 capacity,
                            DreadzoneCollisionGridInfo *info);
#endif
