#include "common.h"

/* Complete 00596390..00596470 (0xE0 bytes), no direct callees. For each
 * negative second-vector xyz component, project the whole xyz segment using
 * abs((first.component - binary32(0.1)) / delta.component). This retains
 * the actual successive second-vector reloads and first-vector aliases.
 * VU xyz subtraction/multiply/add preserve their original W sources.
 * The actual COP1 division/ABS remains in the PS2 body: zero-denominator
 * behavior is not replaced with a host infinity or a fabricated success.
 * Both vectors are aligned quads; no native entry exists.
 */
#ifndef DEADLOCKED_NATIVE
typedef unsigned int ActorClipQuad __attribute__((mode(TI)));
typedef union ActorClipLanes { ActorClipQuad quad;f32 lane[4]; } ActorClipLanes;
typedef union ActorClipBits { u32 bits;f32 value; } ActorClipBits;

void func_L01_00596390(u8 *first,u8 *second)
{
    s32 index;
    ActorClipBits margin,scale;
    ActorClipLanes current,base,delta;
    margin.bits=0x3DCCCCCDU;
    for(index=0;index<3;index++) {
        current.quad=*(volatile ActorClipQuad *)second;
        if(current.lane[index]<0.0f) {
            __asm__ volatile("lqc2 $vf2,0(%2)\n\tqmtc2.ni %3,$vf1\n\t"
                "vsub.xyz $vf1,$vf1,$vf2\n\tqmfc2.ni %0,$vf2\n\t"
                "qmfc2.ni %1,$vf1"
                : "=r"(base.quad),"=r"(delta.quad)
                : "r"(first),"r"(current.quad) : "memory");
            scale.value=(base.lane[index]-margin.value)/delta.lane[index];
            __asm__ volatile("abs.s %0,%1" : "=f"(scale.value) : "f"(scale.value));
            __asm__ volatile("qmtc2.ni %1,$vf3\n\tvmulx.xyz $vf1,$vf1,$vf3x\n\t"
                "vadd.xyz $vf2,$vf2,$vf1\n\tsqc2 $vf2,0(%0)"
                : : "r"(second),"r"(scale.bits) : "memory");
        }
    }
}
#endif
