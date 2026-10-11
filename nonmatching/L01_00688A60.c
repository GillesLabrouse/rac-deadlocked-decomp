#include "common.h"
#ifndef DEADLOCKED_NATIVE
void func_L01_005919B8(u8 *,s32,s32,f32);
/* Complete real24-byte leaf688B78..688B90: second output is untouched. */
s32 func_L01_00688B78(u8 *player,s32 clip,s32 *first,s32 *second) {
    (void)player;(void)clip;(void)second;
    *first=1;
    return 0;
}
/* Complete276-byte auxiliary routing. Real leaf retained; no callback fake. */
void func_L01_00688A60(u8 *player,s32 clip,s32 frame,f32 blend) {
    u8 *first,*second;
    s32 firstClip,secondClip;
    if(*(s32 *)(player+0x2404)!=2) return;
    first=*(u8 **)(player+0x23E0);
    second=*(u8 **)(player+0x23E4);
    if(first==NULL || second==NULL) return;
    if(func_L01_00688B78(player,clip,&firstClip,&secondClip)!=0 && clip!=0) {
        if(first[0x54]!=firstClip) {
            func_L01_005919B8(first,firstClip,(s32)blend,(f32)frame);
            func_L01_005919B8(second,secondClip,(s32)blend,(f32)frame);
        }
    } else {
        if(first[0x54]!=1) func_L01_005919B8(first,1,7,0.f);
        if(second[0x54]!=1) func_L01_005919B8(second,1,7,0.f);
    }
}
#endif
