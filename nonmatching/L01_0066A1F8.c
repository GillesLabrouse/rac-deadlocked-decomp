#include "common.h"
#ifndef DEADLOCKED_NATIVE
u8 *func_L01_0055DC00(s16);
void func_L01_006268A8(s32);
/* Full owner-aware release. Query reads LH; release rereads the entire LW. */
void func_L01_0066A1F8(u8 *player) {
    u8 *slot=player+0x2D30;
    s32 index=0;
    do {
        u32 owner=*(u32 *)(slot+0x24);
        u8 *record;
        if(owner==0) owner=*(u32 *)(player+0x2EF0);
        record=func_L01_0055DC00(*(s16 *)slot);
        if(record!=NULL && owner==*(u32 *)(record+0x1C) && record[0x10]!=0) {
            func_L01_006268A8(*(s32 *)slot);
            *(s32 *)slot=-1;
        }
        index++;
        slot+=4;
    } while(index<9);
}
#endif
