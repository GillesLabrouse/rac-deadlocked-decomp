#include "common.h"

/* Complete 0058ACB8..0058AD10,88 bytes. Capture the next link BEFORE the
 * genuine release clears the current node. This differs from688FE0. */
#ifndef DEADLOCKED_NATIVE
void func_L01_0058A8D8(u8 *);
void func_L01_0058ACB8(u8 *actor) {
    u8 *node=*(u8 *volatile *)(actor+0x50);
    while(node!=NULL) {
        u8 *next=*(u8 *volatile *)(node+0x10);
        func_L01_0058A8D8(node);
        node=next;
    }
    *(volatile u32 *)(actor+0x50)=0;
}
#endif
