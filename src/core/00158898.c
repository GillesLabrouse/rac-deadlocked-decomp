#include "common.h"

extern s32 func_00158C60(s32, s32, void *, void (*)(s32, s64), s64);

s32 func_00158898(s32 value) {
    return func_00158C60(0x17, sizeof(value), &value, NULL, 0);
}
