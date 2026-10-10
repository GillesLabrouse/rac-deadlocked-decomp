#include "common.h"

extern s32 func_00158C60(s32, s32, void *, void (*)(s32, s64), s64);

s32 func_001585F8(s32 first, s32 second) {
    s32 values[2];
    values[0] = first;
    values[1] = second;
    return func_00158C60(9, sizeof(values), values, NULL, 0);
}
