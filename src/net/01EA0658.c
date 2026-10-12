#include "common.h"

extern u32 D_00167EFC[];

void func_01EA0658(u32 value) {
    u32 *context = (u32 *)D_00167EFC[0];
    context[4] = value;
}
