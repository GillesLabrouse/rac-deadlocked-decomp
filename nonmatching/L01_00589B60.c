#include "common.h"

/* Complete L01 00589B60..00589CDC, 384 retail bytes. The two genuine VU0
 * entry points require MPG block00117A60 and its surrounding resident state.
 * The source preserves the paired pipeline and its last lookahead loads.
 * No native body or completed DMA claim is provided here.
 * Nonmatching: compiled extent352 bytes; DMA setup is in a local helper. */
#ifndef DEADLOCKED_NATIVE
static void bind_transfer(u32 source, u32 qwords, u32 scratch_offset) {
    volatile u32 *dma = (volatile u32 *)0x1000D400;
    dma[0x80 / 4] = scratch_offset;
    dma[0x20 / 4] = qwords;
    dma[0x10 / 4] = source;
    dma[0] = 0x100;
    while ((dma[0] & 0x100) != 0) {
        __asm__ volatile("nop\n\tnop\n\tnop\n\tnop");
    }
}

void func_L01_00589B60(u8 *poses, u8 *bindings, s32 count) {
    u8 *scratch = (u8 *)0x70000000;
    u8 *last, *cursor, *next_binding;
    s32 i;
    if (count == 0) {
        for (i = 0; i < 16; i++)
            ((u32 *)scratch)[i] = i % 5 == 0 ? 0x3F800000U : 0;
        return;
    }
    if (poses != scratch)
        bind_transfer((u32)poses, (u32)count << 2, 0);
    bind_transfer((u32)bindings, (u32)count * 3, (u32)count << 6);
    last = scratch + ((u32)count << 6) - 64;
    next_binding = last + 112;
    cursor = scratch;
    __asm__ volatile(
        "lqc2 $vf1,0x40(%0)\n\tlqc2 $vf2,0x50(%0)\n\tlqc2 $vf3,0x60(%0)\n\t"
        "lqc2 $vf5,0(%1)\n\tlqc2 $vf6,0x10(%1)\n\t"
        "lqc2 $vf7,0x20(%1)\n\tlqc2 $vf8,0x30(%1)\n\tvcallms 0xA40\n\tnop"
        : : "r"(last), "r"(cursor) : "memory");
    for (;;) {
        __asm__ volatile(
            "lqc2 $vf11,0(%0)\n\tlqc2 $vf12,0x10(%0)\n\tlqc2 $vf13,0x20(%0)\n\t"
            "lqc2 $vf15,0x40(%1)\n\tlqc2 $vf16,0x50(%1)\n\t"
            "lqc2 $vf17,0x60(%1)\n\tlqc2 $vf18,0x70(%1)\n\tvcallms 0xAA8\n\tnop\n\t"
            "sqc2 $vf20,0(%1)\n\tsqc2 $vf21,0x10(%1)\n\t"
            "sqc2 $vf22,0x20(%1)\n\tsqc2 $vf23,0x30(%1)"
            : : "r"(next_binding), "r"(cursor) : "memory");
        next_binding += 48;
        if (cursor == last) return;
        cursor += 64;
        __asm__ volatile(
            "lqc2 $vf1,0(%0)\n\tlqc2 $vf2,0x10(%0)\n\tlqc2 $vf3,0x20(%0)\n\t"
            "lqc2 $vf5,0x40(%1)\n\tlqc2 $vf6,0x50(%1)\n\t"
            "lqc2 $vf7,0x60(%1)\n\tlqc2 $vf8,0x70(%1)\n\tvcallms 0xA40\n\tnop\n\t"
            "sqc2 $vf24,0(%1)\n\tsqc2 $vf25,0x10(%1)\n\t"
            "sqc2 $vf26,0x20(%1)\n\tsqc2 $vf27,0x30(%1)"
            : : "r"(next_binding), "r"(cursor) : "memory");
        next_binding += 48;
        if (cursor == last) return;
        cursor += 64;
    }
}
#endif
