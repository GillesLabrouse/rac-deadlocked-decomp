/* cflags: -mno-split-addresses */
#include "common.h"

/* Complete 0068A340..0068A3F8, 184 bytes, including all ordered stores.
 * ABI: record, output, add_position; distance/angle/height in f12/f13/f14.
 * C60 is the actual resident trigonometric microprogram. Prior output XYZ
 * is discarded, but its W survives the masked write and the full body
 * transform. This is not an initialized direction constructor.
 */
#ifndef DEADLOCKED_NATIVE
typedef unsigned int SideOffsetQuad __attribute__((mode(TI)));
typedef union SideOffsetLanes { SideOffsetQuad q;f32 lane[4]; } SideOffsetLanes;
typedef union SideOffsetBits { u32 bits;f32 value; } SideOffsetBits;

void func_L01_0068A340(u8 *record,u8 *output,s32 add_position,
                      f32 distance,f32 angle,f32 height) {
    SideOffsetLanes trig,offset;
    SideOffsetBits input,x,y,z;
    input.value=angle;
    __asm__ volatile("qmtc2.ni %1,$vf15\n\tvcallms 0xC60\n\tqmfc2.i %0,$vf15"
        : "=r"(trig.q) : "r"(input.bits) : "memory");
    x.value=trig.lane[1]*distance;y.value=trig.lane[0]*distance;z.value=height;
    offset.lane[0]=x.value;offset.lane[1]=y.value;
    offset.lane[2]=0.f;offset.lane[3]=0.f;
    __asm__ volatile("lqc2 $vf1,0(%0)\n\tqmtc2.ni %1,$vf2\n\t"
        "vmove.xyz $vf1,$vf2\n\tqmtc2.ni %2,$vf2\n\t"
        "vaddx.z $vf1,$vf0,$vf2x\n\tsqc2 $vf1,0(%0)"
        : : "r"(output),"r"(offset.q),"r"(z.bits) : "memory");
    __asm__ volatile("lqc2 $vf3,0(%1)\n\tlqc2 $vf1,0x20(%0)\n\t"
        "lqc2 $vf2,0x30(%0)\n\tvmulax.xyzw ACC,$vf1,$vf3x\n\t"
        "lqc2 $vf1,0x40(%0)\n\tvmadday.xyzw ACC,$vf2,$vf3y\n\t"
        "lqc2 $vf2,0x50(%0)\n\tvmaddaz.xyzw ACC,$vf1,$vf3z\n\t"
        "vmaddw.xyzw $vf2,$vf2,$vf3w\n\tsqc2 $vf2,0(%1)"
        : : "r"(record),"r"(output) : "memory");
    if(add_position) {
        __asm__ volatile("lqc2 $vf1,0xA0(%0)\n\tvadd.xyz $vf2,$vf2,$vf1\n\t"
            "sqc2 $vf2,0(%1)" : : "r"(record),"r"(output) : "memory");
    }
}
#endif
