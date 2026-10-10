/* cflags: -mno-split-addresses */
/* Actual core0015AA58, retail472 bytes. NONMATCHING experimental recovery.
 * Native execution supports mode1, its real state>=10 exit, and explicit
 * boundaries before the remaining network/callback paths. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_core_startup.h"
#define WORD(offset) (host->block->words[(offset) / 4])
#define BYTE(offset) (host->block->bytes[offset])
#define LEVEL host->level_0021DE10
#define FLAG host->flag_0021DFA4
int dreadzone_core_0015AA58_slice(DreadzoneCoreStartupContext *host) {
    s32 i, ignored;
    int status;
    if (!host || !host->block) return DREADZONE_CORE_STARTUP_INVALID_STORAGE;
    if ((s32)WORD(0) >= 10) return DREADZONE_CORE_STARTUP_COMPLETE;
    if (WORD(0x258) != 1) return DREADZONE_CORE_STARTUP_NEEDS_NETWORK;
    status = dreadzone_core_0015F4A0_existing_slice(host, &ignored);
    if (status) return status;
    if (host->callback_0021DB94) return DREADZONE_CORE_STARTUP_NEEDS_CALLBACK;
#else
extern u32 D_00172160[];
extern s32 D_0021DE10[];
extern u8 D_0021DFA4;
void func_0015B498(void);
s32 func_01E9E918(s32, s32);
s32 func_01E9E3E0(void *);
void *func_0011AFC8(void *, const void *, u32);
s32 func_01E9EB08(void *, void *);
void func_0015A028(void);
void func_0015A050(void);
s32 func_01E9E798(void);
void func_0015DA28(void);
s32 func_01E9E0C0(s32, s32, s32);
extern u8 func_0015B2C8[];
extern u8 func_0015C2D8[];
extern u8 func_0015C6B0[];
#define WORD(offset) (D_00172160[(offset) / 4])
#define BYTE(offset) (((u8 *)D_00172160)[offset])
#define LEVEL D_0021DE10[0]
#define FLAG D_0021DFA4
s32 func_0015AA58(void) {
    u32 request[0x50 / 4], result[4];
    s32 i, status, a, b;
    if ((s32)WORD(0) >= 10) return 0;
#endif
    BYTE(0x7F8) = 0;
    BYTE(0x6C8) = 0;
    for (i = 0; i < 10; i++) {
        WORD(0x6A0 + 4 * i) = 0;
        WORD(0x7CC + 4 * i) = 0;
    }
    FLAG = 0;
    WORD(0x7F4) = 0;
    if (WORD(0x258) == 1) {
        WORD(0) = 10;
        if (LEVEL == 0) BYTE(0x265) = 1;
        WORD(0x10) = 0;
#ifdef DEADLOCKED_NATIVE
        return dreadzone_core_0015B498_slice(host);
#else
        func_0015B498();
        return 0;
#endif
    }
#ifndef DEADLOCKED_NATIVE
    func_01E9E918((s32)WORD(8), 0);
    WORD(0) = 9;
    if (func_01E9E3E0(request)) return 1;
    request[1] = WORD(8);
    func_0011AFC8((u8 *)request + 8, (u8 *)D_00172160 + 0xFE, 12);
    request[0x1C / 4] = 2;
    request[0x14 / 4] = 3;
    request[0x18 / 4] = 0;
    request[0x20 / 4] = (u32)func_0015B2C8;
    request[0x28 / 4] = (u32)func_0015C2D8;
    request[0x30 / 4] = (u32)func_0015C6B0;
    if (func_01E9EB08(request, result)) return 1;
    func_0015A028();
    do {
        func_0015A050();
        status = func_01E9E798();
        if (status != 0 && status != 12) { func_0015DA28(); return 1; }
    } while ((s32)WORD(0) == 9);
    if ((s32)WORD(0) != 10) return 1;
    func_0015B498();
    if (BYTE(0x262)) { a = 0; b = 0; }
    else if (BYTE(0x260)) { a = 90; b = 90; }
    else { a = 0; b = 30; }
    return func_01E9E0C0((s32)WORD(8), a, b) != 0;
#else
    return DREADZONE_CORE_STARTUP_NEEDS_NETWORK;
#endif
}
