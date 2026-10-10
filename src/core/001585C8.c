#include "common.h"

extern s32 func_00158C60(s32, s32, void *, void (*)(s32, s64), s64);

s32 func_001585C8(s32 value, void (*callback)(s32, s64), s64 context) {
    return func_00158C60(6, sizeof(value), &value, callback, context);
}
