#ifndef DEADLOCKED_DREADZONE_ACTOR_SPEED_TARGET_H
#define DEADLOCKED_DREADZONE_ACTOR_SPEED_TARGET_H
#include "dreadzone_actor_direction.h"

/* Complete selected CPU calls, using the direction context and its result codes.
 * Stable, disjoint, nonwrapping host/guest mappings are required. All selected
 * reads occur on shadow RAM and writable images publish only after completion.
 * Actor2659 and the genuine mapped global21E878 must be zero. Actual input,
 * body matrix, controller history and selected globals remain prerequisites.
 *
 * CAF8 calls real direction(scale1, cardinal0), then reads the mapped constant
 * 003B01A0, time factor21DDF4 and actor25C4/2E08/2FC0 to produce target1780.
 * D100 derives the three real turn arguments from actor/global state and calls
 * real CFD0(mode0). Neither interface accepts a supplied target or turn rate.
 * Scalar inputs are signed zero or abs>=2^-20 and abs<=16. Derived turn rates
 * must satisfy the existing turn interface's 0..1 domain. Every intermediate
 * must be finite normal/zero. Unsupported paths refuse before publication.
 * Direction may produce a normal target angle below the inherited turn read
 * bound; that subsequent turn call refuses instead of replacing it with zero.
 * Separate calls retain the completed prefix when a later call is refused.
 * Ordered host binary32 remains approximate: physical FPU/VU/ACC precision,
 * flags and scheduling are excluded. No state transition/frame/position or
 * collision completion is claimed by these two original helper calls.
 */
int dreadzone_actor_speed_target_0067CAF8(const DreadzoneActorInputContext *);
int dreadzone_actor_state_turn_0067D100(const DreadzoneActorInputContext *);
/* Internal entries require a validated shadow context. */
int func_L01_0067CAF8(const DreadzoneActorInputContext *);
int func_L01_0067D100(const DreadzoneActorInputContext *);
int dreadzone_speed_target_read(const DreadzoneActorInputContext *,u32,f32 *);
int dreadzone_speed_target_scalar(f32,f32 *);
int dreadzone_speed_target_product(f32,f32,f32 *);
#endif
