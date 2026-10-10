#include "common.h"

/* Complete L01 0058DF58..0058E0D8. This is a PS2 actor transform update,
 * not a CPU-only position setter. The installed VCALLMS B58 program returns
 * vf20/vf21/vf22; actor.50 is a live list of weighted animation vectors.
 * It writes the scaled world vector, saturating transform generation, then
 * genuinely updates spatial cells and (when byte55&A0) the model transform.
 *
 * VU instructions retain ACC rounding, masked W lanes, VF0, VFTOI and the
 * packed coordinate conversion. Those semantics cannot be established by
 * replacing this body with host float matrix multiplication. The actual
 * spatial0058DC78 and skeleton/DMA0058A648 remain unresolved calls. Actor
 * storage must be16-byte aligned and contain valid animation/list/model
 * pointers; negative actor.byte20 skips every update. No native entry exists.
 * Nonmatching: the compiled function is 428 bytes versus 384 retail bytes.
 */
#ifndef DEADLOCKED_NATIVE
typedef unsigned int ActorTransformQuad __attribute__((mode(TI)));
void func_L01_0058DC78(u8 *,u32);
void func_L01_0058A648(u8 *);

#define ATBYTE(p,o) (*(volatile u8 *)((p)+(o)))
#define ATSBYTE(p,o) (*(volatile s8 *)((p)+(o)))
#define ATWORD(p,o) (*(volatile u32 *)((p)+(o)))

void func_L01_0058DF58(u8 *actor)
{
    u16 flags;
    u8 *node;
    s32 generation;
    s8 animationIndex,previousIndex;
    u32 scale=0x44800000U;
    ActorTransformQuad packed;
    u64 key,previous;
    if(ATSBYTE(actor,0x20)<0) return;
    flags=*(volatile u16 *)(actor+0x34);
    __asm__ volatile("lqc2 $vf15,0xF0(%0)\n\tnop" : : "r"(actor) : "memory");
    if((flags&0x100)==0) {
        __asm__ volatile("vcallms 0xB58\n\tnop\n\tvnop" : : : "memory");
        if(flags&0x8000)
            __asm__ volatile("vsub.xyz $vf21,$vf0,$vf21" : : : "memory");
        __asm__ volatile("sqc2 $vf20,0xC0(%0)\n\tsqc2 $vf21,0xD0(%0)\n\tsqc2 $vf22,0xE0(%0)"
            : : "r"(actor) : "memory");
    } else {
        __asm__ volatile("lqc2 $vf20,0xC0(%0)\n\tlqc2 $vf21,0xD0(%0)\n\tlqc2 $vf22,0xE0(%0)"
            : : "r"(actor) : "memory");
    }
    animationIndex=ATSBYTE(actor,0x54);
    previousIndex=ATSBYTE(actor,0x56);
    __asm__ volatile("lqc2 $vf16,0x80(%0)" : : "r"(actor) : "memory");
    if(animationIndex!=previousIndex) {
        u8 *animation=(u8 *)ATWORD(actor,0x40);
        ATBYTE(actor,0x56)=(u8)animationIndex;
        __asm__ volatile("lqc2 $vf16,0(%0)\n\tsqc2 $vf16,0x80(%1)"
            : : "r"(animation),"r"(actor) : "memory");
    }
    node=(u8 *)ATWORD(actor,0x50);
    while(node!=NULL) {
        u8 *vector=(u8 *)ATWORD(node,0);
        __asm__ volatile("lqc2 $vf18,0x10(%0)\n\tvaddax.xyzw ACC,$vf16,$vf0x\n\t"
            "lqc2 $vf17,0(%1)\n\tvmsubaz.xyzw ACC,$vf16,$vf18z\n\t"
            "vmaddz.xyzw $vf16,$vf17,$vf18z"
            : : "r"(node),"r"(vector) : "memory");
        /* +10 is both the next guest pointer and vf18.x raw lane. */
        node=(u8 *)ATWORD(node,0x10);
    }
    generation=*(volatile s16 *)(actor+0x94);
    __asm__ volatile("lqc2 $vf17,0x70(%0)\n\tqmtc2.ni %1,$vf18\n\t"
        "vmulx.xyzw $vf16,$vf16,$vf17x\n\tlqc2 $vf17,0x10(%0)\n\t"
        "vmulax.xyz ACC,$vf20,$vf16x\n\tvmadday.xyz ACC,$vf21,$vf16y\n\t"
        "vmaddaz.xyz ACC,$vf22,$vf16z\n\tvmaddx.xyz $vf16,$vf17,$vf18x"
        : : "r"(actor),"r"(scale) : "memory");
    /* PADDSH's low signed half saturates at32767, including32767+1. */
    if(generation<32767) ++generation;
    *(volatile s16 *)(actor+0x94)=(s16)generation;
    __asm__ volatile("sqc2 $vf16,0(%0)" : : "r"(actor) : "memory");
    __asm__ volatile("vaddx.z $vf17,$vf0,$vf16x\n\tvsubw.xy $vf17,$vf16,$vf16w"
        : : : "memory");
    if(ATWORD(actor,0x98)!=0) {
        previous=(u64)ATWORD(actor,0xA4);
        __asm__ volatile("vaddy.w $vf17,$vf16,$vf16y\n\t"
            "vaddw.z $vf17,$vf17,$vf16w\n\tvftoi0.xyzw $vf17,$vf17\n\t"
            "qmfc2.ni %0,$vf17\n\tpsrlw %0,%0,14\n\tppach %0,$0,%0\n\t"
            "ppacb %0,$0,%0\n\tqmtc2.ni $0,$vf0"
            : "=&r"(packed) : : "memory");
        key=(u64)packed;
        if(previous!=key && (key&0xC0C0U)==0)
            func_L01_0058DC78(actor,(u32)key);
    }
    if(ATBYTE(actor,0x55)&0xA0) func_L01_0058A648(actor);
}
#endif
