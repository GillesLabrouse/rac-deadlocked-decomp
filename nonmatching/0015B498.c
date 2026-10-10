/* cflags: -mno-split-addresses */
/* Actual core0015B498, retail172 bytes. NONMATCHING experimental recovery.
 * Native execution supports only its mode1/existing-state/null-callback path. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_core_startup.h"
#define WORD(offset) (host->block->words[(offset) / 4])
#define BYTE(offset) (host->block->bytes[offset])
int dreadzone_core_0015F4A0_existing_slice(const DreadzoneCoreStartupContext *host, s32 *result) {
    if (!host || !result) return DREADZONE_CORE_STARTUP_INVALID_STORAGE;
    if (!host->state_0021DFE8) return DREADZONE_CORE_STARTUP_NEEDS_NETWORK;
    *result = 1;
    return DREADZONE_CORE_STARTUP_COMPLETE;
}
int dreadzone_core_0015B498_slice(DreadzoneCoreStartupContext *host) {
    s32 ignored;
    int status;
    if (!host || !host->block) return DREADZONE_CORE_STARTUP_INVALID_STORAGE;
    if (WORD(0x258) != 1) return DREADZONE_CORE_STARTUP_NEEDS_NETWORK;
    status = dreadzone_core_0015F4A0_existing_slice(host, &ignored);
    if (status) return status;
    if (host->callback_0021DB94) return DREADZONE_CORE_STARTUP_NEEDS_CALLBACK;
    host->selected_002120F4 = (s32)WORD(0x10);
    WORD(0x254) = WORD(0x10);
    BYTE(0x250) = 1;
    WORD(0x220) = WORD(0x218);
    return DREADZONE_CORE_STARTUP_COMPLETE;
}
#else
extern u32 D_00172160[];
extern s32 D_002120F4[];
extern void (*D_0021DB94[])(void);
s32 func_0015F4A0(void);
s32 func_01EC9730(u32 *, s32);
#define WORD(offset) (D_00172160[(offset) / 4])
#define BYTE(offset) (((u8 *)D_00172160)[offset])
void func_0015B498(void) {
    void (*callback)(void);
    D_002120F4[0] = (s32)WORD(0x10);
    if (WORD(0x258) == 1) {
        BYTE(0x250) = 1;
        WORD(0x254) = WORD(0x10);
        func_0015F4A0();
    } else {
        BYTE(0x250) = 0;
        func_01EC9730(&WORD(0x254), (s32)WORD(0x8));
        if (WORD(0x10) == WORD(0x254)) {
            BYTE(0x250) = 1;
            func_0015F4A0();
        }
    }
    callback = D_0021DB94[0];
    WORD(0x220) = WORD(0x218);
    if (callback) callback();
}
#endif
