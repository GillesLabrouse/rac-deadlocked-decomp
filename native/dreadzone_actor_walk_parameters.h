#ifndef DEADLOCKED_DREADZONE_ACTOR_WALK_PARAMETERS_H
#define DEADLOCKED_DREADZONE_ACTOR_WALK_PARAMETERS_H
#include "dreadzone_actor_direction.h"

/* Complete selected state-two mode-zero path of original 00681B30. This is
 * an upstream scalar-parameter update, not clip selection, frame or locomotion.
 * Uses real history 005BD868, getter 6739B0, wrap 55D870 and progression 594AE8.
 *
 * Input context/backings must be real, stable, exclusive, nonwrapping and
 * mutually disjoint. At most 64 views / 16MiB total. Descriptors cannot overlap
 * backing storage. Guest aliases resolve on copied RAM; writable full images
 * publish only after successful completion. Refusal leaves all RAM unchanged.
 * State 25CC must be 2, mode 2659 zero. Actual selected history and globals
 * 21DDEC/21DDF4 must be mapped. Only selected floats are checked: magnitude
 * 0..1; scale/time/global factors 0..16 with signed zero or abs>=2^-20; selected
 * angles abs<=pi, angular speed abs<=2pi with signed zero or abs>=2^-40. The
 * progression branch accepts counter 370 in 0..39; values>=40 follow its actual
 * bypass. Division by zero and exceptional/subnormal output refuse.
 * Arithmetic uses original operation order with approximate host binary32;
 * PS2 FPU/VU precision, flags and timing are not proved. Unsupported states,
 * spherical projection and their C78/C88 paths retain complete PS2 sources.
 */
int dreadzone_actor_walk_parameters_00681B30(const DreadzoneActorInputContext *);

/* Internal calls require the validated copied context. */
typedef struct DreadzoneWalkParametersRun {
    const DreadzoneActorInputContext *context;
    int status;
} DreadzoneWalkParametersRun;
int dreadzone_walk_parameters_execute(DreadzoneWalkParametersRun *);
f32 dreadzone_walk_parameters_float(DreadzoneWalkParametersRun *,u32 offset);
f32 dreadzone_walk_parameters_global(DreadzoneWalkParametersRun *,u32 address);
void dreadzone_walk_parameters_store(DreadzoneWalkParametersRun *,u32 offset,f32);
f32 dreadzone_walk_parameters_divide(DreadzoneWalkParametersRun *,f32,f32);
s32 dreadzone_walk_parameters_history(DreadzoneWalkParametersRun *,u32 input);
s32 dreadzone_walk_parameters_word(const u8 *);
#endif
