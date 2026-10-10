/* cflags: -ffast-math */
/* Complete540-byte original plus padding. Mode0 returns planar length.
 * Modes1/2 load actor2EF0 angles at F0, construct the genuine transposed
 * Euler basis from three C60 calls, transform the source, then return XY
 * length. Native supports mode0 only, with no fabricated body pointer. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_velocity.h"
#endif
#include "../include/dreadzone_actor_velocity_internal.h"
#ifdef DEADLOCKED_NATIVE
f32 func_L01_0068A920(const u8 *record,const VelocityVector *input) {
    VelocityVector value;
    if(record[0x2659]) return NAN;
    memcpy(&value,input,16);
    return dreadzone_velocity_length(&value,2);
}
#else
static __inline__ f32 velocity_planar_length(const VelocityVector *input) {
    VelocityBits result;VelocityVector scratch;
    __asm__ volatile("lqc2 $vf1,0(%1)\n\tvmul.xyzw $vf1,$vf1,$vf1\n\t"
        "vaddy.x $vf1,$vf1,$vf1y\n\tnop\n\tnop\n\tvsqrt Q,$vf1x\n\t"
        "sqc2 $vf1,0(%2)\n\tvwaitq\n\tvaddq.x $vf1,$vf0,Q\n\t"
        "qmfc2.ni %0,$vf1" : "=&r"(result.word) : "r"(input),"r"(&scratch) : "memory");
    return result.value;
}
f32 func_L01_0068A920(const u8 *record,const VelocityVector *input) {
    if(!record[0x2659]) return velocity_planar_length(input);
    if(record[0x2659]<3) {
        const u8 *actor=*(u8 **)(record+0x2EF0);
        VelocityVector first,second,third,source,transformed;
        f32 s0,c0,s1,c1,s2,c2,f2,f3,f5,f7,f9,f10,f11,f1,f4,snapshot2;
        source=*input;
        V_ANGLE_PAIR(V_FLOAT(actor,0xF0),s0,c0);
        V_ANGLE_PAIR(V_FLOAT(actor,0xF4),s1,c1);
        f3=s0*s1;f5=c0*s1;s1=-s1;
        f2=c1*s0;f7=c0*c1;snapshot2=f2;
        V_ANGLE_PAIR(V_FLOAT(actor,0xF8),s2,c2);
        f10=f3*s2;f1=-c2;f2=c0*s2;
        f3=c2*f3;f4=c0*c2;f11=f5*s2;
        f1=f1*s0;f5=f5*c2;f9=s0*s2;
        f3=f3-f2;f4=f4+f10;f5=f5+f9;f1=f1+f11;
        first.v[0]=c1*c2;first.v[1]=c1*s2;first.v[2]=s1;first.v[3]=0.f;
        second.v[0]=f3;second.v[1]=f4;second.v[2]=snapshot2;second.v[3]=0.f;
        third.v[0]=f5;third.v[1]=f1;third.v[2]=f7;third.v[3]=0.f;
        /* Actual matrix used by the VU is the transpose of these columns. */
        {VelocityVector row0,row1,row2;
         row0.v[0]=first.v[0];row0.v[1]=second.v[0];row0.v[2]=third.v[0];row0.v[3]=0.f;
         row1.v[0]=first.v[1];row1.v[1]=second.v[1];row1.v[2]=third.v[1];row1.v[3]=0.f;
         row2.v[0]=first.v[2];row2.v[1]=second.v[2];row2.v[2]=third.v[2];row2.v[3]=0.f;
         __asm__ volatile("lqc2 $vf2,0(%4)\n\tlqc2 $vf1,0(%0)\n\t"
             "vmulax.xyzw ACC,$vf1,$vf2x\n\tlqc2 $vf1,0(%1)\n\t"
             "vmadday.xyzw ACC,$vf1,$vf2y\n\tlqc2 $vf1,0(%2)\n\t"
             "vmaddaz.xyzw ACC,$vf1,$vf2z\n\tvmaddw.xyzw $vf2,$vf0,$vf2w\n\t"
             "sqc2 $vf2,0(%3)" : : "r"(&row0),"r"(&row1),"r"(&row2),
             "r"(&transformed),"r"(&source) : "memory");}
        return velocity_planar_length(&transformed);
    }
    return 0.f;
}
#endif
