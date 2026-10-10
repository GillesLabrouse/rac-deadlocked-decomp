/* cflags: -mno-split-addresses */
/* Actual core0015A170, retail968 bytes. NONMATCHING experimental recovery.
 * The retail function includes a handwritten Count read; its PS2-only inline
 * instruction preserves that boundary. Native mode1/level1 execution does not
 * substitute a timer, a transport, or any network result. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_core_startup.h"
#define WORD(offset) (host->block->words[(offset) / 4])
#define BYTE(offset) (host->block->bytes[offset])
#define LEVEL host->level_0021DE10
int dreadzone_core_0015A170_slice(DreadzoneCoreStartupContext *host) {
    s32 i;
    if (!host || !host->block) return DREADZONE_CORE_STARTUP_INVALID_STORAGE;
#else
extern u32 D_00172160[];
extern s32 D_0021DE10[];
extern u32 D_0021DFE8[];
extern s32 D_0021DF9C;
void func_0015E7A8(void);
void func_00162840(s32, s32);
s32 func_00161320(void);
void *func_0011A518(void *, s32, u32);
s32 func_01EA2750(void *);
void func_001633C0(u32);
s32 func_001633D0(void);
void func_00162698(void *);
s32 func_01E9E360(void *, void *);
u64 func_0012E370(void);
u64 func_0012FEB0(u64, u64);
void func_0015A028(void);
void func_0015A050(void);
s32 func_01E9E798(void);
s32 func_01E9E6B0(s32 *, s32);
extern u8 func_0015B298[], func_0015B2A0[], func_0015B2B8[], func_0015B2C0[];
#define WORD(offset) (D_00172160[(offset) / 4])
#define BYTE(offset) (((u8 *)D_00172160)[offset])
#define LEVEL D_0021DE10[0]
s32 func_0015A170(const void *argument) {
    u32 request[0x164 / 4], result[5], counter;
    const u8 *source;
    u8 *destination;
    s32 i, value, status, start;
#endif
    BYTE(0x1681) = 0;
    if ((s32)WORD(0) >= 7) return 0;
#ifdef DEADLOCKED_NATIVE
    if (LEVEL == 0) return DREADZONE_CORE_STARTUP_NEEDS_LEVEL_ZERO;
    if (WORD(0x258) != 1) return DREADZONE_CORE_STARTUP_NEEDS_NETWORK;
#else
    if (LEVEL == 0) {
        func_0015E7A8();
        D_0021DFE8[0] = 0;
        for (i = 0; i < 10; i++) func_00162840(i, 1);
    }
#endif
    for (i = 0; i < 10; i++) {
        WORD(0x644 + 4 * i) = 0xFFFFFFFFU;
        BYTE(0x7F9 + i) = 0;
        BYTE(0x803 + 16 * i) = 0;
        BYTE(0x8A3 + i) = 255;
        WORD(0x8B0 + 4 * i) = 0xFFFFFFFFU;
    }
    if (WORD(0x258) == 1) { WORD(8) = 0; WORD(0) = 7; return 0; }
#ifndef DEADLOCKED_NATIVE
    if ((s32)WORD(0) >= 10) return 0;
    D_0021DF9C = 0;
    if (func_00161320() && BYTE(0x260)) { WORD(0) = 7; return 0; }
    WORD(0) = 6;
    func_0011A518(request, 0, 0x164);
    func_0011A518(result, 0, 12);
    if (func_01EA2750(request)) return 1;
    __asm__ volatile("mfc0 %0,$9\n\tsync.p" : "=r"(counter));
    func_001633C0(counter);
    value = func_001633D0() % 500 + 6500;
    /* Retail copies156 bytes with aligned/unaligned variants. This C recovery
     * preserves the byte extent; matching their scheduling remains pending. */
    source = (const u8 *)argument;
    destination = (u8 *)request + 8;
    for (i = 0; i < 156; i++) destination[i] = source[i];
    request[0xA4 / 4] = (u32)func_0015B298;
    request[0xAC / 4] = (u32)func_0015B2A0;
    request[0xB4 / 4] = (u32)func_0015B2B8;
    request[0xBC / 4] = (u32)func_0015B2C0;
    request[1] = 10;
    request[0x148 / 4] = value;
    request[0x160 / 4] = 0x4000;
    request[0xC4 / 4] = 0;
    request[0xC8 / 4] = 0;
    request[0x15C / 4] = 0x4000;
    func_00162698((u8 *)request + 0xD4);
    result[4] = 2;
    if (func_01E9E360(request, result)) return 1;
    start = (s32)func_0012FEB0(func_0012E370(), 0x24000);
    func_0015A028();
    while (result[4] == 2) {
        func_0015A050();
        status = func_01E9E798();
        if (status != 12) {
            if (status) goto failed;
            value = (s32)func_0012FEB0(func_0012E370(), 0x24000);
            /* Retail subu truncates before signed slti, including wraparound. */
            if ((s32)((u32)value - (u32)start) >= 15001) goto failed;
        }
        if (func_01E9E6B0((s32 *)&result[4], (s32)result[2])) goto failed;
    }
    if (result[4] == 1) { WORD(0) = 7; WORD(8) = result[2]; return 0; }
failed:
    WORD(0) = 8;
    return 1;
#else
    return DREADZONE_CORE_STARTUP_NEEDS_NETWORK;
#endif
}
