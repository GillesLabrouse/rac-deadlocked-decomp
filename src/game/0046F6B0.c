/* cflags: -mno-split-addresses */
#include "common.h"

extern u32 D_00221454 __attribute__((section(".data")));
extern u32 D_00221458 __attribute__((section(".data")));
extern s32 D_0022145C __attribute__((section(".data")));
extern s32 D_00221460;
extern s32 D_00221464 __attribute__((section(".data")));
extern s32 D_00221468 __attribute__((section(".data")));
extern s32 D_0022146C __attribute__((section(".data")));

void func_0046F6B0(void) {
    D_00221458 = D_00221454;
    D_0022145C = 0x3094;
    D_0022146C = 0x7FFFFFFF;
    D_00221464 = 0;
    D_00221468 = 0;
    D_00221460 = 0;
}
