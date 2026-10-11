#include "common.h"
#ifndef DEADLOCKED_NATIVE
/* Complete history publication. The three eight-word arrays keep guest layout. */
void func_L01_00686EF8(u8 *player) {
    s32 count, index, next;
    u8 *cursor;
    u32 state, base, timer, previous_state, previous_base;
    count=*(s32 *)(player+0x264C);
    index=(s32)((u32)(count>=8 ? 7 : count)-1U);
    if(index>=0) {
        cursor=player+0x2630+(u32)index*4U;
        do {
            u32 old_state=*(u32 *)(cursor-0x44);
            u32 old_base=*(u32 *)(cursor-0x24);
            u32 old_timer=*(u32 *)(cursor-4);
            index=(s32)((u32)index-1U);
            *(u32 *)(cursor-0x40)=old_state;
            *(u32 *)(cursor-0x20)=old_base;
            *(u32 *)cursor=old_timer;
            cursor-=4;
        } while(index>=0);
    }
    state=*(u32 *)(player+0x25CC);
    next=(s32)(*(u32 *)(player+0x264C)+1U);
    base=*(u32 *)(player+0x25D4);
    timer=*(u32 *)(player+0x370);
    previous_state=*(u32 *)(player+0x25D8);
    previous_base=*(u32 *)(player+0x25DC);
    *(u32 *)(player+0x25DC)=base;
    *(s32 *)(player+0x264C)=next<9 ? next : 8;
    *(u32 *)(player+0x25E4)=previous_state;
    *(u32 *)(player+0x25E8)=previous_base;
    *(u32 *)(player+0x25E0)=timer;
    *(u32 *)(player+0x25D8)=state;
    *(u32 *)(player+0x25EC)=state;
    *(u32 *)(player+0x260C)=base;
    *(u32 *)(player+0x262C)=timer;
}
#endif
