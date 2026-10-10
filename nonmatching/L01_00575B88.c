#include "common.h"

/* Complete actual L01 00575B88..00575BCC (68 bytes).
 * Actual slot is D001DFC60+20, not the subsequent slot at+40.
 * Disc/GPU loading remains a genuine dependency, with no native success entry. */
#ifndef DEADLOCKED_NATIVE
extern u8 D_001DFC60[] __attribute__((section(".data")));
void func_L01_005756D0(s32,s32);
void func_L01_00575A18(u8 *,u8 *);
void func_L01_00575B88(s32 index)
{
    u8 *slot=D_001DFC60+0x20;
    if(index!=*(s32 *)slot) {
        func_L01_005756D0(index,1);
        func_L01_00575A18(*(u8 **)(slot+4),*(u8 **)(slot+0xC));
    }
}
#endif
