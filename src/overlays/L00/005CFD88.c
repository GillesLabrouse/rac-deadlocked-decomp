#include "common.h"

s32 func_L00_005CFD88(s32 a0, s32 a1, s32 a2) {
    u32 result = (u32)a2 + (u32)a0;
    result += (u32)a1;
    result -= 1;
    result &= 0U - (u32)a1;
    return (s32)result;
}
