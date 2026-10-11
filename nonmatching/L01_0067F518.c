/* cflags: -mno-split-addresses */
/* Complete original 0067F518..0067F5E4 (204 bytes), NONMATCHING.
 * EF00's original -1 is a real result, not an unsupported host dependency. */
#include "common.h"
#ifndef DEADLOCKED_NATIVE
typedef unsigned int CapsuleHistoryQuad __attribute__((mode(TI)));
s32 func_L01_0067EF00(u8 *,s32);
void func_L01_0067F518(u8 *record) {
    s32 attempt,limit=record[0x266D]!=0?16:1;
    for(attempt=0;attempt<limit;++attempt) {
        s32 count,index;
        if(func_L01_0067EF00(record,attempt)==1)return;
        count=*(volatile s32 *)(record+0x2C7C);
        index=*(volatile s32 *)(record+0x2C78);
        if(count>=2) {
            *(volatile s32 *)(record+0x2C7C)=count-1;
            index=(s32)((u32)index+31U)%32;
            *(volatile s32 *)(record+0x2C78)=index;
        }
        index=*(volatile s32 *)(record+0x2C78);
        *(volatile CapsuleHistoryQuad *)(record+0xA0)=
            *(volatile CapsuleHistoryQuad *)(record+0x26F0+((u32)index<<4));
    }
}
#endif
