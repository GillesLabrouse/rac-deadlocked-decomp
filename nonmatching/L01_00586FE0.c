#include "common.h"

/* Complete retail 00586FE0..00587110, 304 bytes. All actual dependencies
 * remain calls; record ownership, selector and release are not simulated. */
#ifndef DEADLOCKED_NATIVE
extern volatile s32 D_0021E824;
s32 func_L01_00665A28(u8 *);
u8 *func_L01_0055DC00(s32);
void func_L01_006268A8(s32);
s32 func_L01_00626D50(s32,s32,u8 *);
void func_L01_00586FE0(u8 *actor) {
    s32 suppressed=0,active=0;
    u8 *record;
    if(D_0021E824!=1 || func_L01_00665A28(actor)==0)
        suppressed=(*(u16 *)(actor+0x34)&0x40)!=0;
    record=func_L01_0055DC00(*(s16 *)(actor+0x6E));
    if(record!=NULL && *(u8 **)(record+0x1C)==actor) active=record[0x10]!=0;
    if(active) {
        record=func_L01_0055DC00(*(s16 *)(actor+0x6E));
        if(*(u8 **)(record+0x1C)!=actor) {
            *(u16 *)(actor+0x6E)=0xFFFF;
            return;
        }
        if(*(s16 *)(record+0xA)==actor[0x6D] && suppressed==0 &&
           (*(u16 *)(actor+0x74)&0x8000)==0) return;
        func_L01_006268A8(*(s16 *)(actor+0x6E));
        *(u16 *)(actor+0x6E)=0xFFFF;
    } else if(actor[0x6D]!=0xFF && suppressed==0 &&
              (*(u16 *)(actor+0x74)&0x8000)==0 &&
              (*(u16 *)(actor+0x36)&4)==0) {
        *(u16 *)(actor+0x6E)=(u16)func_L01_00626D50(actor[0x6D],4,actor);
    }
}
#endif
