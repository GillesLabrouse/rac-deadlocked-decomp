/* cflags: -mno-split-addresses -ffast-math */
/* asflags: --float-immediates */
/* Dreadzone packet decoder at 0x005BDA40, retail extent 2184 bytes.
 * NONMATCHING: scalar C replaces the observed VU length calculation; register
 * allocation and scheduling have not matched. The two actual L01 math callees
 * remain dependencies. No controller driver or playable host frame is supplied.
 * Indices/counts must already have their observed ring-buffer domains.
 */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include <math.h>
#include "../native/dreadzone_input.h"
#else

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
#endif

#ifdef DEADLOCKED_NATIVE
#define INPUT_OFFSET(field) offsetof(InputRecord5C0, field)
#else
#define INPUT_OFFSET(field) ((u32)&(((InputRecord5C0 *)0)->field))
#endif
typedef char InputSizeCheck[sizeof(InputRecord5C0) == 0x5C0 ? 1 : -1];
typedef char InputAxesCheck[INPUT_OFFSET(f100) == 0x100 ? 1 : -1];
typedef char InputHistoryCheck[INPUT_OFFSET(f18E) == 0x18E ? 1 : -1];
typedef char InputButtonsCheck[INPUT_OFFSET(f1A0) == 0x1A0 ? 1 : -1];
typedef char InputAngleCheck[INPUT_OFFSET(f258) == 0x258 ? 1 : -1];
typedef char InputMagnitudeCheck[INPUT_OFFSET(f2D0) == 0x2D0 ? 1 : -1];
typedef char InputSampleCheck[INPUT_OFFSET(f354) == 0x354 ? 1 : -1];
typedef char InputChannelCheck[INPUT_OFFSET(f38C) == 0x38C ? 1 : -1];
typedef char InputDelayCheck[INPUT_OFFSET(f568) == 0x568 ? 1 : -1];

extern u8 D_0021DE31[];
extern s32 D_0021DDB4_[];
#ifdef DEADLOCKED_NATIVE
/* Absolute and GP accesses refer to the same retail word. */
#define D_0021DDB4 D_0021DDB4_[0]
#else
extern s32 D_0021DDB4;
#endif
extern s32 D_0021E8A0[];
extern f32 func_L01_0055D708(f32, f32);
extern f32 func_L01_0055D910(f32, f32);

void func_L01_005BDA40(InputRecord5C0 *record, const u8 *packet, s32 length) {
    s32 i, position, count, previous;
    f32 distance, angle, channel;

    record->f354[record->f34C] = ((packet[2] << 8) | packet[3]) ^ 0xFFFF;
    record->f370[record->f34C] = record->f354[record->f34C];
    for (i = 0; i < 16; i++) record->f38C[record->f34C][i] = 0.0f;
    if (length >= 8) {
        for (i = 0; i < 4; i++) {
            position = packet[4 + i] - 127;
            count = position < 0 ? -position : position;
            if (count >= 48) {
                channel = (f32)(count - 48) / 76.0f;
                channel = channel > 1.0f ? 1.0f : channel;
                record->f38C[record->f34C][i] = channel;
                if (packet[4 + i] < 127)
                    record->f38C[record->f34C][i] = -record->f38C[record->f34C][i];
            }
        }
    }
    if (length >= 20) {
        for (i = 4; i < 16; i++)
            record->f38C[record->f34C][i] = (f32)packet[4 + i] * 0.0039215688593685626983642578125f;
    }
    if (D_0021DE31[0] != 0) {
        record->f38C[record->f34C][2] = -record->f38C[record->f34C][2];
        record->f38C[record->f34C][0] = -record->f38C[record->f34C][0];
    }
    if (D_0021DDB4_[0] == 7) {
        if (record->f38C[record->f34C][2] < 0.0f) record->f354[record->f34C] |= 0x8000;
        if (record->f38C[record->f34C][2] > 0.0f) record->f354[record->f34C] |= 0x2000;
        if (record->f38C[record->f34C][3] < 0.0f) record->f354[record->f34C] |= 0x1000;
        if (record->f38C[record->f34C][3] > 0.0f) record->f354[record->f34C] |= 0x4000;
    }
    if (record->f350 < record->f568) {
        record->f1A0 = record->f1AC;
        record->f1B0 = record->f1BC;
    } else {
        position = (record->f34C - record->f568 + 7) % 7;
        record->f1A0 = record->f354[position];
        record->f1B0 = record->f370[position];
        for (i = 0; i < 16; i++) record->f100[i] = record->f38C[position][i];
    }
    count = record->f350 + 1;
    record->f350 = record->f568 < count ? record->f568 : count;
    record->f34C = (record->f34C + 1) % 7;
    for (i = 0; i < 16; i++) record->f140[i] = record->f100[i];
    record->f1D8 = record->f100[2] != 0.0f || record->f100[3] != 0.0f;
    if (record->f100[2] < 0.0f) record->f1A0 |= 0x8000;
    if (record->f100[2] > 0.0f) record->f1A0 |= 0x2000;
    if (record->f100[3] < 0.0f) record->f1A0 |= 0x1000;
    if (record->f100[3] > 0.0f) record->f1A0 |= 0x4000;
    previous = record->f1AC;
    record->f1D4 = (record->f1A0 & 0xF000) == 0;
    record->f1D0 = record->f1A0 == 0;
    record->f1B4 = ~previous & record->f1B0;
    record->f1B8 = ~record->f1A0 & record->f1BC;
    record->f1C4 = ~previous & record->f1A0;
    record->f1C8 = ~record->f1A0 & previous;
    record->f348 = record->f1A0;
    record->f1A4 = record->f1C4;
    record->f1A8 = record->f1C8;
    record->f1C0 = record->f1A0;
    if (D_0021DE31[0] != 0) {
        record->f140[2] = -record->f140[2];
        record->f140[0] = -record->f140[0];
    }
    if (record->f1CC == 1) {
        record->f1A0 &= ~0x5030;
        record->f1A4 &= ~0x5030;
        record->f1A8 &= ~0x5030;
        record->f100[1] = 0.0f;
        record->f100[0] = 0.0f;
        if (D_0021E8A0[0] != 0) record->f1CC = 0;
    }
    if (record->f1CC == 2) {
        record->f1A0 &= 0x900;
        record->f1A4 &= 0x900;
        record->f1A8 &= 0x900;
        record->f1B0 &= 0x900;
        record->f1D4 = 1;
        record->f100[2] = 0.0f;
        record->f100[3] = 0.0f;
        if (D_0021E8A0[0] != 0) record->f1CC = 0;
    }
    /* Recovered VU arithmetic: x=field108, y=field10C, sqrt(x*x+y*y).
     * Scalar C does not reproduce VU rounding or flags. The angle callee takes
     * these two floats, not the stale integer registers inferred by m2c.
     */
#ifdef DEADLOCKED_NATIVE
    distance = sqrtf(record->f100[2] * record->f100[2] +
                     record->f100[3] * record->f100[3]);
#else
    distance = __builtin_sqrtf(record->f100[2] * record->f100[2] +
                               record->f100[3] * record->f100[3]);
#endif
    angle = func_L01_0055D708(record->f100[2], record->f100[3]);
    record->f2D0[record->f18E] = distance;
    record->f258[record->f18E] = angle;
    if (distance > 0.9f) {
        position = record->f18E + 29;
        if (!(record->f2D0[position % 30] > 0.9f)) {
            count = 1;
            do {
                count++;
                if (record->f2D0[position % 30] < 0.25f) {
                    record->f1A4 |= 0x10000;
                    break;
                }
                position--;
            } while (count < 4 && !(record->f2D0[position % 30] > 0.9f));
        }
        if (!(record->f1A4 & 0x10000)) {
            for (i = 1; i < 5; i++) {
                position = (record->f18E - i + 30) % 30;
                if (func_L01_0055D910(record->f258[position], angle) > 0.959931075572967529296875f) {
                    record->f1A4 |= 0x10000;
                    break;
                }
            }
        }
    }
    if (D_0021DDB4 != -1) {
        record->f1E0[record->f18E] = record->f1A4;
        count = record->f190 + 1;
        record->f190 = count < 31 ? count : 30;
        record->f18E = (s16)((record->f18E + 1) % 30);
    }
    if (record->f54E != 0) {
        if (record->f1A0 != 0 && record->f1A0 == record->f1AC) {
            record->f54F--;
            if (record->f54F == 0 || record->f54F == 255) {
                record->f1A4 = record->f1A0;
                record->f54F = record->f54E;
            }
        } else record->f54F = record->f54D;
    }
}
