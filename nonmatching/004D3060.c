/* cflags: -mno-split-addresses */
#include "common.h"

/* Experimental transition orchestration. The current compile is 232 bytes,
 * the retail extent; five instruction words still differ at the conditional
 * access to D_003D9BE0. Return types of callees whose values are discarded
 * remain provisional. This file is excluded from matching coverage.
 */

typedef struct TransitionState80 {
    s32 f0;
    u8 gap4[0x4B];
    u8 f4F;
} TransitionState80;

extern s32 func_0047EF38(void);
extern s32 func_00158868(s32);
extern s32 func_0049B120(s32);
extern s32 func_00489458(s32);
extern s32 func_0048A060(void *, s32);
extern s32 func_00157F98(void);
extern s32 func_00505B20(void);
extern s32 func_00505AD8(s32);
extern s32 func_004D36E0(void);
extern s32 func_00501498(s32);
extern s32 func_00505C90(s32);
extern s32 func_004E21D8(u32, s32, s32);
extern s32 func_0048FA80(s32);
extern u8 D_002214D0[];
extern TransitionState80 D_003D9BE0;
extern u8 D_0021FB2C;
extern s32 D_0021DDCC __attribute__((section(".data")));

typedef char transition_state_flag_offset_check[
    (u32)&((TransitionState80 *)0)->f4F == 0x4F ? 1 : -1];

void func_004D3060(s32 request) {
    s32 previous;
    func_0047EF38();
    func_00158868(0xCD);
    if (func_0049B120(0) != 24) func_00489458(0);
    func_0048A060(D_002214D0, 1);
    func_00157F98();
    if (D_003D9BE0.f4F == 0) D_003D9BE0.f0 = request;
    previous = func_00505B20();
    func_00505AD8(4);
    func_00501498(func_004D36E0());
    func_00505C90(func_004D36E0());
    func_00505AD8(previous);
    if (D_0021FB2C != 0) {
        D_0021FB2C = 0;
        func_004E21D8(0xFFFFFFFFU, 0, 0);
    }
    if (D_0021DDCC != 0) func_0048FA80(D_0021DDCC);
}
