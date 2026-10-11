#include "common.h"
#ifndef DEADLOCKED_NATIVE
extern u8 D_0028EB40[],D_0028EF40[];
extern u32 D_0028EF80[];
/* Complete132-byte first-free search; original trailing padding excluded. */
u8 *func_L01_0058A850(u8 *actor) {
    s32 slot;
    if(actor[0x31]==0 || (actor[0x20]&0x80)!=0) return NULL;
    for(slot=0;slot<32;++slot) {
        if(D_0028EF40[slot]==0) {
            D_0028EF40[slot]=1;
            D_0028EF80[slot]=(u32)actor;
            return D_0028EB40+slot*32;
        }
    }
    return NULL;
}
#endif
