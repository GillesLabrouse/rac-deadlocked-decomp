#include "common.h"

extern volatile u32 D_002206E8;
extern void func_004535E0(u32 amount);

void func_004E7830(u32 mask) {
    while ((D_002206E8 & mask) != 0) {
        func_004535E0(0x400);
    }
}
