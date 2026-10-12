#include "common.h"

extern u32 D_0021D940[];

s32 func_00163928(void) {
    s32 result = 0;
    if (D_0021D940[1] != 0 || (D_0021D940[2] & 1) != 0) {
        result = 1;
    }
    return result;
}
