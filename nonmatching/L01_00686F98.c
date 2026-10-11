/* Complete original 00686F98..00687000, 104 bytes, NONMATCHING.
 * Accumulated time uses the original wrapping ADDU, followed by signed SLT.
 * The current command is read only after the time-limit test; each duration
 * is consumed after that command's comparison, including the final one. */
#include "common.h"
#ifndef DEADLOCKED_NATIVE
s32 func_L01_00686F98(u8 *record,s32 command,s32 maximum) {
    s32 count=*(volatile s32 *)(record+0x264C),index;
    u32 clock=*(volatile u32 *)(record+0x370);
    for(index=0;index<count;++index) {
        if(maximum<(s32)clock)return 0;
        if(*(volatile s32 *)(record+0x25EC+(u32)index*4)==command)return 1;
        clock+=*(volatile u32 *)(record+0x262C+(u32)index*4);
    }
    return 0;
}
#endif
