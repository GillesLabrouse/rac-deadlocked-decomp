#include "common.h"
#ifndef DEADLOCKED_NATIVE
/* Complete 72-byte predicate includes its second return. */
s32 func_L01_00673A00(u8 *player) {
    s32 state;
    if((u32)(*(u32 *)(player+0x25D4)-17U)<2U) return 1;
    state=*(s32 *)(player+0x25CC);
    if(state==106 || state==129 || state==117 || state==116) return 1;
    return 0;
}
#endif
