/* cflags: -ffast-math -mno-split-addresses */
/* asflags: --float-immediates */
/* Complete original00594DC8..00594F0C (324 bytes), NONMATCHING.
 * Target is reloaded only for the final snap, after both genuine stores. */
#include "common.h"
typedef unsigned int PositionProgressQuad __attribute__((mode(TI)));
f32 func_L01_00594A88(f32 *,f32,f32,f32,f32);
f32 func_L01_00594DC8(u8 *value,const u8 *target,f32 *rate,
                    f32 force,f32 damping,f32 maximum) {
    f32 scratch[12] __attribute__((aligned(16)));
    f32 distance,current;
    u64 observed;
    union {u32 bits;f32 value;}scalar;
    __asm__ volatile("vmaxw.xyzw $vf1,$vf0,$vf0w\n\tlqc2 $vf2,0(%1)\n\t"
        "lqc2 $vf4,0(%2)\n\tvsub.xyz $vf4,$vf4,$vf2\n\t"
        "vmul.xyzw $vf2,$vf4,$vf4\n\tvadday.x ACC,$vf2,$vf2y\n\t"
        "vmaddz.x $vf1,$vf1,$vf2z\n\tnop\n\tnop\n\tvsqrt Q,$vf1x\n\t"
        "sqc2 $vf1,0(%3)\n\tvwaitq\n\tvaddq.x $vf1,$vf0,Q\n\t"
        "qmfc2.ni %0,$vf1\n\tsqc2 $vf1,0x10(%3)\n\tsqc2 $vf4,0x20(%3)"
        : "=r"(scalar.bits) : "r"(value),"r"(target),"r"(scratch) : "memory");
    distance=scalar.value;
    func_L01_00594A88(rate,distance,force,damping,maximum);
    scalar.value=*rate;
    __asm__ volatile("lqc2 $vf4,0x20(%1)\n\tvmaxw.xyzw $vf2,$vf0,$vf0w\n\t"
        "vmul.xyzw $vf3,$vf4,$vf4\n\tvadday.x ACC,$vf3,$vf3y\n\t"
        "vmaddz.x $vf2,$vf2,$vf3z\n\tqmtc2.ni %2,$vf1\n\t"
        "vmove.xyzw $vf5,$vf4\n\tvrsqrt Q,$vf1x,$vf2x\n\t"
        "qmfc2.ni %0,$vf2\n\tsqc2 $vf2,0(%1)"
        : "=r"(observed) : "r"(scratch),"r"(scalar.bits) : "memory");
    if(observed!=0) {
        __asm__ volatile("vwaitq\n\tvmove.xyzw $vf2,$vf5\n\tvmulq.xyz $vf2,$vf2,Q"
            : : : "memory");
    } else {
        __asm__ volatile("vmove.xyzw $vf2,$vf4\n\tvsub.xyz $vf2,$vf0,$vf0"
            : : : "memory");
    }
    __asm__ volatile("lqc2 $vf1,0(%0)\n\tvadd.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0(%0)"
        : : "r"(value) : "memory");
    current=distance-*rate;
    if(__builtin_fabsf(current)<maximum*0.01f) {
        *(PositionProgressQuad *)value=*(const PositionProgressQuad *)target;
        *rate=0.f;
        current=*rate;
    }
    return current;
}
