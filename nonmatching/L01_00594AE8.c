/* cflags: -ffast-math */
/* Complete 44-byte original entry plus four-byte padding; NONMATCHING draft. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_velocity.h"
#endif
#include "../include/dreadzone_actor_velocity_internal.h"
#ifdef DEADLOCKED_NATIVE
f32 dreadzone_velocity_progress_bytes(u8 *value,f32 target,f32 step) {
    f32 current=V_FLOAT(value,0),difference=target-current;
    difference=V_MIN(difference,step);
    difference=V_MAX(difference,-step);
    current=current+difference;
    target=target-current;
    V_STORE(value,0,current);
    return V_ABS(target);
}
f32 func_L01_00594AE8(f32 *value,f32 target,f32 step) {
    return dreadzone_velocity_progress_bytes((u8 *)value,target,step);
}
#else
f32 func_L01_00594AE8(f32 *value,f32 target,f32 step) {
    f32 current=*value,difference=target-current;
    difference=V_MIN(difference,step);
    difference=V_MAX(difference,-step);
    current=current+difference;
    target=target-current;
    *value=current;
    return V_ABS(target);
}
#endif
