/* cflags: -ffast-math */
/* Complete 172-byte vector projection plus four-byte padding, NONMATCHING.
 * Modes1/2 use both actual record matrices; native selected path is mode0. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_direction.h"
#endif
#include "../include/dreadzone_actor_direction_internal.h"
#ifdef DEADLOCKED_NATIVE
int dreadzone_direction_projection_bytes(u8 *record,u8 *out,const u8 *in,f32 height) {
    if(record[0x2659]) return DREADZONE_DIRECTION_UNSUPPORTED_MODE;
    memmove(out,in,16);dreadzone_direction_store_float(out+8,0.f+height);
    return 0;
}
int func_L01_0068ABA8(u8 *record,DirectionVector *out,const DirectionVector *in,f32 height) {
    return dreadzone_direction_projection_bytes(record,(u8 *)out,(const u8 *)in,height);
}
#else
void func_L01_0068ABA8(u8 *record,DirectionVector *out,const DirectionVector *in,f32 height) {
    union {f32 f;u32 u;} bits;bits.f=height;
    if(!record[0x2659]) {
        __asm__ volatile("lqc2 $vf1,0(%1)\n\tqmtc2.ni %2,$vf2\n\t"
            "sqc2 $vf1,0(%0)\n\tvaddx.z $vf1,$vf0,$vf2x\n\tsqc2 $vf1,0(%0)"
            : : "r"(out),"r"(in),"r"(bits.u) : "memory");
    } else if(record[0x2659]<3) {
        DirectionVector projected;
        __asm__ volatile("lqc2 $vf3,0(%2)\n\tlqc2 $vf2,0x60(%0)\n\t"
            "lqc2 $vf1,0x70(%0)\n\tvmulax.xyzw ACC,$vf2,$vf3x\n\t"
            "lqc2 $vf4,0x80(%0)\n\tvmadday.xyzw ACC,$vf1,$vf3y\n\t"
            "lqc2 $vf2,0x90(%0)\n\tvmaddaz.xyzw ACC,$vf4,$vf3z\n\t"
            "vmaddw.xyzw $vf2,$vf2,$vf3w\n\tqmtc2.ni %3,$vf1\n\t"
            "vmove.xyzw $vf3,$vf2\n\tvaddx.z $vf3,$vf0,$vf1x\n\t"
            "sqc2 $vf2,0(%4)\n\tsqc2 $vf3,0(%1)\n\t"
            "lqc2 $vf1,0x20(%0)\n\tlqc2 $vf2,0x30(%0)\n\t"
            "vmulax.xyzw ACC,$vf1,$vf3x\n\tlqc2 $vf4,0x40(%0)\n\t"
            "vmadday.xyzw ACC,$vf2,$vf3y\n\tlqc2 $vf1,0x50(%0)\n\t"
            "vmaddaz.xyzw ACC,$vf4,$vf3z\n\tvmaddw.xyzw $vf1,$vf1,$vf3w\n\t"
            "sqc2 $vf1,0(%1)\n\tsqc2 $vf1,0(%4)"
            : : "r"(record),"r"(out),"r"(in),"r"(bits.u),"r"(&projected) : "memory");
    }
}
#endif
