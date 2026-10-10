#include "common.h"

/* Complete L01 model-update orchestrator, 0058A648..0058A69C, plus4 padding
 * bytes before0058A6A0. The return delay slot is at0058A698.
 * The two callees perform the actual scratchpad animation/vector work and
 * launch the final fromSPR DMA. This function does not wait for that DMA.
 * No native body is supplied: clearing the dirty bit without those effects
 * would incorrectly report a completed model update. The caller ignores v0;
 * the retail null-actor path leaves its incoming value unchanged.
 */
#ifndef DEADLOCKED_NATIVE
void func_L01_0058A440(u8 *actor, u8 *scratch);
void func_L01_00589950(u8 *destination, s32 count, u8 *unused_scratch);

void func_L01_0058A648(u8 *actor) {
    if (actor != NULL && *(u32 *)(actor + 0x58) != 0) {
        func_L01_0058A440(actor, (u8 *)0x70000000);
        func_L01_00589950((u8 *)*(u32 *)(actor + 0x58), actor[0x57],
            (u8 *)0x70000000);
        actor[0x55] &= 0xDF;
    }
}
#endif
