#include "common.h"

s32 func_00127348(void *unused, void *record) {
    *(u64 *)((u8 *)record + 0x48) = 0;
    *(u32 *)((u8 *)record + 4) = 0x2000;
    return 0;
}