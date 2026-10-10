/* Complete 136-byte L01 button preparation, including four-byte padding.
 * NONMATCHING; baseline SN compiler flags. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_input_bridge.h"
int func_L01_0067BD58(const DreadzoneActorInputContext *ctx,u8 *record) {
    const u8 *p;
    s32 predicate,mask,status;
    u32 input;
    status=dreadzone_actor_input_predicate(ctx,record,&predicate);
    if(status) return status;
#else
s32 func_L01_0068DE60(u8 *);
s32 func_L01_0055D180(s16 *);
s32 func_L01_005BE808(s32,s32);
extern volatile s32 D_0021E824;
void func_L01_0067BD58(u8 *record) {
    s32 predicate=func_L01_0068DE60(record),mask;
#endif
    if(predicate) {
        (void)func_L01_0055D180((s16 *)(record+0x5F4));
#ifdef DEADLOCKED_NATIVE
        return DREADZONE_ACTOR_INPUT_COMPLETED;
#else
        return;
#endif
    }
#ifdef DEADLOCKED_NATIVE
    status=dreadzone_actor_input_005BE808(ctx,1,(s32)dreadzone_actor_input_word(record+0x2F10),&mask);
    if(status) return status;
    input=dreadzone_actor_input_word(record+0x2F00);
    if(input&3 || input>0xFFFFFFFFU-0x1C0) return DREADZONE_ACTOR_INPUT_NEEDS_MEMORY;
    p=dreadzone_actor_input_read(ctx,input+0x1C0,4);
    if(!p) return DREADZONE_ACTOR_INPUT_NEEDS_MEMORY;
    record[0x5F4]=(u8)((dreadzone_actor_input_word(p)&(u32)mask)!=0);record[0x5F5]=0;
    p=dreadzone_actor_input_read(ctx,0x0021E824,4);
    if(!p) return DREADZONE_ACTOR_INPUT_NEEDS_MEMORY;
    if(dreadzone_actor_input_word(p)==1 && dreadzone_actor_input_word(record+0x5F0)) {
        record[0x5F4]=1;record[0x5F5]=0;
    }
    return DREADZONE_ACTOR_INPUT_COMPLETED;
#else
    mask=func_L01_005BE808(1,*(s32 *)(record+0x2F10));
    *(s16 *)(record+0x5F4)=(s16)((*(u32 *)(*(u32 *)(record+0x2F00)+0x1C0)&(u32)mask)!=0);
    if(D_0021E824==1 && *(u32 *)(record+0x5F0)) *(s16 *)(record+0x5F4)=1;
#endif
}
