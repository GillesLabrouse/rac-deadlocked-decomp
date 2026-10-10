/* cflags: -ffast-math */
/* asflags: --float-immediates */
/* Complete 348-byte scalar rotation controller plus four-byte padding.
 * NONMATCHING. Reads angle after the actual speed mutation; pointers may alias.
 * Returns remaining angular error, including the genuine snap-to-target path. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_direction.h"
#endif
#include "../include/dreadzone_actor_direction_internal.h"
#ifdef DEADLOCKED_NATIVE
#define ANGLE dreadzone_direction_float(angle)
#define SPEED dreadzone_direction_float(speed)
#define STORE_ANGLE(f) dreadzone_direction_store_float(angle,(f))
#define STORE_SPEED(f) dreadzone_direction_store_float(speed,(f))
#define PROGRESS dreadzone_direction_progress_bytes
f32 dreadzone_direction_rotation_bytes(u8 *angle,u8 *speed,s32 mode,f32 target,
#else
#define ANGLE (*angle)
#define SPEED (*speed)
#define STORE_ANGLE(f) (*angle=(f))
#define STORE_SPEED(f) (*speed=(f))
#define PROGRESS func_L01_00594A88
f32 func_L01_005952B8(f32 *angle,f32 *speed,s32 mode,f32 target,
#endif
                     f32 force,f32 damping,f32 maximum) {
    f32 error;
    if(mode==2) {
        if((target>0.f && ANGLE<0.f) || (target<0.f && ANGLE>0.f))
            mode=target<0.f?-1:1;
        else mode=0;
    }
    error=func_L01_00595188(mode,target,ANGLE);
    (void)PROGRESS(speed,error,force,damping,maximum);
    STORE_ANGLE(func_L01_0055D828(ANGLE,SPEED));
    error=func_L01_00595188(mode,target,ANGLE);
    if(D_ABS(error)<maximum*0.00999999977648258209228515625f) {
        STORE_ANGLE(target);STORE_SPEED(0.f);return SPEED;
    }
    return error;
}
#ifdef DEADLOCKED_NATIVE
f32 func_L01_005952B8(f32 *angle,f32 *speed,s32 mode,f32 target,f32 force,f32 damping,f32 maximum) {
    return dreadzone_direction_rotation_bytes((u8 *)angle,(u8 *)speed,mode,target,force,damping,maximum);
}
#endif
