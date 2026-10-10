/* cflags: -ffast-math */
/* Complete 452-byte original plus four-byte padding. Mode0 preserves the
 * destination X/Y/W and sets Z from the source Z minus the original offset.
 * Modes1/2 retain genuine normalization and both projection dependencies.
 * NONMATCHING: SN factors normalization into a local helper and selects a
 * different register/control-flow layout. Kept outside the accepted build. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_velocity.h"
#endif
#include "../include/dreadzone_actor_velocity_internal.h"
#ifdef DEADLOCKED_NATIVE
#include <float.h>
int dreadzone_velocity_0068A178(const u8 *record,u8 *output,const u8 *input,f32 offset) {
    VelocityVector source,destination;volatile f32 z;
    if(record[0x2659]) return DREADZONE_VELOCITY_UNSUPPORTED_MODE;
    memcpy(&source,input,16);memcpy(&destination,output,16);
    destination.v[2]=source.v[2];memcpy(output,&destination,16);
    z=destination.v[2]-offset;
    if(!isfinite(z) || (z!=0.f && fabsf(z)<FLT_MIN))
        return DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
    destination.v[2]=z;memcpy(output,&destination,16);
    return DREADZONE_VELOCITY_APPROXIMATE;
}
#else
extern void func_L01_0068ABA8(u8 *,VelocityVector *,const VelocityVector *,f32);
extern void func_L01_0068AC58(const u8 *,VelocityVector *,const VelocityVector *,f32);
static void height_normal(const VelocityVector *direction,f32 amount,VelocityVector *scaled) {
    VelocityBits bits;VelocityLanes length;bits.value=amount;
    __asm__ volatile("lqc2 $vf4,0(%1)\n\tqmtc2.ni %2,$vf3\n\t"
        "vmaxw.xyzw $vf1,$vf0,$vf0w\n\tvmul.xyzw $vf2,$vf4,$vf4\n\t"
        "vadday.x ACC,$vf2,$vf2y\n\tvmaddz.x $vf1,$vf1,$vf2z\n\t"
        "nop\n\tnop\n\tvrsqrt Q,$vf3x,$vf1x\n\tqmfc2.ni %0,$vf1"
        : "=r"(length.quad) : "r"(direction),"r"(bits.word) : "memory");
    if((u64)length.quad) {
        __asm__ volatile("vwaitq\n\tvmulq.xyz $vf4,$vf4,Q\n\tsqc2 $vf4,0(%0)"
            : : "r"(scaled) : "memory");
    } else {
        __asm__ volatile("vsub.xyz $vf4,$vf0,$vf0\n\tsqc2 $vf4,0(%0)"
            : : "r"(scaled) : "memory");
    }
}
void func_L01_0068A178(u8 *record,VelocityVector *output,const VelocityVector *input,f32 offset) {
    VelocityBits bits;VelocityVector scaled,planar;u32 mode=record[0x2659];bits.value=offset;
    if(mode==0) {
        __asm__ volatile("lqc2 $vf1,0(%1)\n\tlqc2 $vf2,0(%0)\n\t"
            "vmove.z $vf2,$vf1\n\tqmtc2.ni %2,$vf1\n\tsqc2 $vf2,0(%0)\n\t"
            "vsubx.z $vf2,$vf2,$vf1x\n\tsqc2 $vf2,0(%0)"
            : : "r"(output),"r"(input),"r"(bits.word) : "memory");
    } else if(mode==1 || mode==2) {
        height_normal((const VelocityVector *)(record+(mode==1?0x250:0x270)),
                      mode==1?-offset:offset,&scaled);
        func_L01_0068AC58(record,&planar,input,0.f);
        func_L01_0068ABA8(record,output,output,0.f);
        __asm__ volatile("lqc2 $vf1,0(%0)\n\tlqc2 $vf2,0(%1)\n\t"
            "vadd.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0(%0)\n\t"
            "lqc2 $vf2,0(%2)\n\tvadd.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0(%0)"
            : : "r"(output),"r"(&planar),"r"(&scaled) : "memory");
    }
}
#endif
