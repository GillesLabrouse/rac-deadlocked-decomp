/* cflags: -mno-split-addresses */
/* asflags: --float-immediates */
/* Actual L01 scalar angular-distance leaf, retail extent 56 bytes.
 * Like retail this wraps once, not repeatedly. Inputs from the angle helper
 * lie in [-pi,pi]; arbitrary larger angular separations are not normalized.
 */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_input.h"
#include <math.h>
#endif

f32 func_L01_0055D910(f32 previous, f32 current) {
    f32 difference = previous - current;
    f32 pi = 3.1415927410125732421875f;
#ifdef DEADLOCKED_NATIVE
    difference = fabsf(difference);
#else
    difference = __builtin_fabsf(difference);
#endif
    if (!(difference < pi)) difference = (pi + pi) - difference;
    return difference;
}
