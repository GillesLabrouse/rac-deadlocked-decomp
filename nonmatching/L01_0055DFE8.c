/* Real Dreadzone pre-loop state initializer; dependencies live in L01_0055DCF8.c. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_state_init.h"
#else
extern s32 D_0021DE10 __attribute__((section(".data")));
extern u8 D_0021E868;
extern u32 D_0021DE98[3];
extern u32 D_0021DEA8[3];
extern u32 D_0021DEB8[3];
extern void func_L01_0055DCF8(s32 slot, u32 base, u32 table);
extern void func_L01_0055E0A0(s32 slot);
extern u8 D_001743C0_[];
extern u8 D_00173C00_[];
extern u8 D_0017CF80_[];
extern u8 D_0017C740_[];
extern u8 D_0021E1D0_[];
extern u8 D_0021E1D4_[];
#endif

void func_L01_0055DFE8(void) {
#ifdef DEADLOCKED_NATIVE
    func_L01_0055DCF8(0, 0x001743C0U, 0x00173C00U);
    func_L01_0055DCF8(1, 0x0017CF80U, 0x0017C740U);
#else
    func_L01_0055DCF8(0, (u32)D_001743C0_, (u32)D_00173C00_);
    func_L01_0055DCF8(1, (u32)D_0017CF80_, (u32)D_0017C740_);
#endif
    if (D_0021DE10 < 0 || D_0021E868 != 0) {
#ifdef DEADLOCKED_NATIVE
        func_L01_0055DCF8(2, 0x0021E1D0U, 0x0021E1D4U);
#else
        func_L01_0055DCF8(2, (u32)D_0021E1D0_, (u32)D_0021E1D4_);
#endif
    } else {
        D_0021DE98[2] = 0;
        D_0021DEA8[2] = 0;
        D_0021DEB8[2] = 0;
    }
    func_L01_0055E0A0(0);
}
