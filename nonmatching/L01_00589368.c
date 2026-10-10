#include "common.h"

/* Complete original88-byte skeleton-to-local initializer. The retail loop
 * is do/while and requires a positive count. Skeleton quad.w contains the
 * parent/scale-inheritance metadata and is copied without float arithmetic.
 * Actual scratch source2000, destination0, strides16 and64 are preserved. */
#ifndef DEADLOCKED_NATIVE
void func_L01_00589368(s32 count) {
    u8 *source=(u8 *)0x70002000;
    u8 *destination=(u8 *)0x70000000;
    u32 scale_bits=0x4D800000U,translation_bits=0x47800000U;
    __asm__ volatile(
        "qmtc2 %0,$vf20\n\tvaddx.xyz $vf20,$vf0,$vf20x\n\t"
        "vmfir.w $vf20,$vi0\n\tqmtc2 %1,$vf21"
        : : "r"(scale_bits),"r"(translation_bits));
    do {
        __asm__ volatile(
            "lqc2 $vf16,0(%0)\n\tvmulx.xyz $vf16,$vf16,$vf21x\n\t"
            "sqc2 $vf0,0(%1)\n\tsqc2 $vf20,0x10(%1)\n\t"
            "sqc2 $vf16,0x20(%1)\n\tsqc2 $vf16,0x30(%1)"
            : : "r"(source),"r"(destination) : "memory");
        source+=16;destination+=64;
    } while (--count!=0);
}
#endif
