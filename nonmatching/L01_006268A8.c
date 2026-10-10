#include "common.h"

/* Complete actual 48-byte lookup/release adapter. */
#ifndef DEADLOCKED_NATIVE
u8 *func_L01_0055DC00(s32);
void func_L01_006268D8(u8 *);
void func_L01_006268A8(s32 handle) {
    u8 *record=func_L01_0055DC00((s16)handle);
    if(record!=NULL) func_L01_006268D8(record);
}
#endif
