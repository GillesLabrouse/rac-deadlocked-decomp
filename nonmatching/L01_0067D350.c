/* cflags: -ffast-math */
/* Complete original speed progression: 76 bytes plus four-byte padding. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_velocity.h"
#endif
#include "../include/dreadzone_actor_velocity_internal.h"
void func_L01_0067D350(u8 *record,f32 acceleration,f32 deceleration) {
    f32 target=V_FLOAT(record,0x1780),current=V_FLOAT(record,0x1784);
#ifdef DEADLOCKED_NATIVE
    dreadzone_velocity_progress_bytes(record+0x1784,target,
        current<target?acceleration:deceleration);
#else
    func_L01_00594AE8((f32 *)(record+0x1784),target,
        current<target?acceleration:deceleration);
#endif
}
