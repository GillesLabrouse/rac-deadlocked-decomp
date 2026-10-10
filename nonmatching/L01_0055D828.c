/* cflags: -ffast-math */
/* asflags: --float-immediates */
/* Complete 72-byte wrapped sum, NONMATCHING. One revolution only. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_direction.h"
#endif
#include "../include/dreadzone_actor_direction_internal.h"
f32 func_L01_0055D828(f32 a,f32 b) {
    f32 value=a+b;
    if(value>=3.1415927410125732421875f) {
        value-=3.1415927410125732421875f;value-=3.1415927410125732421875f;
    } else if(value< -3.1415927410125732421875f) {
        value+=3.1415927410125732421875f;value+=3.1415927410125732421875f;
    }
    return value;
}
