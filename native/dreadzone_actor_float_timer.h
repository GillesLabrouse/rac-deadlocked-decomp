#ifndef DREADZONE_ACTOR_FLOAT_TIMER_H
#define DREADZONE_ACTOR_FLOAT_TIMER_H
#include "common.h"
typedef struct DreadzoneActorFloatTimerRequest {
    u8 *storage;
    u32 guest_base,storage_bytes,timer_guest,gp_guest;
} DreadzoneActorFloatTimerRequest;
typedef struct DreadzoneActorFloatTimerInfo {s32 returned;u32 stored_word;} DreadzoneActorFloatTimerInfo;
/* Complete L01 55D240 RAM/scalar effects in a bounded approximate binary32
 * domain. The real incoming GP determines the threshold read; no fixed clock
 * is substituted. No full-GPR/FPU flags, physical rounding or frame claim. */
int dreadzone_actor_float_timer_0055D240(const DreadzoneActorFloatTimerRequest *,DreadzoneActorFloatTimerInfo *);
#endif
