#include "common.h"

extern void *func_00120378(void *output);
extern void *func_0011EF60(u32 bytes);
extern s32 D_0021DFC8;

void *func_0015DAC8(u32 bytes) {
    s32 context[10];
    register s32 result __asm__("$2");
    func_00120378(context);
    result = context[7];
    D_0021DFC8 = (s32)((u32)result + bytes);
    return func_0011EF60(bytes);
}
