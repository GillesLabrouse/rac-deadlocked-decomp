/* cflags: -ffast-math */
/* asflags: --float-immediates */
/* Complete 176-byte direction-sensitive wrapped error, NONMATCHING. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_direction.h"
#endif
#include "../include/dreadzone_actor_direction_internal.h"
f32 func_L01_00595188(s32 direction,f32 target,f32 current) {
    f32 difference=func_L01_0055D870(target,current);
    if(!direction || difference*(f32)direction>0.f) return difference;
    if(D_ABS(difference)<=0.001745329238474369049072265625f) return 0.f;
    return difference>0.f?difference-6.283185482025146484375f:
                         difference+6.283185482025146484375f;
}
