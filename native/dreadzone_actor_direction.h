#ifndef DEADLOCKED_DREADZONE_ACTOR_DIRECTION_H
#define DEADLOCKED_DREADZONE_ACTOR_DIRECTION_H
#include "dreadzone_actor_input_bridge.h"
typedef struct DirectionVector { f32 v[4]; } DirectionVector;
enum DreadzoneActorDirectionResult {
    DREADZONE_DIRECTION_APPROXIMATE=1,
    DREADZONE_DIRECTION_INVALID=-1,
    DREADZONE_DIRECTION_NEEDS_MEMORY=-2,
    DREADZONE_DIRECTION_UNSUPPORTED_MODE=-3,
    DREADZONE_DIRECTION_UNSUPPORTED_FLOAT=-4,
    DREADZONE_DIRECTION_TRANSACTION_MEMORY=-5
};
/* Complete selected paths of the original direction and rotation entries.
 * The actual mapped global21E878 and actor2659 must initially be zero.
 * This is the constructor path selected by configuration0057B038 input+2C=0,
 * not an assertion about every Dreadzone resource or player state.
 *
 * Context/backings obey the input bridge's disjoint, stable, exclusive RAM
 * contract. Native wrappers preserve ordered reads and guest aliases in full
 * shadow images, committing writable RAM only after a supported completion.
 * Direction computes actor1790/1780/17A0 from actor120/124, the genuine body
 * matrix and actor2CF0 matrix; it never adds position or invents a camera.
 * Rotation uses those actual state fields and recovered scalar callees.
 * Numeric domain: signed zero or |value|>=2^-20; axes<=1, scale0..16,
 * read facing/target/base angles<=pi, prior angular speed<=2pi. Actual matrix
 * coefficients are bounded by16. Turn modes are-1..2 and caller force,
 * damping and maximum0..1; selected boost globals and actor25C4 have |x|<=16.
 * The classification3 magnitude read has |x|<=1. All bounds concern the
 * actual selected reads; exceptional/subnormal intermediates/results refuse
 * before publication. No angle table, body basis or input history is invented.
 * Native arithmetic is approximate IEEE binary32, excluding physical FPU/VU
 * rounding, ACC precision, flags and scheduling. No playable frame is claimed.
 */
int dreadzone_actor_direction_0067C5F8(const DreadzoneActorInputContext *,s32 cardinal,f32 scale);
int dreadzone_actor_turn_0067CFD0(const DreadzoneActorInputContext *,s32 mode,f32 force,f32 damping,f32 maximum);
/* Internal calls require the validated shadow context. */
int func_L01_0067C5F8(const DreadzoneActorInputContext *,u8 *,s32,f32);
int func_L01_0067CFD0(const DreadzoneActorInputContext *,u8 *,s32,f32,f32,f32);
int dreadzone_direction_history(const DreadzoneActorInputContext *,s32,s32,u32,u32,s32 *);
s32 func_L01_006739B0(const u8 *);
f32 func_L01_0055D828(f32,f32);
f32 func_L01_0055D870(f32,f32);
f32 func_L01_00595188(s32,f32,f32);
f32 func_L01_00594A88(f32 *,f32,f32,f32,f32);
f32 func_L01_005952B8(f32 *,f32 *,s32,f32,f32,f32,f32);
int func_L01_0068ABA8(u8 *,DirectionVector *,const DirectionVector *,f32);
int dreadzone_direction_projection_bytes(u8 *,u8 *,const u8 *,f32);
f32 dreadzone_direction_progress_bytes(u8 *,f32,f32,f32,f32);
f32 dreadzone_direction_rotation_bytes(u8 *,u8 *,s32,f32,f32,f32,f32);
f32 dreadzone_direction_float(const u8 *);
void dreadzone_direction_store_float(u8 *,f32);
int dreadzone_direction_valid_float(f32,f32);
f32 dreadzone_direction_min(f32,f32);
f32 dreadzone_direction_max(f32,f32);
void dreadzone_direction_matrix(const u8 *,const DirectionVector *,DirectionVector *);
void dreadzone_direction_limit_xy(DirectionVector *);
void dreadzone_direction_normalize(DirectionVector *);
f32 dreadzone_direction_length_xy(const DirectionVector *);
#endif
