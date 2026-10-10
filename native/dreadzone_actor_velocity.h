#ifndef DEADLOCKED_DREADZONE_ACTOR_VELOCITY_H
#define DEADLOCKED_DREADZONE_ACTOR_VELOCITY_H
#include "dreadzone_actor_input_bridge.h"

typedef struct VelocityVector { f32 v[4]; } VelocityVector;
enum DreadzoneActorVelocityResult {
    DREADZONE_VELOCITY_APPROXIMATE=1,
    DREADZONE_VELOCITY_INVALID=-1,
    DREADZONE_VELOCITY_NEEDS_MEMORY=-2,
    DREADZONE_VELOCITY_UNSUPPORTED_MODE=-3,
    DREADZONE_VELOCITY_UNSUPPORTED_FLOAT=-4,
    DREADZONE_VELOCITY_TRANSACTION_MEMORY=-5
};
/* Complete selected original calls, with the input bridge's stable, exclusive,
 * disjoint/nonwrapping host and guest views. At most64 views and16MiB total.
 * All writable RAM is published only after supported completion. Guest aliases
 * preserve original ordered reads; host descriptors may not overlap backing.
 * Every entry requires actor2659==0. No world/state branch is invented here.
 *
 * Speed writes1784 using target1780 and actual acceleration/deceleration.
 * Velocity writes130 from speed1784, pitch2D4 and selected heading: caller
 * angle<=pi uses that angle, otherwise uses actorB8 (including original100000
 * sentinel). Actual C60 is evaluated for pitch before heading. This is the
 * genuine producer used by state2 and target-position state100, not a complete
 * state/frame or position update. No collision, animation or elapsed time is
 * supplied by these interfaces.
 *
 * Numerical domain: signed zero or abs>=2^-20, consumed scalar/component
 * magnitudes<=16; selected headings/pitch<=pi, caller angle<=100000. Speed
 * steps and planar requested length are0..16. All arithmetic intermediates
 * and results must be finite normal/zero. Native C60 and VU arithmetic use
 * ordered host binary32 approximations; physical PS2 rounding, ACC precision,
 * exception flags and scheduling are excluded. No tolerant test proves VU
 * identity. Planar projection retains raw sourceZ/W; normalized velocity
 * retains rawW. Squared unused VU lanes/flags are outside this CPU contract.
 * The normalized zero-vector path preserves signs by the genuine saturated
 * positive Q multiplication, rather than an invented zero-vector branch;
 * sign-negative speed (including -0) with zero vector is refused.
 */
int dreadzone_actor_speed_0067D350(const DreadzoneActorInputContext *,f32 acceleration,f32 deceleration);
int dreadzone_actor_velocity_0067D3A0(const DreadzoneActorInputContext *,f32 caller_angle);
int dreadzone_actor_normalized_velocity_0067D480(const DreadzoneActorInputContext *,u32 input_guest);
int dreadzone_actor_planar_projection_0068AC58(const DreadzoneActorInputContext *,u32 output_guest,u32 input_guest,f32 length);
/* Vertical projection preserves raw X/Y/W and computes Z as +0 + height.
 * Accumulated motion consumes actor1A0, projects away Z, reduces planar length
 * by the supplied original scalar, then adds XYZ to actor130 preserving its W.
 * At/below the genuine threshold, all four actor1A0 words are cleared instead.
 * Neither entry writes positionA0 or performs terrain/collision response.
 * The same mapped/transactional and finite bounded numerical domain applies.
 * The reduction scalar is signed; no invented nonnegative clamp is imposed. */
int dreadzone_actor_vertical_projection_0068ABA8(const DreadzoneActorInputContext *,u32 output_guest,u32 input_guest,f32 height);
int dreadzone_actor_accumulated_motion_006834C8(const DreadzoneActorInputContext *,f32 reduction);
/* Mode0 height offset preserves destination X/Y/W, reads the original source
 * Z before any store, and subtracts the original scalar. Ordered aliases and
 * the same finite bounded scalar/domain and transactional rules apply. */
int dreadzone_actor_height_offset_0068A178(const DreadzoneActorInputContext *,u32 output_guest,u32 input_guest,f32 offset);
/* Internal routines require a validated shadow context and supported floats. */
f32 dreadzone_velocity_float(const void *);
void dreadzone_velocity_store(void *,f32);
f32 dreadzone_velocity_min(f32,f32);
f32 dreadzone_velocity_max(f32,f32);
f32 dreadzone_velocity_length(const VelocityVector *,u32 dimensions);
int dreadzone_velocity_scale(VelocityVector *,f32,u32 dimensions,int multiply_zero);
f32 dreadzone_velocity_progress_bytes(u8 *,f32,f32);
f32 func_L01_00594AE8(f32 *,f32,f32);
void func_L01_0067D350(u8 *,f32,f32);
int dreadzone_velocity_00652978(f32,f32,f32,VelocityVector *);
int dreadzone_velocity_0067D3A0(u8 *,f32);
int dreadzone_velocity_0067D480(u8 *,const u8 *);
f32 func_L01_0068A920(const u8 *,const VelocityVector *);
f32 func_L01_0068AB40(const u8 *,const VelocityVector *);
int dreadzone_velocity_0068AC58(const u8 *,u8 *,const u8 *,f32);
int dreadzone_velocity_0068ABA8(const u8 *,u8 *,const u8 *,f32);
int dreadzone_velocity_006834C8(u8 *,f32);
int dreadzone_velocity_0068A178(const u8 *,u8 *,const u8 *,f32);
#endif
