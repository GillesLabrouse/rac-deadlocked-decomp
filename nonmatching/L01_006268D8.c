#include "common.h"

/* Complete actual 64-byte release. State7 has three real clears; inactive
 * state0 and state6 do not mutate, all other states become4. */
#ifndef DEADLOCKED_NATIVE
void func_L01_006268D8(u8 *record) {
    u8 state=record[0x10];
    if(state==7) {
        record[0x10]=0;
        *(u32 *)(record+0x1C)=0;
        *(u32 *)(record+0x40)=0;
    } else if(state!=0 && state!=6) record[0x10]=4;
}
#endif
