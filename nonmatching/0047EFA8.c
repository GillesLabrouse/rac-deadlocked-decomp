/* cflags: -mno-split-addresses -ffast-math */
/* asflags: --float-immediates */
/* Pending-request consumer at 0x0047EFA8.
 * NONMATCHING: 1460 compiled bytes versus 1416 in retail; register allocation,
 * frame layout and scheduling still differ after relocation-aware comparison.
 * Both 17-entry switches are read from the resident retail image, not an
 * overlay's similarly located data. This draft does not implement its callees
 * or provide a playable host runtime. Tests may capture those boundaries.
 * The transition progress is an internal finite game value; PS2 exceptional
 * float/min/conversion behavior is not established by a Windows C build.
 */
#include "common.h"

#ifdef DEADLOCKED_NATIVE
#define ABS_DATA
extern s32 D_0021DDB4_[], D_0021EDC4_[], D_0021EDD4_[];
#define D_0021DDB4 (D_0021DDB4_[0])
#define D_0021EDC4 (D_0021EDC4_[0])
#define D_0021EDD4 (D_0021EDD4_[0])
#else
#define ABS_DATA __attribute__((section(".data")))
extern s32 D_0021DDB4 ABS_DATA;
extern s32 D_0021EDC4 ABS_DATA;
extern s32 D_0021EDD4 ABS_DATA;
#endif

typedef struct TransitionState {
    u8 pad0[0x8E];
    s16 f8E;
} TransitionState;

typedef struct TransitionRecord14C {
    s32 f0;
    s32 f4;
    u8 pad8[0x144];
} TransitionRecord14C;

typedef char TransitionStateOffsetCheck[
    sizeof(TransitionState) == 0x90 ? 1 : -1];
typedef char TransitionRecordStrideCheck[
    sizeof(TransitionRecord14C) == 0x14C ? 1 : -1];

extern s32 *D_0021DDA4 ABS_DATA;
extern s32 D_0021DE10 ABS_DATA;
extern s32 D_0021EDC0 ABS_DATA;
extern s32 D_0021EDC8;
extern s32 D_0021EDCC ABS_DATA;
extern f32 D_0021EE0C ABS_DATA;
extern u8 D_0021EE19;
extern s32 D_0021F2E4 ABS_DATA;
extern s32 D_0021F2E8 ABS_DATA;
extern s32 D_0021F2EC ABS_DATA;
extern u8 *D_0021F2F0 ABS_DATA;
extern s32 D_0021FC90 ABS_DATA;
extern f32 D_002211D0;
extern f32 D_002211D4 ABS_DATA;
extern s32 D_00221E28 ABS_DATA;
extern TransitionState D_00237000;
extern f32 D_002372F0 ABS_DATA;
extern s32 D_0023AB0C ABS_DATA;
extern s32 D_00241B00 ABS_DATA;
extern s16 D_0020694C ABS_DATA;
extern s32 D_00171FC8[];
extern u8 D_001C2040[];

extern s32 func_00163840(s32, s32, s32, s32, void *, s32, s32);
extern void func_00437DD0(void);
extern void func_00437DF0(void);
extern void func_00437E10(void);
extern void func_00437E18(void);
extern void func_0043A118(s32, s32, s32, f32, f32, f32, f32);
extern void func_00447BA8(void);
extern void func_004494E8(s32, s32);
extern s32 func_004598E0(void);
extern void func_00459978(void);
extern void func_004599F8(s32, s32);
extern s32 func_00465C08(void);
extern f32 func_004757A0(f32 *, f32, f32);
extern void func_0047F538(s32);
extern void func_00489458(s32);
extern void func_00489C28(s32);
extern void func_00496EA8(s32, s32);
extern void func_004D3060(s32);
extern void func_004D33C0(void);
extern void func_004D38F8(s32);
extern void func_004DCBA8(s32, s32);
extern void func_004DCC38(void);
extern s32 func_004E2AF8(s32);
extern void func_004E49D0(void);
extern void func_004E4DF0(s32);
extern void func_004E4FA0(void);
extern void func_004E5638(s32);
extern void func_004E5C10(void);
extern void func_004E7520(void);
extern void func_004EAAE0(s32);
extern void func_004EABC8(s32);
extern void func_004EAE88(void);

void func_0047EFA8(void) {
    s32 waiting, previous, index, value;
    f32 progress;
    TransitionRecord14C *record;

    if (D_0021EDD4 == 0) func_004EAAE0(0);
    previous = D_0021DDB4;
    D_0021EDC0 = previous;
    if (D_0021EDC4 == -2) return;
    if (D_0021F2E4 > 0) {
        D_0021F2E4--;
        return;
    }
    if (D_0021EDD4 == 0 && previous == 0) {
        func_00465C08();
        previous = D_0021DDB4;
    }
    switch (previous) {
    case 3:
        if (D_0021EDC4 != 4 || D_0021F2E8 == 7) func_004D33C0();
        break;
    case 4: func_00459978(); break;
    case 1:
        func_004E4FA0();
        goto reset_transition;
    case 2:
        if (D_0021EDD4 == 0) {
            func_004E5C10();
reset_transition:
            D_002372F0 = 0.62f;
            func_0043A118(0, 0, 3, 1.11f, 0.005f, 0.2f, 0.0f);
            func_00447BA8();
        }
        break;
    case 7: func_004DCC38(); break;
    case 8: func_004EAE88(); break;
    case 10: func_00437E10(); break;
    case 9: func_00437DF0(); break;
    case 15:
        D_0021EDC8 = *D_0021DDA4 - 1;
        func_004E49D0();
        func_00496EA8(D_0021DE10, D_00241B00);
        break;
    }
    waiting = 0;
    func_0047F538(10);
    switch (D_0021EDC4) {
    case 3:
        if (D_0021EDD4 == 0) {
            previous = D_0021EDC0;
            if (previous != 4 || func_004598E0() == 7 ||
                (D_0021EDC0 == previous && func_004598E0() == 10) ||
                D_0021EE19 != 0) func_004D3060(D_0021F2E8);
        }
        break;
    case 4: func_004599F8(D_0021F2E8, D_0021F2EC); break;
    case 1: func_004E4DF0(D_0021F2EC); break;
    case 2:
        if ((u32)D_0021DDB4 - 1U < 2U) {
            func_004E5638(D_0021F2E8);
            break;
        }
        if (D_00237000.f8E == 0) {
            D_00237000.f8E = 1;
            if (D_0021EE0C < D_002211D0) D_0021EE0C = D_002211D0;
            func_00489458(0);
            func_00489C28(0);
            if (D_0021EE0C < 0.25f) {
                index = func_004E2AF8(D_0021F2E8);
                record = (TransitionRecord14C *)(D_001C2040 + 0xDAF0 + index * 0x14C);
                if (record->f4 > 0) {
                    if (D_0020694C != 0) func_00489C28(1);
                    func_004E7520();
                    D_00221E28 -= 0x1000;
                    func_00163840(0, D_0023AB0C + D_00221E28,
                        record->f0 + *(s32 *)(D_001C2040 + 0xDABC),
                        1, D_001C2040 + 0xDAB8, 0, 0);
                }
            }
        }
        func_004757A0(&D_0021EE0C, 1.1f, 0.050000004f);
        progress = D_0021EE0C < 1.0f ? D_0021EE0C : 1.0f;
        D_002211D0 = progress;
        D_002211D4 = progress;
        value = (s32)((1.0f - progress) * 1024.0f);
        for (index = 5; index >= 0; index--) D_00171FC8[index] = value;
        if (D_0021EE0C >= 1.1f) {
            D_0021EE0C = 1.0f;
            if (D_0021EDC8 == 0) func_004E5638(D_0021F2E8);
            else {
                D_0021EDCC = 0;
                waiting = 1;
            }
        } else waiting = 1;
        break;
    case 7: func_004DCBA8(D_0021F2E8 == 0 ? D_0021FC90 : D_0021F2E8, D_0021F2EC); break;
    case 8: func_004EABC8(0); break;
    case 10: func_00437E18(); break;
    case 16: func_004D38F8(D_0021F2E8); break;
    case 9: func_00437DD0(); break;
    case 15: func_004494E8(2, 0); break;
    }
    if (waiting == 0) {
        if (D_0021F2F0 != NULL) *D_0021F2F0 = 1;
        previous = D_0021EDC4;
        D_0021EDC4 = -2;
        D_0021DDB4 = previous;
        D_0021F2F0 = NULL;
    }
}
