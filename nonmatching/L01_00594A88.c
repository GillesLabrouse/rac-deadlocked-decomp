/* cflags: -ffast-math */
/* Complete 88-byte damped scalar progression, NONMATCHING. Ordered stores. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_direction.h"
#endif
#include "../include/dreadzone_actor_direction_internal.h"
#ifdef DEADLOCKED_NATIVE
#define VALUE dreadzone_direction_float(value)
#define STORE(v) dreadzone_direction_store_float(value,(v))
f32 dreadzone_direction_progress_bytes(u8 *value,f32 target,f32 force,f32 damping,f32 maximum) {
#else
#define VALUE (*value)
#define STORE(v) (*value=(v))
f32 func_L01_00594A88(f32 *value,f32 target,f32 force,f32 damping,f32 maximum) {
#endif
    f32 bound=D_ABS(target),current=D_MIN(VALUE,bound);
    current=D_MAX(current,-bound);
    force=force*target;damping=damping*current;
    force=force-damping;
    STORE(current+force);
    if(maximum>0.f) {
        current=D_MIN(VALUE,maximum);current=D_MAX(current,-maximum);
        STORE(current);
    }
    current=D_MIN(VALUE,bound);current=D_MAX(current,-bound);
    STORE(current);return current;
}
#ifdef DEADLOCKED_NATIVE
f32 func_L01_00594A88(f32 *value,f32 target,f32 force,f32 damping,f32 maximum) {
    return dreadzone_direction_progress_bytes((u8 *)value,target,force,damping,maximum);
}
#endif
