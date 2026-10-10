#include "common.h"

/* Complete0058DBA0..0058DC00, including the released-bit trap omitted by
 * the splitter. Arguments are first32-byte block and signed block count.
 * Retail executes at least one iteration even for a nonpositive count.
 * The bitmap is192 bytes for1536 blocks, initialized by0057AF38/0057D110.
 * Out-of-domain indices are not converted into successful releases here.
 * Nonmatching: compiled88 bytes versus96 retail bytes. */
#ifndef DEADLOCKED_NATIVE
extern volatile u8 D_0028BF20[];
void func_L01_0058DBA0(u32 first,s32 count)
{
    do {
        u32 bit=1U<<(first&7);
        volatile u8 *byte=D_0028BF20+(first>>3);
        u32 old=*byte;
        u32 cleared=old&~bit;
        ++first;count=(s32)((u32)count-1U);
        if(cleared==old) {
            __asm__ volatile("teq $0,$0,0" : : : "memory");
            return;
        }
        *byte=(u8)cleared;
    } while(count>0);
}
#endif
