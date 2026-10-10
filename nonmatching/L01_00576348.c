#include "common.h"

/* 30 - PLZCW(signed word) is the index of the highest bit different from
 * its sign. Zero and -1 produce -1; all other values produce 0..30. */
static inline s32 geometry_exponent(s32 value) {
#ifndef DEADLOCKED_NATIVE
    s32 leading;
    __asm__("plzcw %0,%1" : "=r"(leading) : "r"(value));
    return 30 - leading;
#else
    u32 bits = value < 0 ? ~(u32)value : (u32)value;
    s32 exponent = -1;
    while (bits != 0) { exponent++; bits >>= 1; }
    return exponent;
#endif
}

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_geometry.h"
#define GEO_CONFIG DREADZONE_GEOMETRY_CONFIG
#define GEO_FALLBACK DREADZONE_GEOMETRY_FALLBACK
#define GEO_MINUS_THREE DREADZONE_GEOMETRY_FALLBACK_MINUS_THREE
#define READ64(address, value) do { if (!dreadzone_geometry_read(plan, (address), 8, &(value))) return 0; } while (0)
#define READ32(address, value) do { if (!dreadzone_geometry_read(plan, (address), 4, &(value))) return 0; } while (0)
#define READ16(offset, value) do { u64 raw; if (!dreadzone_geometry_read(plan, texture + (offset), 2, &raw)) return 0; (value) = (s16)(u16)raw; } while (0)
#define STORE64(offset, value) do { if (!dreadzone_geometry_write(plan, output + (offset), 8, (value))) return 0; } while (0)
int dreadzone_geometry_emit_textured(DreadzoneGeometryPlan *plan, u32 output, u32 texture,
                                    s32 word0, s32 word4, s32 word10, s32 word14, s32 index) {
#else
extern u32 D_001DFCF0[] __attribute__((section(".data")));
extern u64 D_00269B20[] __attribute__((section(".data")));
extern u64 D_00269B38[] __attribute__((section(".data")));
#define GEO_CONFIG ((u32)D_001DFCF0)
#define GEO_FALLBACK ((u32)D_00269B20)
#define GEO_MINUS_THREE ((u32)D_00269B38)
#define READ64(address, value) do { (value) = *(u64 *)(address); } while (0)
#define READ32(address, value) do { (value) = (u64)(s64)*(s32 *)(address); } while (0)
#define READ16(offset, value) do { (value) = *(s16 *)(texture + (offset)); } while (0)
#define STORE64(offset, value) do { *(u64 *)(output + (offset)) = (value); } while (0)
void func_L01_00576348(u8 *output, u8 *texture, s32 word0, s32 word4, s32 word10, s32 word14, s32 index) {
#endif
    s32 width, height, format, fA, fC, fE, offset, block_width, row_width;
    s32 width_exponent, height_exponent;
    u64 config, first, third;
    u64 q0 = ((u64)(s64)word0 << 32) | ((u64)(s64)word4 << 6) | 0x20;
    READ16(8, format);
    READ16(4, width);
    READ32(GEO_CONFIG + 0x2C, config);
    offset = (s32)(u32)config >> 8;
    READ16(0xE, fE);
    READ16(0xC, fC);
    READ16(0xA, fA);
    READ16(6, height);
    block_width = width >> 7; if (block_width <= 0) block_width = 1;
    row_width = width >> 6; if (row_width <= 0) row_width = 1;
    fA = (s32)((u32)fA + (u32)offset);
    fC = (s32)((u32)fC + (u32)offset);
    fE = (s32)((u32)fE + (u32)offset);
    width_exponent = geometry_exponent(width);
    height_exponent = geometry_exponent(height);
    if (index >= 0) {
        u64 q1 = (u64)(s64)word10 | ((u64)(s64)word14 << 2) | ((u64)(s64)index << 24);
        u64 q2 = ((u64)(s64)row_width << 14) | ((u64)(s64)width_exponent << 26) |
                 ((u64)(s64)height_exponent << 30) | ((u64)(s64)fA << 37) |
                 ((u64)0x8000 << 19) | ((u64)1 << 63);
        u64 q3;
        if (format == 0) {
            s32 base;
            READ32(texture, first);
            READ32(GEO_CONFIG + 0x44, config);
            base = (s32)((u32)config + ((u32)first << 2));
            q2 |= (u64)(s64)(base >> 8) | 0x01B00000;
            q3 = ((((u64)0x8000 << 20) | 0x8000) << 19) | 0x4000;
        } else {
            q0 |= (u64)(s64)(format - 1) << 2;
            q2 |= 0x01300000;
            q3 = ((u64)(s64)block_width << 14) | ((u64)(s64)fC << 20) |
                 ((u64)(s64)fE << 40) | ((u64)0x8000 << 19) | ((u64)0x8000 << 39);
        }
        STORE64(0, q0);
        STORE64(0x10, q1);
        STORE64(0x20, q2);
        STORE64(0x30, q3);
    } else if (index < -1) {
        u32 address = index == -3 ? GEO_MINUS_THREE : GEO_FALLBACK;
        STORE64(0, q0);
        STORE64(0x10, 5);
        READ64(address, first);
        STORE64(0x20, first);
        READ64(address + 16, third);
        STORE64(0x30, third);
    } else {
        STORE64(0, q0);
        STORE64(0x10, 5);
        READ32(GEO_CONFIG + 0x10, config);
        STORE64(0x30, 0);
        STORE64(0x20, (u64)(s64)((s32)(u32)config >> 8) | 0x0C004000 |
                      ((((u64)0x8000 << 29) | 0x9800) << 19));
    }
#ifdef DEADLOCKED_NATIVE
    return 1;
#endif
}
