/* cflags: -mno-split-addresses */
#include "common.h"

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_model_pool.h"
#else
extern u32 D_00222D7C __attribute__((section(".data")));
extern u32 D_00222D80 __attribute__((section(".data")));
extern s32 D_00222D84 __attribute__((section(".data")));
extern s32 D_00222D88;
extern s32 D_00222D8C __attribute__((section(".data")));
extern s32 D_00222D90 __attribute__((section(".data")));
extern s32 D_00222D94 __attribute__((section(".data")));
#endif

void func_L01_0058B050(void) {
    D_00222D80 = D_00222D7C;
    D_00222D84 = 0x3094;
    D_00222D94 = 0x7FFFFFFF;
    D_00222D8C = 0;
    D_00222D90 = 0;
    D_00222D88 = 0;
}
