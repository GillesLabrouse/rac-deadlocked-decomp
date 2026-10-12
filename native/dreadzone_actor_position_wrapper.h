#ifndef DEADLOCKED_DREADZONE_ACTOR_POSITION_WRAPPER_H
#define DEADLOCKED_DREADZONE_ACTOR_POSITION_WRAPPER_H
#include "dreadzone_actor_position_step.h"
#include "dreadzone_actor_offset_probe.h"
typedef struct DreadzonePositionWrapperRequest {
    u8 *storage;
    u32 guest_base,storage_bytes,record_guest,actor_pool_count,grid_block_count;
    u32 incoming_high64[2],incoming_vf25_29_w[5];
    u32 incoming_ground_side_w[2],incoming_offset_local_w[2];
} DreadzonePositionWrapperRequest;
typedef struct DreadzonePositionWrapperInfo {
    u32 early_exit,position_calls,offset_calls,mod5_resets,angle_calls,object_calls,active_wall_calls;
    u32 outgoing_vf25_29_w[5];
    DreadzonePositionStepInfo position;
    DreadzoneOffsetProbeInfo offset;
    DreadzoneWallInfo active_wall;
} DreadzonePositionWrapperInfo;
/* Selected L01 684A60 RAM composition with bounded approximate native math.
 * Original API/storage identities and higher ownership remain hypotheses.
 * Early exit and true modulo scheduling are retained; otherwise67F5E8 runs
 * before both gates. Genuine GP22597F resolves the frame global21DDB8.
 * Actual caller A0 high64, W and per-call local W are explicit projected
 * component inputs; authentic live stack provenance remains separate.
 * Mode-zero offset hits execute actual raw inclination and object predicate; nonnull2F0
 * near active wall contacts execute; reached sphere branches refuse atomically. No fake contacts/results.
 * Nested established domains apply; <=16MiB writable prefix with original
 * higher RAM retained by the caller. Host byte alignment is arbitrary; guest
 * record/load/store alignment is checked. Request may alias backing and is
 * captured before writes; info is disjoint from both. Reached effects run on
 * a shadow image and commit only on complete supported return. All failures
 * preserve backing/info. Guest aliases follow ordered original stores/reads.
 * Nearest host binary32 and gradual underflow are required. Consumed values
 * and all executed intermediate/unused squares must be finite normal/zero;
 * nested helper angle/position/vector/clock/table domains still apply.
 * Original call-stack locals are separate from mapped backing; the wrapper
 * locally reproduces their values but does not publish the physical stack.
 * Actual GP22597F/global addresses and actual L01 geometry/actor counts are
 * inputs, not synthesized startup/resource binding. A genuine wall miss or
 * distance>3 reaches an unsupported sphere path and refuses atomically.
 * Status is an adapter result, not the original void return. Full GPR/FPR/
 * VF/VI/ACC/Q output, VU status/timing, physical PS2 rounding and a complete
 * 67C010/frame/control/scene are excluded. Only reported W lanes are exposed.
 */
int dreadzone_actor_position_wrapper_00684A60(const DreadzonePositionWrapperRequest *,
    DreadzonePositionWrapperInfo *);
#endif
