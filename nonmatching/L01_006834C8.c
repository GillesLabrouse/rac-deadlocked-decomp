/* cflags: -ffast-math */
/* Complete 184-byte original. Consumes the genuine vector at actor1A0,
 * reduces its planar length, then adds the result to actor130. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_velocity.h"
#include <float.h>
#endif
#include "../include/dreadzone_actor_velocity_internal.h"
#ifdef DEADLOCKED_NATIVE
int dreadzone_velocity_006834C8(u8 *record,f32 reduction) {
    const u32 threshold_bits=0x3A83126FU;
    VelocityVector value,current;f32 threshold,length,requested;
    volatile f32 difference,sum;u32 i;int status;
    memcpy(&threshold,&threshold_bits,4);
    memcpy(&value,record+0x1A0,16);
    length=func_L01_0068A920(record,&value);
    if(!isfinite(length)) return DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
    if(length>threshold) {
        status=dreadzone_velocity_0068ABA8(record,record+0x1A0,record+0x1A0,0.f);
        if(status<0) return status;
        difference=length-reduction;
        requested=dreadzone_velocity_max(difference,0.f);
        if(!isfinite(requested)) return DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
        status=dreadzone_velocity_0068AC58(record,record+0x1A0,record+0x1A0,requested);
        if(status<0) return status;
        memcpy(&value,record+0x1A0,16);memcpy(&current,record+0x130,16);
        for(i=0;i<3;++i) {
            sum=current.v[i]+value.v[i];
            if(!isfinite(sum) || (sum!=0.f && fabsf(sum)<FLT_MIN))
                return DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
            current.v[i]=sum;
        }
        memcpy(record+0x130,&current,16);
    } else memset(record+0x1A0,0,16);
    return DREADZONE_VELOCITY_APPROXIMATE;
}
#else
extern f32 func_L01_0068A920(const u8 *,const VelocityVector *);
extern void func_L01_0068ABA8(const u8 *,VelocityVector *,const VelocityVector *,f32);
extern void func_L01_0068AC58(const u8 *,VelocityVector *,const VelocityVector *,f32);
void func_L01_006834C8(u8 *record,f32 reduction) {
    VelocityBits threshold;f32 length;VelocityVector *value=(VelocityVector *)(record+0x1A0);
    threshold.word=0x3A83126FU;
    length=func_L01_0068A920(record,value);
    if(length>threshold.value) {
        func_L01_0068ABA8(record,value,value,0.f);
        func_L01_0068AC58(record,value,value,V_MAX(length-reduction,0.f));
        __asm__ volatile("lqc2 $vf1,0x130(%0)\n\tlqc2 $vf2,0x1A0(%0)\n\t"
            "vadd.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0x130(%0)"
            : : "r"(record) : "memory");
    } else {
        __asm__ volatile("sq $0,0x1A0(%0)" : : "r"(record) : "memory");
    }
}
#endif
