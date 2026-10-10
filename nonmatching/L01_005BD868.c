/* Complete 144-byte recent-press history query, NONMATCHING baseline SN.
 * A hit reloads its history word after the optional result store. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_direction.h"
#endif
#include "../include/dreadzone_actor_direction_internal.h"
#ifdef DEADLOCKED_NATIVE
int dreadzone_direction_history(const DreadzoneActorInputContext *ctx,s32 mask,
                                s32 count,u32 out,u32 input,s32 *result) {
    const u8 *p;u8 *write;s32 available,index,i,value;
    if(!input) input=0x001EE600;
    if(input&3 || input>0xFFFFFFFFU-0x257) return DREADZONE_DIRECTION_NEEDS_MEMORY;
    p=dreadzone_actor_input_read(ctx,input+0x190,4);
    if(!p) return DREADZONE_DIRECTION_NEEDS_MEMORY;
    available=(s32)dreadzone_actor_input_word(p);
    if(available<0 || available>30) return DREADZONE_DIRECTION_UNSUPPORTED_MODE;
    if(count>available) count=available;
    for(i=1;i<=count;++i) {
        p=dreadzone_actor_input_read(ctx,input+0x18E,2);
        if(!p) return DREADZONE_DIRECTION_NEEDS_MEMORY;
        index=(s16)((u16)p[0]|(u16)p[1]<<8);
        if(index<0 || index>=30) return DREADZONE_DIRECTION_UNSUPPORTED_MODE;
        index=(index-i+30)%30;
        p=dreadzone_actor_input_read(ctx,input+0x1E0+(u32)index*4,4);
        if(!p) return DREADZONE_DIRECTION_NEEDS_MEMORY;
        value=(s32)dreadzone_actor_input_word(p);
        if(value&mask) {
            if(out) {
                if(out&3) return DREADZONE_DIRECTION_NEEDS_MEMORY;
                write=dreadzone_actor_input_write(ctx,out,4);
                if(!write) return DREADZONE_DIRECTION_NEEDS_MEMORY;
                dreadzone_actor_input_store32(write,(u32)(i-1));
            }
            *result=(s32)dreadzone_actor_input_word(p)&mask;return 0;
        }
    }
    *result=0;return 0;
}
#else
extern u8 D_001EE600[];
s32 func_L01_005BD868(s32 mask,s32 count,s32 *out,u8 *input) {
    s32 i,index;
    if(!input) input=D_001EE600;
    if(count>*(s32 *)(input+0x190)) count=*(s32 *)(input+0x190);
    for(i=1;i<=count;++i) {
        index=(*(s16 *)(input+0x18E)-i+30)%30;
        if(*(s32 *)(input+0x1E0+index*4)&mask) {
            if(out) *out=i-1;
            return *(s32 *)(input+0x1E0+index*4)&mask;
        }
    }
    return 0;
}
#endif
