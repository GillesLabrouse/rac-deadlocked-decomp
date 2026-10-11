#include "common.h"
#ifndef DEADLOCKED_NATIVE
void func_L01_00586E80(u8 *);
/* Full child cleanup; the count is genuinely reloaded after destruction. */
void func_L01_0059AFB8(u8 *container) {
    s32 index;
    u8 **slot;
    if(*(s32 *)(container+0x14C)!=0) {
        index=0;
        if(*(s32 *)(container+0x148)>0) {
            slot=(u8 **)(container+0x120);
            do {
                u8 *child=*slot;
                if(child!=NULL) {
                    func_L01_00586E80(child);
                    *slot=NULL;
                }
                index++;
                slot++;
            } while(index<*(s32 *)(container+0x148));
        }
        *(u32 *)(container+0x14C)=0;
    }
}
#endif
