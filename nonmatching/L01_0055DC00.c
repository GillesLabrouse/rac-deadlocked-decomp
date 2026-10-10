#include "common.h"

/* Actual 64-byte handle lookup. Reserved index63 never reads a pool record;
 * generation is compared to the signed halfword, not its unsigned encoding. */
#ifndef DEADLOCKED_NATIVE
extern u8 *volatile D_0021F710;
u8 *func_L01_0055DC00(s32 handle) {
    u8 *base=D_0021F710;
    u32 low=(u32)handle&0xFFFFU;
    u32 index=low&63U;
    u8 *record;
    if(index==63U) return NULL;
    record=base+(index<<7);
    if((s32)(low>>6)==*(s16 *)(record+0x58)) return record;
    return NULL;
}
#endif
