/* cflags: -ffast-math */
/* Complete 112-byte angular projection and directed wrap; NONMATCHING. */
#include "common.h"
typedef unsigned int ParameterQuad __attribute__((mode(TI)));
typedef union ParameterLanes {ParameterQuad quad; f32 lanes[4];} ParameterLanes;
extern f32 func_L01_0055D708(f32,f32);
extern f32 func_L01_00595188(s32,f32,f32);
f32 func_L01_00683580(const u8 *record,const void *vector,s32 direction) {
    ParameterLanes transformed;
    f32 angle;
    __asm__ volatile("lqc2 $vf3,0(%2)\n\tlqc2 $vf2,0(%1)\n\t"
        "vmulax.xyzw ACC,$vf2,$vf3x\n\tlqc2 $vf1,16(%1)\n\t"
        "vmadday.xyzw ACC,$vf1,$vf3y\n\tlqc2 $vf2,32(%1)\n\t"
        "lqc2 $vf1,48(%1)\n\tvmaddaz.xyzw ACC,$vf2,$vf3z\n\t"
        "vmaddw.xyzw $vf2,$vf1,$vf3w\n\tqmfc2.ni %0,$vf2"
        : "=r"(transformed.quad):"r"(record+0x60),"r"(vector):"memory");
    angle=func_L01_0055D708(transformed.lanes[0],transformed.lanes[1]);
    return func_L01_00595188(direction,angle,0.f);
}
