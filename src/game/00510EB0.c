/* cflags: -mno-split-addresses */
#include "common.h"

#ifdef DEADLOCKED_NATIVE
extern s32 D_0021DE10;
#else
extern s32 D_0021DE10 __attribute__((section(".data")));
#endif

s32 func_00510EB0(void) {
    return ((u32)D_0021DE10 - 0x15U) < 0xFU;
}
