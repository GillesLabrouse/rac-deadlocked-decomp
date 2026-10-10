#include "common.h"

/* Complete0058DC00..0058DC78. Scans the real bitmap for an aligned run
 * within one32-bit bitmap word, sets its bits and returns the first block.
 * The actor-cell caller supplies powers of two1..16. The original scan has
 * no exhaustion bound or failure result; it reads subsequent memory when
 * its initialized192-byte bitmap is full. No bounded success is fabricated.
 * Scalar shifts deliberately retain their32/64-bit R5900 masks.
 * Nonmatching: compiled172 bytes versus120 retail bytes. */
#ifndef DEADLOCKED_NATIVE
extern volatile u32 D_0028BF20[];
s32 func_L01_0058DC00(s32 blocks)
{
    volatile u32 *word=D_0028BF20;
    u64 initial=(u64)(s64)(s32)((1U<<((u32)blocks&31))-1U);
    s32 first=-32;
    for(;;) {
        u64 used=*word++;
        u64 mask;
        s32 offset;
        first=(s32)((u32)first+32U);
        if(used==0xFFFFFFFFU) continue;
        mask=initial;offset=0;
        for(;;) {
            u64 occupied=used&mask;
            mask=mask<<((u32)blocks&63);
            offset=(s32)((u32)offset+(u32)blocks);
            if(occupied==0) break;
        }
        offset=(s32)((u32)offset-(u32)blocks);
        mask=mask>>((u32)blocks&63);
        if(offset==32) continue;
        word[-1]=(u32)(used|mask);
        return (s32)((u32)first+(u32)offset);
    }
}
#endif
