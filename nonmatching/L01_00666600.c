/* cflags: -mno-split-addresses */
#include "common.h"

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_byte_fill.h"
#else
extern s32 D_0021DE10 __attribute__((section(".data")));
#endif

s32 func_L01_00666600(void) {
    return ((u32)D_0021DE10 - 0x15U) < 0xFU;
}
