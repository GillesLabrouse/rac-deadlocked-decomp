#include "common.h"

extern s32 D_00167BC0_[];

s32 func_00139AB8(u32 index) {
    if (index >= 10) {
        return 0;
    }
    return D_00167BC0_[index];
}