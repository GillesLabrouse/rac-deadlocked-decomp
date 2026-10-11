#include "common.h"

/* Record backlink; actual L01 00667050 uses the same complete actor+90 store. */
void func_00511310(u8 *record, u8 *actor) {
    *(u32 *)(actor + 0x90) = (u32)record;
}
