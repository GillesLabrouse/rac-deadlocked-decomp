#ifndef DEADLOCKED_DREADZONE_INPUT_CONTEXT_INIT_H
#define DEADLOCKED_DREADZONE_INPUT_CONTEXT_INIT_H

#include "dreadzone_input_callback.h"

/* Real virtual reset at L01 006A6598 operates on the complete actor record.
 * The word view supplies its real 0055D2A8 dependency without host pointers;
 * the structured view exposes only independently verified accessed fields.
 */
typedef union DreadzoneActorInit31F0 {
    u32 words[0x31F0 / 4];
    struct {
        u8 pad0[0x14];
        u32 f14;
        u8 pad18[0x2655];
        u8 f266D;
        u8 pad266E[0x97E];
        DreadzoneInputContext1E0 f2FEC;
        u8 pad31CC[3];
        u8 f31CF;
        u8 pad31D0[0x20];
    } fields;
} DreadzoneActorInit31F0;

void func_L01_006A6598(DreadzoneActorInit31F0 *);
DreadzoneActorInit31F0 *func_L01_006A93A0(DreadzoneActorInit31F0 *);
void *func_L01_006A9638(void *);

#endif
