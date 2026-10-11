/* cflags: -mno-split-addresses */
/* Complete original 00543028..00543050, 40 bytes, NONMATCHING.
 * The sign test observes the complete live packet word before masking. */
#include "common.h"
#ifndef DEADLOCKED_NATIVE
extern u8 D_00242340[];
s32 func_L01_00543028(void) {
    s32 flags=*(volatile s32 *)(D_00242340+0x1C);
    return flags<0 ? -1 : (s32)(((u32)flags&0x60U)>>5);
}
#endif
