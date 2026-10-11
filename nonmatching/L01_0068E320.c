/* cflags: -mno-split-addresses */
/* asflags: --float-immediates */
#include "common.h"
#ifndef DEADLOCKED_NATIVE
/* Actual field scalar producer, not an animation or frame substitute. */
void func_L01_0068E320(u8 *player) {
    s32 mode=*(s32 *)(player+0x1AE0),state;
    f32 value;
    if(mode==1) {
        *(f32 *)(player+0x1AFC)=2.f;
        return;
    }
    state=*(s32 *)(player+0x25CC);
    if(((state==4 || state==21) && mode==2) || player[0x2ECD]!=0) value=0.6f;
    else value=1.15f;
    *(f32 *)(player+0x1AFC)=value;
    if(*(s32 *)(player+0x25D4)==3) *(f32 *)(player+0x1AFC)=1.75f;
}
#endif
