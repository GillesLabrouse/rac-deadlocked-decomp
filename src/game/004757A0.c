/* cflags: -ffast-math -fno-delayed-branch */
#include "common.h"

#ifdef DEADLOCKED_NATIVE
#include <math.h>
#endif

/* Move by the ordered min/max clamp, then report the remaining distance.
 * Native arithmetic is intended for finite values without overflow or
 * subnormal intermediates. Host rounding can differ from the PS2 FPU;
 * exceptional arithmetic and FPU flags are not reproduced.
 */
f32 func_004757A0(f32 *value, f32 target, f32 step) {
    f32 delta = target - *value;
#ifdef DEADLOCKED_NATIVE
    /* Preserve the PS2 min/max ordering of -0 and +0 on finite inputs. */
    delta = step < delta || (step == delta && signbit(step)) ? step : delta;
    delta = -step > delta || (-step == delta && !signbit(-step)) ? -step : delta;
#else
    delta = step < delta ? step : delta;
    delta = -step > delta ? -step : delta;
#endif
    *value += delta;
#ifdef DEADLOCKED_NATIVE
    return fabsf(target - *value);
#else
    return __builtin_fabsf(target - *value);
#endif
}
