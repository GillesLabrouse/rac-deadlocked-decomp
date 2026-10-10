#ifndef DEADLOCKED_DREADZONE_INPUT_H
#define DEADLOCKED_DREADZONE_INPUT_H

#include "common.h"

/* Input storage remains the observed guest layout, including padding. It is
 * separate from the actor record; actors refer to it through offset 2F00.
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

extern u8 D_0021DE31[];
extern s32 D_0021DDB4_[];
extern s32 D_0021E8A0[];

/* Recovered scalar arithmetic for normal finite controller channels and zero,
 * bounded to [-1,1]. Subnormal and exceptional float behavior is unvalidated.
 * These
 * are actual helpers, not atan2 substitutions or captured call responses.
 * VU rounding/accumulator flags are not reproduced by the host float path.
 */
f32 func_L01_0055D708(f32 x, f32 y);
f32 func_L01_0055D910(f32 previous, f32 current);
void func_L01_005BDA40(InputRecord5C0 *record, const u8 *packet, s32 length);

#endif
