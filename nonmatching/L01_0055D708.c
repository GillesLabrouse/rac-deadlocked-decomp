/* cflags: -mno-split-addresses */
/* asflags: --float-immediates */
/* NONMATCHING: scalar recovery of the handwritten L01 VU polynomial.
 * Includes its zero-input return branch at 0055D818. No inline assembly.
 * Native float operations preserve the recovered powers and addition order,
 * but do not reproduce VU rounding, ACC precision, or status flags.
 * Tested domain: finite controller inputs in [-1,1], excluding subnormals.
 */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_input.h"
/* Independently read numeric constants from the user's actual L01 overlay.
 * Coefficients at 003B9540, quadrant scale/offset pairs at 003B9580.
 */
static const f32 D_003B9540[8] = {
    0.999999344348907470703125f,
    -0.33329856395721435546875f,
    0.1994653642177581787109375f,
    -0.13908533751964569091796875f,
    0.096420042216777801513671875f,
    -0.05590988695621490478515625f,
    0.0218612290918827056884765625f,
    -0.0040540578775107860565185546875f
};
static const f32 D_003B9580[16] = {
    1.0f, 0.0f, -1.0f, 1.57079637050628662109375f,
    -1.0f, 0.0f, 1.0f, -1.57079637050628662109375f,
    -1.0f, 3.1415927410125732421875f, 1.0f, 1.57079637050628662109375f,
    1.0f, -3.1415927410125732421875f, -1.0f, -1.57079637050628662109375f
};
#else
extern const f32 D_003B9540[];
extern const f32 D_003B9580[];
#endif

f32 func_L01_0055D708(f32 x, f32 y) {
    union { f32 f; u32 u; } original_x, original_y, difference;
    f32 smaller, larger, ratio, square, fourth, eighth;
    f32 odd[4], high[4], sum;
    u32 pair;
    s32 i;

    original_x.f = x;
    original_y.f = y;
    smaller = x < 0.0f ? -x : x;
    larger = y < 0.0f ? -y : y;
    /* Retail indexes by the sign bit of |x|-|y|, then chooses min/max. */
    difference.f = smaller - larger;
    pair = (difference.u >> 31) * 2 + (original_y.u >> 31) * 4 +
           (original_x.u >> 31) * 8;
    if ((difference.u >> 31) == 0) {
        smaller = y < 0.0f ? -y : y;
        larger = x < 0.0f ? -x : x;
    }
    if (larger <= 0.0f) return 0.0f; /* actual branch 0055D818 */
    ratio = (smaller - larger) / (smaller + larger);
    square = ratio * ratio;
    fourth = square * square;
    eighth = fourth * fourth;
    odd[0] = ratio;
    odd[1] = ratio * square;
    odd[2] = ratio * fourth;
    odd[3] = (ratio * fourth) * square;
    for (i = 0; i < 4; i++) {
        high[i] = (odd[i] * eighth) * D_003B9540[i + 4];
        odd[i] *= D_003B9540[i];
    }
    sum = odd[0] + odd[1];
    sum += odd[2];
    sum += odd[3];
    sum += high[0];
    sum += high[1];
    sum += high[2];
    sum += high[3];
    sum = 0.785398185253143310546875f + sum;
    return sum * D_003B9580[pair] + D_003B9580[pair + 1];
}
