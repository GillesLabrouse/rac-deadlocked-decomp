#include "common.h"
#ifndef DEADLOCKED_NATIVE
extern s32 D_0021DDB4 __attribute__((section(".sdata")));
/* Full 344-byte transition gate, including the branches beyond first return. */
s32 func_L01_00687068(u8 *player,s32 target,s32 option) {
    s32 state=*(s32 *)(player+0x25CC);
    s32 previous, special=0, base;
    if(state==57 || state==142) special=1;
    else if(state==153) {
        previous=*(s32 *)(player+0x25D8);
        if(previous==57 || previous==142) special=1;
    }
    if(special && target!=122 && target!=126 && target!=123 && target!=118 && target!=148)
        special=0;
    if(!special && *(s32 *)(player+0x25D4)==20 && (target!=99 || option!=2)) return 0;
    if((D_0021DDB4==2 || D_0021DDB4==6) && option!=2) return 0;
    if(special) return 1;
    if(*(f32 *)(player+0x2E20)!=0.f) return 1;
    base=*(s32 *)(player+0x25D4);
    if(base!=2 && base!=7) return 1;
    return target==6 || target==57 || target==118 || target==122 || target==123 ||
           target==126 || target==106 || target==127 || target==129 || target==153 || target==148;
}
#endif
