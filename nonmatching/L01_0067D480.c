/* cflags: -ffast-math */
/* Complete100-byte original normalized velocity producer plus padding.
 * Input quad is loaded before record1784 and all output stores; W survives.
 * The original BEQ tests the low64 of squared-sum VF1, whose Y lane is1.
 * Its nominal zero alternative is therefore not a host vector-zero branch. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_velocity.h"
#endif
#include "../include/dreadzone_actor_velocity_internal.h"
#ifdef DEADLOCKED_NATIVE
int dreadzone_velocity_0067D480(u8 *record,const u8 *input) {
    VelocityVector result;f32 speed;
    memcpy(&result,input,16);speed=V_FLOAT(record,0x1784);
    if(!dreadzone_velocity_scale(&result,speed,3,1))
        return DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
    memcpy(record+0x130,&result,16);
    return DREADZONE_VELOCITY_APPROXIMATE;
}
#else
void func_L01_0067D480(u8 *record,const VelocityVector *input) {
    VelocityVector scratch;u32 sum;
    __asm__ volatile("lqc2 $vf5,0(%3)\n\tvmaxw.xyzw $vf1,$vf0,$vf0w\n\t"
        "lwc1 $f0,0x1784(%1)\n\tmfc1 %0,$f0\n\tvmove.xyzw $vf4,$vf5\n\t"
        "qmtc2.ni %0,$vf3\n\tvmul.xyzw $vf2,$vf4,$vf4\n\t"
        "vadday.x ACC,$vf2,$vf2y\n\tvmaddz.x $vf1,$vf1,$vf2z\n\t"
        "nop\n\tnop\n\tvrsqrt Q,$vf3x,$vf1x\n\tqmfc2.ni %0,$vf1\n\t"
        ".set noreorder\n\tbeqz %0,1f\n\tsqc2 $vf1,0(%2)\n\t"
        "vwaitq\n\tvmove.xyzw $vf4,$vf5\n\tvmulq.xyz $vf4,$vf4,Q\n\t"
        "b 2f\n\tsqc2 $vf4,0x130(%1)\n\t1:\n\tvsub.xyz $vf4,$vf0,$vf0\n\t"
        "sqc2 $vf4,0x130(%1)\n\t2:\n\t.set reorder"
        : "=&r"(sum) : "r"(record),"r"(&scratch),"r"(input) : "$f0","memory");
}
#endif
