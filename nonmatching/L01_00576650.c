#include "common.h"

/* The metadata splits the first return at 005766F0. Real internal branches
 * continue through 005766F0 and 00576750 using inherited t4/t3 temporaries;
 * this recovered routine includes that complete 0x168-byte control-flow span.
 */

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_geometry.h"
#define GEO_TABLE DREADZONE_GEOMETRY_TABLE
#define GEO_FALLBACK DREADZONE_GEOMETRY_FALLBACK
#define GEO_MINUS_THREE DREADZONE_GEOMETRY_FALLBACK_MINUS_THREE
#define GEO_CONFIG_WORD (DREADZONE_GEOMETRY_CONFIG + 0x10)
#define READ64(address, value) do { if (!dreadzone_geometry_read(plan, (address), 8, &(value))) return 0; } while (0)
#define READ32(address, value) do { if (!dreadzone_geometry_read(plan, (address), 4, &(value))) return 0; } while (0)
#define STORE64(offset, value) do { if (!dreadzone_geometry_write(plan, output + (offset), 8, (value))) return 0; } while (0)
int dreadzone_geometry_emit_table(DreadzoneGeometryPlan *plan, u32 output,
                                 s32 word0, s32 word4, s32 word10, s32 word14, s32 index) {
#else
extern u64 D_002699A0[] __attribute__((section(".data")));
extern u64 D_00269B20[] __attribute__((section(".data")));
extern u64 D_00269B38[] __attribute__((section(".data")));
extern s32 D_001DFD00 __attribute__((section(".data")));
#define GEO_TABLE ((u32)D_002699A0)
#define GEO_FALLBACK ((u32)D_00269B20)
#define GEO_MINUS_THREE ((u32)D_00269B38)
#define GEO_CONFIG_WORD ((u32)&D_001DFD00)
#define READ64(address, value) do { (value) = *(u64 *)(address); } while (0)
#define READ32(address, value) do { (value) = (u64)(s64)*(s32 *)(address); } while (0)
#define STORE64(offset, value) do { *(u64 *)(output + (offset)) = (value); } while (0)
void func_L01_00576650(u8 *output, s32 word0, s32 word4, s32 word10, s32 word14, s32 index) {
#endif
    u64 first, second, third, config;
    u32 address = GEO_TABLE + (u32)index * 24;
    u64 q0 = ((u64)(s64)word0 << 32) | ((u64)(s64)word4 << 6) | 0x20;
    /* All three table words are read even for a negative index. */
    READ64(address, first);
    READ64(address + 8, second);
    READ64(address + 16, third);
    if (index >= 0) {
        STORE64(0, q0 | (second & 0x1C));
        STORE64(0x10, (u64)(s64)word10 | ((u64)(s64)word14 << 2) | ((u64)(s64)index << 24));
        STORE64(0x30, third);
        STORE64(0x20, first);
    } else if (index < -1) {
        address = index == -3 ? GEO_MINUS_THREE : GEO_FALLBACK;
        STORE64(0, q0);
        STORE64(0x10, 5);
        READ64(address, first);
        STORE64(0x20, first);
        READ64(address + 16, third);
        STORE64(0x30, third);
    } else {
        STORE64(0, q0);
        STORE64(0x10, 5);
        READ32(GEO_CONFIG_WORD, config);
        STORE64(0x30, 0);
        STORE64(0x20, (u64)(s64)((s32)(u32)config >> 8) | 0x0C004000 |
                      ((((u64)0x8000 << 29) | 0x9800) << 19));
    }
#ifdef DEADLOCKED_NATIVE
    return 1;
#endif
}
