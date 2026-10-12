#include "dreadzone_movement_basis.h"
#include "dreadzone_angle_pair.h"
#include <float.h>
#include <math.h>
#include <fenv.h>
#include <string.h>

_Static_assert(sizeof(float) == 4 && FLT_RADIX == 2 && FLT_MANT_DIG == 24,
               "Movement basis requires IEEE binary32 float");

static int finite_normal(float value) {
    return isfinite(value) && (value == 0.0f || fabsf(value) >= FLT_MIN);
}
static int gradual_underflow(void) {
    volatile float minimum = FLT_MIN, half = 0.5f;
    volatile float result = minimum * half;
    float value = result;
    uint32_t word, tiny_word = 0x00400000;
    volatile float tiny, twice = 2.0f;
    memcpy(&word, &value, sizeof(word));
    if (word != tiny_word) return 0;
    memcpy(&value, &tiny_word, sizeof(value));
    tiny = value;
    result = tiny * twice;
    value = result;
    memcpy(&word, &value, sizeof(word));
    return word == 0x00800000;
}
static int multiply(float left, float right, float *output) {
    volatile float value = left * right;
    *output = value;
    return finite_normal(value);
}
static int add(float left, float right, float *output) {
    volatile float value = left + right;
    *output = value;
    return finite_normal(value);
}
static int subtract(float left, float right, float *output) {
    volatile float value = left - right;
    *output = value;
    return finite_normal(value);
}
#define MUL(a,b,d) do { if (!multiply(a,b,&(d))) return DREADZONE_MOVEMENT_BASIS_UNSUPPORTED; } while (0)
#define ADD(a,b,d) do { if (!add(a,b,&(d))) return DREADZONE_MOVEMENT_BASIS_UNSUPPORTED; } while (0)
#define SUB(a,b,d) do { if (!subtract(a,b,&(d))) return DREADZONE_MOVEMENT_BASIS_UNSUPPORTED; } while (0)

int dreadzone_movement_basis_before_locomotion(uint8_t *record, size_t bytes) {
    float angles[3], pair[3][2], matrix[4][4] = {{0}};
    float t, u, v, w, a, b, c, d, e, f, g;
    uint8_t staged[128];
    unsigned row, column;
    if (!record || bytes < 0xBC || (uintptr_t)record > UINTPTR_MAX - bytes)
        return DREADZONE_MOVEMENT_BASIS_INVALID;
    if (fegetround() != FE_TONEAREST || !gradual_underflow())
        return DREADZONE_MOVEMENT_BASIS_UNSUPPORTED;
    memcpy(angles, record + 0xB0, sizeof(angles));
    for (row = 0; row < 3; ++row)
        if (dreadzone_angle_pair(angles[row], pair[row]) != DREADZONE_ANGLE_APPROXIMATE)
            return DREADZONE_MOVEMENT_BASIS_UNSUPPORTED;

    /* Retain the actual COP1 order at 284/288, 29C/2A4 and 2C8..32C. */
    MUL(pair[0][0], pair[1][0], t); MUL(pair[0][1], pair[1][0], u);
    MUL(pair[1][1], pair[0][0], v); MUL(pair[0][1], pair[1][1], w);
    MUL(t, pair[2][0], a); MUL(pair[0][1], pair[2][0], b);
    MUL(pair[2][1], t, c); MUL(pair[0][1], pair[2][1], d);
    MUL(u, pair[2][0], e); MUL(-pair[2][1], pair[0][0], f);
    MUL(pair[0][0], pair[2][0], g); MUL(u, pair[2][1], u);
    SUB(c, b, matrix[1][0]); ADD(d, a, matrix[1][1]);
    ADD(u, g, matrix[2][0]); ADD(f, e, matrix[2][1]);
    MUL(pair[1][1], pair[2][1], matrix[0][0]);
    MUL(pair[1][1], pair[2][0], matrix[0][1]);
    matrix[0][2] = -pair[1][0];
    matrix[1][2] = v; matrix[2][2] = w; matrix[3][3] = 1.0f;

    memcpy(staged, matrix, sizeof(matrix));
    for (row = 0; row < 4; ++row)
        for (column = 0; column < 4; ++column)
            memcpy(staged + 64 + row * 16 + column * 4,
                   &matrix[column][row], sizeof(float));
    /* Include 67C3B4, the genuine final column store, before callee execution. */
    memcpy(record + 0x20, staged, sizeof(staged));
    return DREADZONE_MOVEMENT_BASIS_APPROXIMATE;
}
