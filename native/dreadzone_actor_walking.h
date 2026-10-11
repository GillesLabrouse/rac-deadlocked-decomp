#ifndef DEADLOCKED_DREADZONE_ACTOR_WALKING_H
#define DEADLOCKED_DREADZONE_ACTOR_WALKING_H
#include "dreadzone_actor_wall_response.h"
typedef struct DreadzoneActorWalkingRequest {
    DreadzoneWallRequest wall;
    u32 record_guest;
    u32 incoming_vf28_29_w[2];
} DreadzoneActorWalkingRequest;
typedef struct DreadzoneActorWalkingInfo {
    DreadzoneActorWallResponseInfo wall;
    u32 direction_calls,turn_calls,parameter_calls,speed_calls,velocity_calls;
    u32 heading_calls,history_calls,angle_history_calls,height_calls,slope_calls;
} DreadzoneActorWalkingInfo;
enum DreadzoneActorWalkingResult {
    DREADZONE_WALKING_APPROXIMATE=1,DREADZONE_WALKING_INVALID=-1,
    DREADZONE_WALKING_UNSUPPORTED_PATH=-2,DREADZONE_WALKING_UNSUPPORTED_FLOAT=-3,
    DREADZONE_WALKING_MEMORY=-4
};
/* Bounded native approximation of the complete selected L01 walking call.
 * Only state2/mode0 is supported; all original operations in that path run.
 * One exclusive mapped range <=16MiB below2GiB, actual record stride31F0,
 * real body/input/globals/world. No radius, state, clock, grid or contact is
 * supplied by this implementation. Existing bounded recovered dependencies
 * execute. Reached event, special turning or6840B8 paths refuse atomically.
 * The genuine velocity C60 producer clears VF28.W before wall response;
 * remaining caller W words retain the actual dependency publication rules.
 * This is neither exact matching nor a constructor, position or player frame.
 */
int dreadzone_actor_walking_00695CB8(const DreadzoneActorWalkingRequest *,DreadzoneActorWalkingInfo *);
#endif
