/* Draft of Dreadzone's pre-loop initialization at 00665D58 (152 bytes).
 * This call chain has unresolved subsystem dependencies and is not a native
 * initialization implementation. Comparison status is recorded below after
 * compiling with the original toolchain; do not count this file as matching.
 * Default SN flags produce 148 bytes plus four bytes of retail zero padding.
 * All actual relocation fields resolve correctly; three instructions still use
 * v0 rather than retail's v1 for the comparison of field 20.
 */
#include "common.h"

typedef struct L01StartupBlock {
    u8 f0[0x1C];
    s32 f1C;
    s32 f20;
    u8 f24[0x204];
    s32 f228;
    u8 f22C[0x2C];
    s32 f258;
    s32 f25C;
    u8 f260[0x469];
    u8 f6C9;
} L01StartupBlock;

extern L01StartupBlock D_00172160;
extern u8 D_00171DF4_[];
extern void func_0015A170(s32);
extern void func_0015AA58(void);
extern void func_L01_006C1C00(void);
extern void func_L01_006669D8(void);
extern void func_L01_00666A28(s32, s32);
extern void func_L01_006AF588(void);
extern void func_L01_006A9640(void);
extern void func_L01_006B41D0(void);

#define L01_FIELD_OFFSET(type, field) ((u32)&((type *)0)->field)
typedef char check_startup1C[L01_FIELD_OFFSET(L01StartupBlock, f1C) == 0x1C ? 1 : -1];
typedef char check_startup20[L01_FIELD_OFFSET(L01StartupBlock, f20) == 0x20 ? 1 : -1];
typedef char check_startup228[L01_FIELD_OFFSET(L01StartupBlock, f228) == 0x228 ? 1 : -1];
typedef char check_startup258[L01_FIELD_OFFSET(L01StartupBlock, f258) == 0x258 ? 1 : -1];
typedef char check_startup25C[L01_FIELD_OFFSET(L01StartupBlock, f25C) == 0x25C ? 1 : -1];
typedef char check_startup6C9[L01_FIELD_OFFSET(L01StartupBlock, f6C9) == 0x6C9 ? 1 : -1];
#undef L01_FIELD_OFFSET

void func_L01_00665D58(void) {
    D_00172160.f258 = 1;
    D_00172160.f25C = 1;
    D_00172160.f1C = 1;
    D_00172160.f228 = 0;
    D_00172160.f6C9 = 0;
    func_0015A170(0);
    func_0015AA58();
    if (D_00172160.f20 >= 2) {
        D_00171DF4_[0] = 0;
    }
    func_L01_006C1C00();
    func_L01_006669D8();
    func_L01_00666A28(0, 0);
    func_L01_006AF588();
    func_L01_006A9640();
    func_L01_006B41D0();
}
