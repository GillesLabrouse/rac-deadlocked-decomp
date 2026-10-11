#include "common.h"

/* Complete L01 0068AFF8..0068B0D0, 0xD8 including padding. NONMATCHING.
 * Actual ABI: a0 record, a1 raw normal quad; result f0. LBU record2659
 * selects mode0 raw coordinates, modes1/2 full affine4x4 including W,
 * or mode>=3 positive zero without reading either matrix or normal.
 * Preserves genuine masked VU operations/Q and the real polynomial callee;
 * does not normalize the normal or replace 0055D708 with CRT atan2.
 */
f32 func_L01_0055D708(f32,f32);

f32 func_L01_0068AFF8(const u8 *record,const u8 *normal)
{
    f32 transformed[4] __attribute__((aligned(16)));
    f32 radial[4] __attribute__((aligned(16)));
    u32 mode=record[0x2659];
    if(mode==0) {
        __asm__ volatile("lqc2 $vf1,0(%0)\n\t"
            "vmul.xyzw $vf2,$vf1,$vf1\n\tvaddy.x $vf2,$vf2,$vf2y\n\t"
            "sqc2 $vf1,0(%1)\n\tnop\n\tnop\n\tvsqrt Q,$vf2x\n\t"
            "sqc2 $vf2,0(%2)\n\tvwaitq\n\t"
            "vaddq.x $vf1,$vf0,Q\n\tsqc2 $vf1,0(%2)"
            : : "r"(normal),"r"(transformed),"r"(radial) : "memory");
    } else if(mode<3) {
        __asm__ volatile("lqc2 $vf3,0(%0)\n\tlqc2 $vf1,0x60(%1)\n\t"
            "lqc2 $vf2,0x70(%1)\n\tvmulax.xyzw ACC,$vf1,$vf3x\n\t"
            "lqc2 $vf4,0x80(%1)\n\tvmadday.xyzw ACC,$vf2,$vf3y\n\t"
            "lqc2 $vf1,0x90(%1)\n\tvmaddaz.xyzw ACC,$vf4,$vf3z\n\t"
            "vmaddw.xyzw $vf1,$vf1,$vf3w\n\t"
            "vmul.xyzw $vf2,$vf1,$vf1\n\tsqc2 $vf1,0(%2)\n\t"
            "vaddy.x $vf2,$vf2,$vf2y\n\tnop\n\tnop\n\t"
            "vsqrt Q,$vf2x\n\tsqc2 $vf2,0(%3)\n\tvwaitq\n\t"
            "vaddq.x $vf1,$vf0,Q\n\tsqc2 $vf1,0(%3)"
            : : "r"(normal),"r"(record),"r"(transformed),"r"(radial) : "memory");
    } else return 0.0f;
    return func_L01_0055D708(transformed[2],radial[0]);
}
