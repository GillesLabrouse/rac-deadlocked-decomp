#include "common.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

extern f32 func_004757A0(f32 *, f32, f32);
extern void func_0047F538(s32);
extern s32 func_004598E0(void);

s32 D_0021F2E0;
s32 D_0021ED20;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "Check failed at line %d: %s\n", __LINE__, #condition); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

static void progression(f32 initial, f32 target, f32 step,
                        f32 expected_value, f32 expected_remaining) {
    f32 value = initial;
    f32 remaining = func_004757A0(&value, target, step);
    CHECK(value == expected_value);
    CHECK(remaining == expected_remaining);
    CHECK(!signbit(remaining));
}

int main(void) {
    f32 value;
    f32 remaining;
    unsigned calls;

    progression(0.0f, 1.0f, 0.25f, 0.25f, 0.75f);
    progression(1.0f, 0.0f, 0.25f, 0.75f, 0.75f);
    progression(0.875f, 1.0f, 0.25f, 1.0f, 0.0f);
    progression(0.125f, 0.0f, 0.25f, 0.0f, 0.0f);
    progression(1.0f, 1.0f, 0.25f, 1.0f, 0.0f);
    progression(0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    progression(-2.0f, -1.0f, 0.25f, -1.75f, 0.75f);
    progression(0.0f, 1.0f, 2.0f, 1.0f, 0.0f);
    /* Retail applies the two bounds in order even for a negative step. */
    progression(0.0f, -1.0f, -0.25f, 0.25f, 1.25f);

    value = -0.0f;
    remaining = func_004757A0(&value, -0.0f, 0.0f);
    CHECK(value == 0.0f && !signbit(value));
    CHECK(remaining == 0.0f && !signbit(remaining));
    value = -0.0f;
    remaining = func_004757A0(&value, -0.0f, -0.0f);
    CHECK(value == 0.0f && !signbit(value));
    CHECK(remaining == 0.0f && !signbit(remaining));

    /* Use the consumer's actual decimal constants: convergence must terminate
     * at the target despite their inexact binary representation. */
    value = 0.0f;
    remaining = 1.1f;
    for (calls = 0; remaining != 0.0f && calls < 30; ++calls) {
        f32 previous = value;
        remaining = func_004757A0(&value, 1.1f, 0.050000004f);
        CHECK(value >= previous && value <= 1.1f);
        CHECK(remaining >= 0.0f);
    }
    CHECK(calls == 22 && value == 1.1f && remaining == 0.0f);

    func_0047F538(INT_MIN);
    CHECK(D_0021F2E0 == INT_MIN);
    func_0047F538(INT_MAX);
    CHECK(D_0021F2E0 == INT_MAX);
    func_0047F538(-1);
    CHECK(D_0021F2E0 == -1);
    D_0021ED20 = INT_MIN;
    CHECK(func_004598E0() == INT_MIN);
    D_0021ED20 = INT_MAX;
    CHECK(func_004598E0() == INT_MAX);
    D_0021ED20 = 0;
    CHECK(func_004598E0() == 0);

    puts("Native transition progression and scalar access tests passed.");
    return EXIT_SUCCESS;
}
