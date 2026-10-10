#include "common.h"

/* Complete retail 00589F98..0058A080, 232 bytes. The first crossed event
 * returns through its real dispatcher; the scheduler does not then run. */
#ifndef DEADLOCKED_NATIVE
void func_L01_00589CE0(u8 *);
void func_L01_00586FE0(u8 *);
s32 func_L01_00626D50(s32,s32,u8 *);
extern volatile u32 D_0021DDB8;
void func_L01_00589F98(u8 *actor) {
    u32 oldFrame=*(u32 *)(actor+0x44),newFrame,phase;
    s32 oldFixed,newFixed,threshold,count;
    u8 flags=actor[0x55];
    u8 *event;
    __asm__ volatile("qmtc2.ni %0,$vf16" : : "r"(oldFrame));
    actor[0x55]=flags&0xFC;
    if(actor[0x57]!=0) {
        func_L01_00589CE0(actor);
        count=actor[0x6C];
        __asm__ volatile("vftoi4.x $vf16,$vf16");
        newFrame=*(u32 *)(actor+0x44);
        if(count!=0 && *(s32 *)(actor+0x48)>0) {
            __asm__ volatile("qmtc2.ni %2,$vf17\n\tvftoi4.x $vf17,$vf17\n\t"
                             "qmfc2.ni %0,$vf16\n\tqmfc2.ni %1,$vf17"
                : "=r"(oldFixed),"=r"(newFixed) : "r"(newFrame));
            event=*(u8 **)(*(u8 **)(actor+0x40)+0x14);
            do {
                threshold=*(u16 *)(event+2);
                if(threshold-oldFixed>0 && threshold-newFixed<=0) {
                    func_L01_00626D50(*(u16 *)event,0,actor);
                    return;
                }
                event+=4;
            } while(--count!=0);
        }
    }
    phase=D_0021DDB8;
    if(*(s16 *)(actor+0x6E)!=-1 ||
       (phase&3U)==(((u32)actor>>8)&3U)) func_L01_00586FE0(actor);
}
#endif
