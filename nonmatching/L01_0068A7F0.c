/* cflags: -ffast-math -mno-split-addresses */
/* Complete original0068A7F0..0068A91C (300 bytes). Three real modes;
 * masked vector stores preserve the original raw W lane. NONMATCHING. */
#include "common.h"
void func_L01_0068A7F0(u8 *record,u8 *output,const u8 *input) {
    u8 mode=record[0x2659];
    if(mode==0) {
        __asm__ volatile("lqc2 $vf1,0(%1)\n\tqmtc2.ni $0,$vf2\n\t"
            "sqc2 $vf1,0(%0)\n\tvaddx.z $vf1,$vf0,$vf2x\n\tsqc2 $vf1,0(%0)"
            : : "r"(output),"r"(input) : "memory");
    } else if(mode==1) {
        u64 observed;
        f32 scratch[12] __attribute__((aligned(16)));
        __asm__ volatile("lqc2 $vf2,0x250(%1)\n\tvmaxw.xyzw $vf1,$vf0,$vf0w\n\t"
            "vmul.xyzw $vf3,$vf2,$vf2\n\tvmove.xyzw $vf4,$vf1\n\t"
            "vadday.x ACC,$vf3,$vf3y\n\tvmaddz.x $vf1,$vf1,$vf3z\n\t"
            "sqc2 $vf2,0x20(%2)\n\tqmfc2.ni %0,$vf1\n\tsqc2 $vf1,0(%2)\n\t"
            "nop\n\tnop\n\tvrsqrt Q,$vf0w,$vf1x"
            : "=r"(observed) : "r"(record),"r"(scratch) : "memory");
        if(observed!=0) {
            __asm__ volatile("vwaitq\n\tlqc2 $vf1,0x20(%0)\n\t"
                "vmulq.xyz $vf1,$vf1,Q\n\tsqc2 $vf1,0x20(%0)"
                : : "r"(scratch) : "memory");
        } else {
            __asm__ volatile("vsub.xyz $vf1,$vf0,$vf0\n\tsqc2 $vf1,0x20(%0)\n\t"
                "sqc2 $vf1,0x10(%0)" : : "r"(scratch) : "memory");
        }
        __asm__ volatile("lqc2 $vf3,0(%1)\n\tlqc2 $vf2,0x20(%2)\n\t"
            "vmul.xyzw $vf1,$vf2,$vf3\n\tvadday.x ACC,$vf1,$vf1y\n\t"
            "vmaddz.x $vf1,$vf4,$vf1z\n\tqmfc2.ni %0,$vf1\n\t"
            "sqc2 $vf1,0(%2)"
            : "=r"(observed) : "r"(input),"r"(scratch) : "memory");
        __asm__ volatile("qmtc2.ni %2,$vf1\n\tvmulx.xyz $vf2,$vf2,$vf1x\n\t"
            "vsub.xyz $vf3,$vf3,$vf2\n\tsqc2 $vf3,0(%0)"
            : : "r"(output),"r"(input),"r"((u32)observed) : "memory");
    } else if(mode==2) {
        u64 observed;
        f32 scratch[4] __attribute__((aligned(16)));
        __asm__ volatile("vmaxw.xyzw $vf1,$vf0,$vf0w\n\tlqc2 $vf4,0x270(%1)\n\t"
            "lqc2 $vf3,0(%2)\n\tvmul.xyzw $vf2,$vf4,$vf3\n\t"
            "vadday.x ACC,$vf2,$vf2y\n\tvmaddz.x $vf1,$vf1,$vf2z\n\t"
            "qmfc2.ni %0,$vf1\n\tsqc2 $vf1,0(%3)"
            : "=r"(observed) : "r"(record),"r"(input),"r"(scratch) : "memory");
        __asm__ volatile("qmtc2.ni %1,$vf1\n\tvmulx.xyz $vf4,$vf4,$vf1x\n\t"
            "vsub.xyz $vf3,$vf3,$vf4\n\tsqc2 $vf3,0(%0)"
            : : "r"(output),"r"((u32)observed) : "memory");
    }
}
