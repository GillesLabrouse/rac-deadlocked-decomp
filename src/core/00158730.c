#include "common.h"

extern s32 func_00158C60(s32, s32, void *, void (*)(s32, s64), s64);

s32 func_00158730(s32 value) {
    return func_00158C60(0x64, sizeof(value), &value, NULL, 0);
}
