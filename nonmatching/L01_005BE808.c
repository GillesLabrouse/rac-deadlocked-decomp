/* Complete 144-byte L01 input-mask remapper, through its final return delay.
 * NONMATCHING; baseline SN compiler flags. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_input_bridge.h"
int dreadzone_actor_input_005BE808(const DreadzoneActorInputContext *ctx,s32 mask,s32 index,s32 *result) {
    const u8 *p=dreadzone_actor_input_read(ctx,0x0021E824,4);
    s32 mode,value;
    if(!p) return DREADZONE_ACTOR_INPUT_NEEDS_MEMORY;
    mode=(s32)dreadzone_actor_input_word(p);
#else
extern volatile s32 D_0021E824;
extern const u8 D_00171DE0[];
s32 func_L01_005BE808(s32 mask,s32 index) {
    s32 mode=D_0021E824,value;
#endif
    if(mode!=1 && mask==1) value=3;
    else {
#ifdef DEADLOCKED_NATIVE
        if(index<0 || index>1) return DREADZONE_ACTOR_INPUT_UNSUPPORTED_INDEX;
        p=dreadzone_actor_input_read(ctx,0x00171DE0+(u32)index,1);
        if(!p) return DREADZONE_ACTOR_INPUT_NEEDS_MEMORY;
        if(*p) {
#else
        if(D_00171DE0[index]) {
#endif
            if(mask==4 || mask==8) value=1;
            else if(mask==16) value=18;
            else if(mask==64) value=68;
            else value=mask;
        } else value=mask;
    }
#ifdef DEADLOCKED_NATIVE
    *result=value;
    return DREADZONE_ACTOR_INPUT_COMPLETED;
#else
    return value;
#endif
}
