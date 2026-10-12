#ifndef DEADLOCKED_DREADZONE_ACTOR_IDLE_H
#define DEADLOCKED_DREADZONE_ACTOR_IDLE_H
#include "dreadzone_actor_wall_response.h"
#include "dreadzone_actor_motion_probe.h"
typedef struct DreadzoneActorIdleRequest {
    DreadzoneWallRequest wall;
    u32 record_guest,gp_guest,incoming_vf28_29_w[2];
} DreadzoneActorIdleRequest;
typedef struct DreadzoneActorIdleInfo {
    DreadzoneMotionProbeInfo motion;
    DreadzoneActorWallResponseInfo wall;
    u32 speed_calls,accumulation_calls,height_calls;
} DreadzoneActorIdleInfo;
/* Complete selected 695570 state0 path, not the entire state machine. Requires
 * mode0, states25D8/25CC=0, zero2661/2ECE/3BA/2ECD/437/2FE and a nonzero actual
 * config byte171DE0+index (index0..1). Planar130 must meet motion-probe domain;
 * accumulated1A0 must contain signed zeros in every lane, preserving incoming
 * VF28/29W without manufacturing a bank producer. All real nested calls run.
 * Raw1970/19B0 transport has no float-domain restriction. GP must be22597F
 * for the nested APIs' actual L01 global bindings. The ordered GP-relative
 * and absolute timestep reads remain separate; scale/time
 * inputs have normal/zero binary32 magnitude<=16, nonzero>=2^-40.
 * Products must be normal/zero; nested APIs impose rate/offset bounds. The height
 * API imposes its own offset floor2^-20. Nearest rounding/gradual underflow
 * required. Hardware FCSR/VU flags, precision, timing and full banks excluded.
 * One exclusive writable range<=16MiB below10000000 separates original stack;
 * record16 alignment/31F0 bytes and actual wall mappings required if reached.
 * Request may alias backing; info is disjoint from request/backing. All writes
 * publish only after complete success. No original API/type ownership, normalized
 * original return, matching credit, initialized player or playable frame implied. */
int dreadzone_actor_idle_00695570(const DreadzoneActorIdleRequest *,DreadzoneActorIdleInfo *);
#endif
