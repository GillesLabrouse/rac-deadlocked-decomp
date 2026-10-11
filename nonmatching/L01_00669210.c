/* cflags: -mno-split-addresses */
/* asflags: --float-immediates */
#include "common.h"
#ifndef DEADLOCKED_NATIVE
/* Actual health/max-health ratio; no health or divisor is supplied here. */
s32 func_L01_00669210(u8 *player) {
    return *(f32 *)(player+0x2E20) / *(f32 *)(player+0x2F24) <= 0.2f;
}
#endif
