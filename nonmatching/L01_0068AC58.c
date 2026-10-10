/* cflags: -ffast-math */
/* Complete288-byte original, including mode1/2 transform-normalize-transform.
 * Reads the source before output, then loads the second matrix after the
 * intermediate output store: output-to-matrix aliases retain this order. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_velocity.h"
#endif
#include "../include/dreadzone_actor_velocity_internal.h"
#ifdef DEADLOCKED_NATIVE
int dreadzone_velocity_0068AC58(const u8 *record,u8 *output,const u8 *input,f32 length) {
    VelocityVector result;
    if(record[0x2659]) return DREADZONE_VELOCITY_UNSUPPORTED_MODE;
    memcpy(&result,input,16);
    if(!dreadzone_velocity_scale(&result,length,2,0))
        return DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
    memcpy(output,&result,16);
    return DREADZONE_VELOCITY_APPROXIMATE;
}
#else
#define V_SCALE_XY \
    "vmove.xyzw $vf4,$vf3\n\tvmul.xyzw $vf1,$vf4,$vf4\n\t" \
    "vaddy.x $vf1,$vf1,$vf1y\n\tnop\n\tnop\n\t" \
    "vrsqrt Q,$vf2x,$vf1x\n\tqmfc2.ni %0,$vf1\n\t" \
    ".set noreorder\n\tbeqz %0,1f\n\tsqc2 $vf1,0(%3)\n\t" \
    "vwaitq\n\tvmove.xyzw $vf4,$vf3\n\tvmulq.xy $vf4,$vf4,Q\n\t" \
    "b 2f\n\tsqc2 $vf4,0(%1)\n\t1:\n\tvsub.xy $vf4,$vf0,$vf0\n\t" \
    "sqc2 $vf4,0(%1)\n\t2:\n\t.set reorder\n\t"
void func_L01_0068AC58(const u8 *record,VelocityVector *output,const VelocityVector *input,f32 length) {
    VelocityBits bits;VelocityVector scratch;u32 low;
    bits.value=length;
    if(!record[0x2659]) {
        __asm__ volatile("lqc2 $vf3,0(%2)\n\tqmtc2.ni %4,$vf2\n\t" V_SCALE_XY
            : "=&r"(low) : "r"(output),"r"(input),"r"(&scratch),"r"(bits.word) : "memory");
    } else if(record[0x2659]<3) {
        __asm__ volatile("lqc2 $vf3,0(%2)\n\tlqc2 $vf2,0x60(%5)\n\t"
            "lqc2 $vf1,0x70(%5)\n\tvmulax.xyzw ACC,$vf2,$vf3x\n\t"
            "lqc2 $vf4,0x80(%5)\n\tvmadday.xyzw ACC,$vf1,$vf3y\n\t"
            "lqc2 $vf2,0x90(%5)\n\tvmaddaz.xyzw ACC,$vf4,$vf3z\n\t"
            "vmaddw.xyzw $vf3,$vf2,$vf3w\n\tqmtc2.ni %4,$vf2\n\t"
            "sqc2 $vf3,0(%1)\n\t" V_SCALE_XY
            "lqc2 $vf1,0x20(%5)\n\tlqc2 $vf2,0x30(%5)\n\t"
            "vmulax.xyzw ACC,$vf1,$vf4x\n\tlqc2 $vf3,0x40(%5)\n\t"
            "vmadday.xyzw ACC,$vf2,$vf4y\n\tlqc2 $vf1,0x50(%5)\n\t"
            "vmaddaz.xyzw ACC,$vf3,$vf4z\n\tvmaddw.xyzw $vf1,$vf1,$vf4w\n\t"
            "sqc2 $vf1,0(%1)\n\tsqc2 $vf1,0(%3)"
            : "=&r"(low) : "r"(output),"r"(input),"r"(&scratch),"r"(bits.word),"r"(record) : "memory");
    }
}
#endif
