#include "common.h"

/* Entire 00667050..00667058: the actual return delay slot stores the raw
 * guest record address in actor+90. Neither argument is a host pointer in a
 * native ABI; this PS2 source deliberately retains its 32-bit store. */
#ifndef DEADLOCKED_NATIVE
void func_L01_00667050(u8 *record,u8 *actor)
{
    *(u32 *)(actor+0x90)=(u32)record;
}
#endif
