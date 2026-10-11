#include "common.h"

extern void *func_00120DA0(void *allocation, u32 bytes);
extern void *func_00120378(void *output);
extern s32 D_0021DFC8;

void *func_0015DB08(void *allocation, u32 bytes) {
    s32 context[10];
    void *result = func_00120DA0(allocation, bytes);
    func_00120378(context);
    D_0021DFC8 = context[7];
    return result;
}
