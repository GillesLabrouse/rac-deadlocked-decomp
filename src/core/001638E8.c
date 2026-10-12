#include "common.h"

extern s32 func_00163928(void);
extern void func_001634F0(f32 value);
extern u32 D_0021D940[];

void func_001638E8(void) {
    if (func_00163928() != 0) {
        D_0021D940[2] |= 4;
        func_001634F0(0.0f);
    }
}
