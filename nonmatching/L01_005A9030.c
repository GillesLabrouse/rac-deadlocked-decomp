/* cflags: -mno-split-addresses -ffast-math */
/* asflags: --float-immediates */
/* Dreadzone pending-request consumer at 0x005A9030.
 * NONMATCHING: compiled 1460 bytes versus the retail 1416-byte extent.
 * Both 17-entry switches and all observed constant-address references are
 * positionally verified against the actual Dreadzone overlay. This evidence
 * does not establish callee signatures or semantic equivalence. This draft
 * does not implement its callees
 * or provide a playable host runtime. Tests may capture those boundaries.
 * The transition progress is an internal finite game value; PS2 exceptional
 * float/min/conversion behavior is not established by a Windows C build.
 */
#include "common.h"

#ifdef DEADLOCKED_NATIVE
#define ABS_DATA
extern s32 D_0021DDB4_[], D_0021E814_[], D_0021E824_[];
#define D_0021DDB4 (D_0021DDB4_[0])
#define D_0021E814 (D_0021E814_[0])
#define D_0021E824 (D_0021E824_[0])
#else
#define ABS_DATA __attribute__((section(".data")))
extern s32 D_0021DDB4 ABS_DATA;
extern s32 D_0021E814 ABS_DATA;
extern s32 D_0021E824 ABS_DATA;
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
extern s32 D_0021E810 ABS_DATA;
extern s32 D_0021E818;
extern s32 D_0021E81C ABS_DATA;
extern f32 D_0021E85C ABS_DATA;
extern u8 D_0021E869;
extern s32 D_0021ED24 ABS_DATA;
extern s32 D_0021ED28 ABS_DATA;
extern s32 D_0021ED2C ABS_DATA;
extern u8 *D_0021ED30 ABS_DATA;
extern s32 D_0021F6E0 ABS_DATA;
extern f32 D_00222A50;
extern f32 D_00222A54 ABS_DATA;
extern s32 D_00223798 ABS_DATA;
extern TransitionState D_0023F600;
extern f32 D_0023FEF0 ABS_DATA;
extern s32 D_0024370C ABS_DATA;
extern s32 D_00269940 ABS_DATA;
extern s16 D_0020694C ABS_DATA;
extern s32 D_00171FC8[];
extern u8 D_001C2040[];

extern s32 func_00163840(s32, s32, s32, s32, void *, s32, s32);
extern void func_L01_0052A5E8(void);
extern void func_L01_0052A608(void);
extern void func_L01_0052C5D8(void);
extern void func_L01_0052C5E0(void);
extern void func_L01_00534F30(s32, s32, s32, f32, f32, f32, f32);
extern void func_L01_00546B38(void);
extern void func_L01_0054A7C8(s32, s32);
extern s32 func_L01_005654C8(void);
extern void func_L01_00565568(void);
extern void func_L01_005655E8(s32, s32);
extern s32 func_L01_0057FB68(void);
extern f32 func_L01_00594AE8(f32 *, f32, f32);
extern void func_L01_005A95C0(s32);
extern void func_L01_005B6128(s32);
extern void func_L01_005B68F8(s32);
extern void func_L01_005D76F0(s32, s32);
extern void func_L01_006149C0(s32);
extern void func_L01_00614D80(void);
extern void func_L01_006153F0(s32);
extern void func_L01_0061E6A0(s32, s32);
extern void func_L01_0061E730(void);
extern s32 func_L01_006281C8(s32);
extern void func_L01_0062A778(void);
extern void func_L01_0062FBC0(s32);
extern void func_L01_0062FD70(void);
extern void func_L01_00630A58(s32);
extern void func_L01_00631030(void);
extern void func_L01_00633248(void);
extern void func_L01_00637610(s32);
extern void func_L01_006376F8(s32);
extern void func_L01_006379B8(void);

void func_L01_005A9030(void) {
    s32 waiting, previous, index, value;
    f32 progress;
    TransitionRecord14C *record;

    if (D_0021E824 == 0) func_L01_00637610(0);
    previous = D_0021DDB4;
    D_0021E810 = previous;
    if (D_0021E814 == -2) return;
    if (D_0021ED24 > 0) {
        D_0021ED24--;
        return;
    }
    if (D_0021E824 == 0 && previous == 0) {
        func_L01_0057FB68();
        previous = D_0021DDB4;
    }
    switch (previous) {
    case 3:
        if (D_0021E814 != 4 || D_0021ED28 == 7) func_L01_00614D80();
        break;
    case 4: func_L01_00565568(); break;
    case 1:
        func_L01_0062FD70();
        goto reset_transition;
    case 2:
        if (D_0021E824 == 0) {
            func_L01_00631030();
reset_transition:
            D_0023FEF0 = 0.62f;
            func_L01_00534F30(0, 0, 3, 1.11f, 0.005f, 0.2f, 0.0f);
            func_L01_00546B38();
        }
        break;
    case 7: func_L01_0061E730(); break;
    case 8: func_L01_006379B8(); break;
    case 10: func_L01_0052C5D8(); break;
    case 9: func_L01_0052A608(); break;
    case 15:
        D_0021E818 = *D_0021DDA4 - 1;
        func_L01_0062A778();
        func_L01_005D76F0(D_0021DE10, D_00269940);
        break;
    }
    waiting = 0;
    func_L01_005A95C0(10);
    switch (D_0021E814) {
    case 3:
        if (D_0021E824 == 0) {
            previous = D_0021E810;
            if (previous != 4 || func_L01_005654C8() == 7 ||
                (D_0021E810 == previous && func_L01_005654C8() == 10) ||
                D_0021E869 != 0) func_L01_006149C0(D_0021ED28);
        }
        break;
    case 4: func_L01_005655E8(D_0021ED28, D_0021ED2C); break;
    case 1: func_L01_0062FBC0(D_0021ED2C); break;
    case 2:
        if ((u32)D_0021DDB4 - 1U < 2U) {
            func_L01_00630A58(D_0021ED28);
            break;
        }
        if (D_0023F600.f8E == 0) {
            D_0023F600.f8E = 1;
            if (D_0021E85C < D_00222A50) D_0021E85C = D_00222A50;
            func_L01_005B6128(0);
            func_L01_005B68F8(0);
            if (D_0021E85C < 0.25f) {
                index = func_L01_006281C8(D_0021ED28);
                /* Actual L01 register sums preserve these byte addresses:
                 * record: 0x001CFB30 + index * 0x14C;
                 * shared word: 0x001CFAFC; payload pointer: 0x001CFAF8.
                 * The callee's index domain remains a runtime precondition.
                 */
                record = (TransitionRecord14C *)(D_001C2040 + 0xDAF0 + index * 0x14C);
                if (record->f4 > 0) {
                    if (D_0020694C != 0) func_L01_005B68F8(1);
                    func_L01_00633248();
                    D_00223798 -= 0x1000;
                    func_00163840(0, D_0024370C + D_00223798,
                        record->f0 + *(s32 *)(D_001C2040 + 0xDABC),
                        1, D_001C2040 + 0xDAB8, 0, 0);
                }
            }
        }
        func_L01_00594AE8(&D_0021E85C, 1.1f, 0.050000004f);
        progress = D_0021E85C < 1.0f ? D_0021E85C : 1.0f;
        D_00222A50 = progress;
        D_00222A54 = progress;
        value = (s32)((1.0f - progress) * 1024.0f);
        for (index = 5; index >= 0; index--) D_00171FC8[index] = value;
        if (D_0021E85C >= 1.1f) {
            D_0021E85C = 1.0f;
            if (D_0021E818 == 0) func_L01_00630A58(D_0021ED28);
            else {
                D_0021E81C = 0;
                waiting = 1;
            }
        } else waiting = 1;
        break;
    case 7: func_L01_0061E6A0(D_0021ED28 == 0 ? D_0021F6E0 : D_0021ED28, D_0021ED2C); break;
    case 8: func_L01_006376F8(0); break;
    case 10: func_L01_0052C5E0(); break;
    case 16: func_L01_006153F0(D_0021ED28); break;
    case 9: func_L01_0052A5E8(); break;
    case 15: func_L01_0054A7C8(2, 0); break;
    }
    if (waiting == 0) {
        if (D_0021ED30 != NULL) *D_0021ED30 = 1;
        previous = D_0021E814;
        D_0021E814 = -2;
        D_0021DDB4 = previous;
        D_0021ED30 = NULL;
    }
}
