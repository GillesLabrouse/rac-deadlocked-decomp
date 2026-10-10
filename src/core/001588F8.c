#include "common.h"

extern s32 func_00158A30(s32, s32, void *);

s32 func_001588F8(s32 value) {
    return func_00158A30(0x19, sizeof(value), &value);
}
