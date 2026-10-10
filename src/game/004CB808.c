#include "common.h"

void func_00497980(s32 a0);

s32 func_004CD5F0();
extern s32 D_30EAA0_[];
#define D_30EAA0 (D_30EAA0_[0])

void func_004CB808(s32 a0) {
    *((a0 * 0x19) + &D_30EAA0) = 2; /* 0x64-byte stride. */
    func_00497980(6);
    func_004CD5F0();
}
