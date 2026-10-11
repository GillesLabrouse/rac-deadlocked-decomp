/* cflags: -mno-split-addresses */
/* asflags: --float-immediates */
/* Complete original 00593478..005935C4, 332 bytes, NONMATCHING.
 * Input is a genuine quad passed by value in a0. The real override634948
 * remains an external dependency; its success is not fabricated. The second
 * override is selected by actual Z-distance and XY-radius comparisons.
 * The supplied normal output is overwritten only on the original fallback
 * routes. Global reads following the optional callback remain ordered. */
#include "common.h"
#ifndef DEADLOCKED_NATIVE
typedef unsigned int GroundHeightQuad __attribute__((mode(TI)));
typedef union GroundHeightLanes {GroundHeightQuad quad;f32 lane[4];} GroundHeightLanes;
extern volatile s32 D_002237D8,D_002237DC;
extern u8 D_002237C0[];
s32 func_L01_00634948(f32 *,f32,f32,f32);
f32 func_L01_00593478(GroundHeightQuad point,u8 *normal) {
    GroundHeightLanes input,special;f32 height,distance;
    input.quad=point;
    if(D_002237D8!=0) {
        height=input.lane[2];
        if(func_L01_00634948(&height,input.lane[0],input.lane[1],input.lane[2])!=0)
            return height;
    }
    if(D_002237DC!=0) {
        special.quad=*(volatile GroundHeightQuad *)D_002237C0;
        height=input.lane[2]-special.lane[2];
        __asm__ volatile("abs.s %0,%1":"=f"(height):"f"(height));
        if(height<0.5f) {
            union {f32 value;u32 bits;}scalar;
            __asm__ volatile("qmtc2.ni %1,$vf1\n\tqmtc2.ni %2,$vf2\n\t"
                "vsub.xyz $vf1,$vf1,$vf2\n\tvmul.xyzw $vf1,$vf1,$vf1\n\t"
                "vaddy.x $vf1,$vf1,$vf1y\n\tnop\n\tnop\n\t"
                "vsqrt Q,$vf1x\n\tvwaitq\n\tvaddq.x $vf1,$vf0,Q\n\t"
                "qmfc2.ni %0,$vf1" : "=r"(scalar.bits)
                : "r"(point),"r"(special.quad) : "memory");
            distance=scalar.value;
            if(distance<*(volatile f32 *)(D_002237C0+0xC)) {
                if(normal!=NULL) {
                    __asm__ volatile("vmr32.xyzw $vf1,$vf0\n\tsqc2 $vf1,0(%0)"
                        : : "r"(normal) : "memory");
                }
                return *(volatile f32 *)(D_002237C0+8);
            }
        }
    }
    if(normal!=NULL) {
        __asm__ volatile("vmr32.xyzw $vf1,$vf0\n\tsqc2 $vf1,0(%0)"
            : : "r"(normal) : "memory");
    }
    return input.lane[2];
}
#endif
