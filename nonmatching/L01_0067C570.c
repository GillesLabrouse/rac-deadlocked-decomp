/* cflags: -ffast-math */
/* Complete 80-byte L01 VU xyz magnitude capped at one; NONMATCHING SN draft.
 * Native scalar square/add/sqrt uses the recovered order, but is approximate.
 * VU rounding, ACC precision, scheduling and status flags are not reproduced. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_input_bridge.h"
#include <math.h>
#include <string.h>
#endif
f32 func_L01_0067C570(const u8 *unused,const f32 *vector) {
    union {u32 bits;f32 value;} magnitude;
    (void)unused;
#ifdef DEADLOCKED_NATIVE
    f32 values[3];
    volatile f32 x2,y2,z2,xy,sum;
    u32 i,bits;
    memcpy(values,vector,sizeof(values));
    for(i=0;i<3;++i) {
        memcpy(&bits,values+i,4);bits&=0x7FFFFFFFU;
        if(bits && (bits<0x30800000U || bits>0x3F800000U)) return NAN;
    }
    x2=values[0]*values[0];y2=values[1]*values[1];z2=values[2]*values[2];
    xy=x2+y2;sum=xy+z2;magnitude.value=sqrtf(sum);
#else
    __asm__ volatile("lqc2 $vf1,0(%1)\n\tvmaxw.xyzw $vf2,$vf0,$vf0w\n\t"
        "vmul.xyzw $vf1,$vf1,$vf1\n\tvadday.x ACC,$vf1,$vf1y\n\t"
        "vmaddz.x $vf2,$vf2,$vf1z\n\tnop\n\tnop\n\t"
        "vsqrt Q,$vf2x\n\tvwaitq\n\tvaddq.x $vf1,$vf0,Q\n\t"
        "qmfc2.ni %0,$vf1" : "=r"(magnitude.bits) : "r"(vector) : "memory");
#endif
    return magnitude.value<1.f?magnitude.value:1.f;
}
