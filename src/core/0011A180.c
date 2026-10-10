#include "common.h"

/* Character classification uses the resident byte table. */
extern u8 D_00191F01[];

s32 func_0011A180(s32 value) {
    return (D_00191F01[value] & 1) ? value + 0x20 : value;
}
