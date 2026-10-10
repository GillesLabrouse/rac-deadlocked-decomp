/* cflags: -mno-split-addresses */
#include "common.h"

/* Actual L01 005742C8..00574418, 336 bytes. Full reset has genuine subsystem
 * calls; the native entry below executes only the explicit table writes. */
#ifndef DEADLOCKED_NATIVE
void func_001637F0(void);
void func_L01_00633248(void);
void func_L01_00574430(void);
void func_L01_0055D2A8(void *,u32,s32);
void func_L01_005469A0(void);
void func_L01_00546B38(void);
void func_L01_0054CFA0(void);
void func_L01_00633360(void);
void func_L01_005BF960(void);
void func_L01_00633EF8(void);
void func_L01_005886D0(void);
void func_L01_00588A60(void);
extern u32 D_001DFD1C __attribute__((section(".data")));
extern u32 D_0021DA34,D_0021DA30;
extern u8 D_0026B590[],D_0026B2B0[],D_0026EFB0[],D_0026E830[];
extern u8 D_00353D40[],D_00353940[],D_0034CB20[],D_0034C520[];
extern u32 D_0028DA20[];
void func_L01_005742C8(void) {
    s32 i;
    func_001637F0(); func_L01_00633248(); func_L01_00574430();
    D_0021DA34=D_001DFD1C;
    D_0021DA30=D_001DFD1C;
    func_L01_0055D2A8(D_0026B590,0xFFFFFFFFU,0x2800);
    func_L01_0055D2A8(D_0026B2B0,0xFFFFFFFFU,0x1E0);
    func_L01_0055D2A8(D_0026EFB0,0xFFFFFFFFU,0xF00);
    func_L01_0055D2A8(D_0026E830,0,0xF0);
    func_L01_0055D2A8(D_00353D40,0xFFFFFFFFU,0x800);
    func_L01_0055D2A8(D_00353940,0,0x80);
    func_L01_0055D2A8(D_0034CB20,0xFFFFFFFFU,0x400);
    func_L01_0055D2A8(D_0034C520,0,0x40);
    func_L01_005469A0(); func_L01_00546B38(); func_L01_0054CFA0();
    func_L01_00633360(); func_L01_005BF960(); func_L01_00633EF8();
    func_L01_005886D0(); func_L01_00588A60();
    for (i=127;i>=0;i--) D_0028DA20[i]=0;
}
#endif
