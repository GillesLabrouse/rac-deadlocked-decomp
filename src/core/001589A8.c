#include "common.h"

extern s32 func_00158C60(s32, s32, void *, void (*)(s32, s64), s64);

s32 func_001589A8(s32 first, s32 second, s32 third, s32 fourth, s32 fifth, s32 sixth, void (*callback)(s32, s64), s64 context) {
    s32 values[6];
    values[0] = first;
    values[1] = second;
    values[2] = third;
    values[3] = fourth;
    values[4] = fifth;
    values[5] = sixth;
    return func_00158C60(0x21, sizeof(values), values, callback, context);
}
