#include "common.h"

/* Complete528-byte normal-matrix orchestrator recovered as typed control flow.
 * The actual two VU entry points remain, including speculative lookahead even
 * when the count is odd. Original E90/E10/F00/F40 save/restore the caller's VF
 * state and EE saved-register upper halves. The real fromSPR DMA is launched
 * only when destination differs from scratch; retail does not wait here.
 * This draft is nonmatching and has no fake-success native implementation. */
#ifndef DEADLOCKED_NATIVE
void func_L01_00589950(u8 *destination,s32 count,u8 *unused_scratch) {
    u8 *cursor=(u8 *)0x70000000;
    u8 *parent;
    s32 remaining,even_flags,odd_flags;
    u32 halves[4] __attribute__((aligned(16)));
    u32 half=0x3F000000U;
    volatile u32 *dma=(volatile u32 *)0x1000D000;
    (void)unused_scratch;
    if(count==0) return;
    halves[0]=halves[2]=0x31800000U;
    halves[1]=halves[3]=0x37800000U;
    __asm__ volatile(
        "vcallms 0xE90\n\tnop\n\t"
        "qmtc2 $16,$vf16\n\tqmtc2 $17,$vf17\n\tqmtc2 $18,$vf18\n\t"
        "qmtc2 $19,$vf19\n\tqmtc2 $20,$vf20\n\tqmtc2 $21,$vf21\n\t"
        "qmtc2 $22,$vf22\n\tqmtc2 $23,$vf23\n\tvcallms 0xE10\n\tvnop\n\t"
        "vmr32.xyzw $vf10,$vf0\n\tvmr32.xyzw $vf9,$vf10\n\t"
        "vmr32.xyzw $vf8,$vf9\n\tvmove.xyzw $vf11,$vf0\n\t"
        "lqc2 $vf24,0(%0)\n\tqmtc2 %1,$vf1\n\t"
        "vaddx.xyz $vf28,$vf0,$vf1x\n\tvmulx.w $vf28,$vf0,$vf1x\n\t"
        "vmove.xw $vf25,$vf28\n\tvsub.yz $vf25,$vf0,$vf28\n\t"
        "vmove.yw $vf26,$vf28\n\tvsub.xz $vf26,$vf0,$vf28\n\t"
        "vmr32.xyzw $vf27,$vf25\n\tctc2 %2,$vi2\n\t"
        "sqc2 $vf8,0x3FC0(%3)\n\tsqc2 $vf9,0x3FD0(%3)\n\t"
        "sqc2 $vf10,0x3FE0(%3)\n\tsqc2 $vf11,0x3FF0(%3)\n\t"
        "lqc2 $vf1,0(%3)\n\tlqc2 $vf2,0x10(%3)\n\t"
        "lqc2 $vf3,0x20(%3)\n\tlqc2 $vf23,0x30(%3)\n\tvcallms 0x738"
        : : "r"(halves),"r"(half),"r"(-1),"r"(cursor) : "memory");
    even_flags=(s8)cursor[0x3C];remaining=count;
    for(;;) {
        odd_flags=(s8)cursor[0x7C];
        parent=(u8 *)0x70002000+((odd_flags&127)<<6);
        __asm__ volatile(
            "lqc2 $vf12,0x40(%0)\n\tlqc2 $vf13,0x50(%0)\n\t"
            "lqc2 $vf14,0x60(%0)\n\tlqc2 $vf29,0x70(%0)\n\t"
            "lqc2 $vf19,0(%1)\n\tlqc2 $vf20,0x10(%1)\n\t"
            "lqc2 $vf21,0x20(%1)\n\tlqc2 $vf22,0x30(%1)\n\tvcallms 0x858\n\t"
            "sqc2 $vf4,0x2030(%0)\n\tsqc2 $vf1,0x2000(%0)\n\t"
            "sqc2 $vf2,0x2010(%0)\n\tsqc2 $vf3,0x2020(%0)\n\tsqc2 $vf4,0x30(%0)"
            : : "r"(cursor),"r"(parent) : "memory");
        if(even_flags<0) {
            __asm__ volatile("sqc2 $vf1,0(%0)\n\tsqc2 $vf2,0x10(%0)\n\tsqc2 $vf3,0x20(%0)" : : "r"(cursor) : "memory");
        } else {
            __asm__ volatile("sqc2 $vf5,0(%0)\n\tsqc2 $vf6,0x10(%0)\n\tsqc2 $vf7,0x20(%0)" : : "r"(cursor) : "memory");
        }
        cursor+=64;
        if(--remaining==0) break;
        even_flags=(s8)cursor[0x7C];
        parent=(u8 *)0x70002000+((even_flags&127)<<6);
        __asm__ volatile(
            "lqc2 $vf1,0x40(%0)\n\tlqc2 $vf2,0x50(%0)\n\t"
            "lqc2 $vf3,0x60(%0)\n\tlqc2 $vf23,0x70(%0)\n\t"
            "lqc2 $vf8,0(%1)\n\tlqc2 $vf9,0x10(%1)\n\t"
            "lqc2 $vf10,0x20(%1)\n\tlqc2 $vf11,0x30(%1)\n\tvcallms 0x738\n\t"
            "sqc2 $vf15,0x2030(%0)\n\tsqc2 $vf12,0x2000(%0)\n\t"
            "sqc2 $vf13,0x2010(%0)\n\tsqc2 $vf14,0x2020(%0)\n\tsqc2 $vf15,0x30(%0)"
            : : "r"(cursor),"r"(parent) : "memory");
        if(odd_flags<0) {
            __asm__ volatile("sqc2 $vf12,0(%0)\n\tsqc2 $vf13,0x10(%0)\n\tsqc2 $vf14,0x20(%0)" : : "r"(cursor) : "memory");
        } else {
            __asm__ volatile("sqc2 $vf16,0(%0)\n\tsqc2 $vf17,0x10(%0)\n\tsqc2 $vf18,0x20(%0)" : : "r"(cursor) : "memory");
        }
        cursor+=64;
        if(--remaining==0) break;
    }
    __asm__ volatile(
        "vcallms 0xF00\n\tvcallms 0xF40\n\t"
        "qmfc2 $16,$vf16\n\tqmfc2 $17,$vf17\n\tqmfc2 $18,$vf18\n\t"
        "qmfc2 $19,$vf19\n\tqmfc2 $20,$vf20\n\tqmfc2 $21,$vf21\n\t"
        "qmfc2 $22,$vf22\n\tqmfc2 $23,$vf23"
        : : : "$16","$17","$18","$19","$20","$21","$22","$23","memory");
    if(destination!=(u8 *)0x70000000) {
        dma[0x80/4]=0;
        dma[0x20/4]=(u32)count<<2;
        dma[0x10/4]=(u32)destination;
        dma[0]=0x100;
    }
}
#endif
