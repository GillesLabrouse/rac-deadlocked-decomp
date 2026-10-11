#include "common.h"
#ifndef DEADLOCKED_NATIVE
void func_L01_00671060(u8 *player) {
    s32 remaining=3;
    u8 *cursor=player;
    *(u32 *)(player+0x1864)=0;
    *(u32 *)(player+0x1868)=0;
    do {
        *(u32 *)(cursor+0x1884)=0;
        remaining--;
        *(u32 *)(cursor+0x1888)=0;
        cursor+=0x10;
    } while(remaining>=0);
}
#endif
