/* cflags: -mno-split-addresses */
#include "common.h"

typedef struct BootParameters {
    u64 flag0 : 1;
    u64 flag1 : 1;
    u64 flag2 : 1;
    u64 value3 : 11;
    u64 value14 : 11;
    u64 value25 : 3;
    u64 reserved28 : 4;
    u16 value32;
    u16 value48;
} BootParameters;

typedef struct BootState {
    u8 unused0[8];
    u32 value8;
    u32 valueC;
    u32 value10;
    u8 unused14[0x9F];
    u8 valueB3;
    u8 unusedB4[9];
    u8 valueBD;
} BootState;

extern u32 D_0021DE6C_[];
extern BootState D_00171D38;
extern u16 D_0021DAA8_[];
extern u16 D_0021DAAC_[];
extern u8 D_0021DC88[];
extern s32 func_0011D920(u8 *, u8 *, ...);
extern u8 *func_0011A9EC(u8 *, u8 *);

/* Draft of startup parameter formatting; preserve the original packed fields. */
void func_001579F0(u8 *output) {
    BootParameters parameters;
    u8 text[16];
    u32 i;
    parameters.flag1 = D_0021DE6C_[0];
    parameters.flag0 = D_00171D38.valueB3;
    parameters.flag2 = D_00171D38.value8;
    parameters.value3 = D_00171D38.valueC;
    parameters.value14 = D_00171D38.value10;
    parameters.value25 = D_00171D38.valueBD;
    parameters.value32 = D_0021DAA8_[0];
    parameters.value48 = D_0021DAAC_[0];
    for (i = 0; i < 8; i++) {
        func_0011D920(text, D_0021DC88, ((u8 *)&parameters)[i]);
        func_0011A9EC(output, text);
        output += 2;
    }
}
