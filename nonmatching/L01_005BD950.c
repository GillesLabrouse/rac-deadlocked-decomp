/* cflags: -mno-split-addresses */
/* Independently recovered L01 input reset and two small producers.
 * NONMATCHING: no binary correspondence is claimed. The selective reset is
 * NOT a whole-record initializer: history, indices, delay, repeat options,
 * mode, previous snapshots, callback/context and padding remain intact.
 * Pool zero initialization belongs to core.bss, not to this function.
 * Ring index 34C and count 350 must remain in their observed domains 0..6.
 */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_input_reset.h"
#else
/* Independently verified PS2 layout. Keep the SN build's complete type source
 * here so its build provenance does not depend on native-only headers.
 */
typedef struct InputRecord5C0 {
    u8 pad0[0x100];
    f32 f100[16];
    f32 f140[16];
    u8 pad180[0xE];
    s16 f18E;
    s32 f190;
    u8 pad194[0xC];
    s32 f1A0, f1A4, f1A8, f1AC;
    s32 f1B0, f1B4, f1B8, f1BC;
    s32 f1C0, f1C4, f1C8, f1CC;
    s32 f1D0, f1D4, f1D8;
    u8 pad1DC[4];
    s32 f1E0[30];
    f32 f258[30];
    f32 f2D0[30];
    s32 f348, f34C, f350;
    s32 f354[7];
    s32 f370[7];
    f32 f38C[7][16];
    u8 f54C, f54D, f54E, f54F;
    u8 pad550[0x18];
    s32 f568;
    u8 pad56C[0x54];
} InputRecord5C0;
extern InputRecord5C0 D_001EE600[8];
#endif

typedef char InputResetSizeCheck[sizeof(InputRecord5C0) == 0x5C0 ? 1 : -1];
#ifdef DEADLOCKED_NATIVE
#define RESET_OFFSET(field) offsetof(InputRecord5C0, field)
#else
#define RESET_OFFSET(field) ((u32)&((InputRecord5C0 *)0)->field)
#endif
typedef char InputReset100Check[RESET_OFFSET(f100) == 0x100 ? 1 : -1];
typedef char InputReset140Check[RESET_OFFSET(f140) == 0x140 ? 1 : -1];
typedef char InputReset18ECheck[RESET_OFFSET(f18E) == 0x18E ? 1 : -1];
typedef char InputReset190Check[RESET_OFFSET(f190) == 0x190 ? 1 : -1];
typedef char InputReset1A0Check[RESET_OFFSET(f1A0) == 0x1A0 ? 1 : -1];
typedef char InputReset34CCheck[RESET_OFFSET(f34C) == 0x34C ? 1 : -1];
typedef char InputReset354Check[RESET_OFFSET(f354) == 0x354 ? 1 : -1];
typedef char InputReset370Check[RESET_OFFSET(f370) == 0x370 ? 1 : -1];
typedef char InputReset38CCheck[RESET_OFFSET(f38C) == 0x38C ? 1 : -1];
typedef char InputReset568Check[RESET_OFFSET(f568) == 0x568 ? 1 : -1];
#undef RESET_OFFSET

void func_L01_005BD950(InputRecord5C0 *record) {
    s32 sample, position, channel;
    for (sample = 0; sample < record->f350; sample++) {
        position = (record->f34C - sample + 7) % 7;
        record->f354[position] = 0;
        record->f370[position] = 0;
        for (channel = 15; channel >= 0; channel--)
            record->f38C[position][channel] = 0.0f;
    }
    record->f1B0 = 0;
    record->f1D4 = 1;
    record->f1A0 = 0;
    record->f1A4 = 0;
    record->f1A8 = 0;
    record->f1D0 = 1;
    record->f1B4 = 0;
    record->f1B8 = 0;
    record->f1C0 = 0;
    record->f1C4 = 0;
    record->f1C8 = 0;
    record->f1D8 = 0;
    for (channel = 0; channel < 16; channel++) {
        record->f100[channel] = 0.0f;
        record->f140[channel] = 0.0f;
    }
}

/* Actual history reset: null selects the first record in the core pool. */
void func_L01_005BD710(InputRecord5C0 *record) {
    if (record == 0) record = D_001EE600;
    record->f190 = 0;
    record->f18E = 0;
}

/* Actual delay setter at 005BD790, including return branch 005BD7AC.
 * Delay decreases preserve pending samples and indices. The caller supplies
 * 0..6; retail does not clamp it. Normal-domain subtraction cannot overflow.
 */
void func_L01_005BD790(InputRecord5C0 *record, s32 delay) {
    s32 increase = delay - record->f568;
    record->f568 = delay;
    if (increase > 0) {
        record->f350 = 0;
        record->f34C = 0;
    }
}
