#include "common.h"

typedef struct {
    u32 reserved0;
    s32 counter;
    u32 reserved8;
    u32 reservedC;
    s32 mode;
} IdentifierState;

extern IdentifierState D_00172160;

s32 func_0015C6E8(s32 category) {
    s32 mode = D_00172160.mode;
    s32 counter = D_00172160.counter;
    D_00172160.counter = (s32)((u32)counter + 1U);
    return (s32)(((u32)mode << 28) | ((u32)category << 24) | (u32)counter);
}
