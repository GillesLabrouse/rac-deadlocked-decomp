#include "common.h"
#ifndef DEADLOCKED_NATIVE
extern u8 D_003B5050[];
/* Null input selects row zero. Nonnull index FF alone suppresses clearing. */
void func_L01_00533770(u8 *input) {
    s32 remaining=7;
    u32 row=0;
    u8 *cursor;
    if(input!=NULL) {
        if(input[0x558]==0xFF) return;
        row=(u32)input[0x54C]*4U+input[0x558];
    }
    cursor=D_003B5050+(row<<7)+0x70;
    do {
        *cursor=0;
        remaining--;
        cursor-=0x10;
    } while(remaining>=0);
}
#endif
