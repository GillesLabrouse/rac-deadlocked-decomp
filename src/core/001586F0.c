#include "common.h"

extern s32 func_00158C60(s32, s32, void *, void (*)(s32, s64), s64);

s32 func_001586F0(s32 first, s32 second, s32 third) {
    s32 values[3];
    values[0] = first;
    values[1] = second;
    values[2] = third;
    return func_00158C60(0x4E, sizeof(values), values, NULL, 0);
}
