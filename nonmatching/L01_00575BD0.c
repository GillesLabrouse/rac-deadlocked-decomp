#include "common.h"

/* Complete actual L01 00575BD0..00575BE0 (16 bytes).
 * Publish the secondary armor-definition guest pointer, not actor.half74. */
#ifndef DEADLOCKED_NATIVE
extern u8 *D_001DFC84 __attribute__((section(".data")));
void func_L01_00575BD0(u8 *actor)
{
    *(u8 **)(actor+0x24)=D_001DFC84;
}
#endif
