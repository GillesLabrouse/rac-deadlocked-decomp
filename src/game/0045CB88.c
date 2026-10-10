#include "common.h"

typedef struct StartupRow {
    s32 value;
    u8 unused[12];
} StartupRow;

typedef struct StartupSource {
    u8 unused[0x18];
    StartupRow rows[9];
    u8 fA8;
    u8 gapA9[11];
    u8 fB4;
} StartupSource;

typedef struct StartupSlots {
    s32 rows[9][8];
    u8 f120[8];
    u8 f128[8];
    s32 f130;
    s32 f134;
    s32 f138;
    s32 f13C;
} StartupSlots;

extern StartupSource D_00171D38_[];
extern StartupSlots D_001DFAB0_[];

/* Copy startup values into eight slots and clear the four state markers. */
void func_0045CB88(void) {
    s32 first = D_00171D38_[0].rows[0].value;
    s32 second = D_00171D38_[0].rows[1].value;
    s32 third = D_00171D38_[0].rows[2].value;
    s32 fourth = D_00171D38_[0].rows[3].value;
    s32 fifth = D_00171D38_[0].rows[4].value;
    s32 sixth = D_00171D38_[0].rows[5].value;
    s32 seventh = D_00171D38_[0].rows[6].value;
    s32 eighth = D_00171D38_[0].rows[7].value;
    s32 ninth = D_00171D38_[0].rows[8].value;
    u8 other = D_00171D38_[0].fA8;
    u8 flag = D_00171D38_[0].fB4;
    s32 index;
    for (index = 0; index < 8; index++) {
        D_001DFAB0_[0].rows[0][index] = first;
        D_001DFAB0_[0].rows[1][index] = second;
        D_001DFAB0_[0].rows[2][index] = third;
        D_001DFAB0_[0].rows[3][index] = fourth;
        D_001DFAB0_[0].rows[4][index] = fifth;
        D_001DFAB0_[0].rows[5][index] = sixth;
        D_001DFAB0_[0].rows[6][index] = seventh;
        D_001DFAB0_[0].rows[7][index] = eighth;
        D_001DFAB0_[0].rows[8][index] = ninth;
        D_001DFAB0_[0].f120[index] = other;
        D_001DFAB0_[0].f128[index] = flag;
    }
    D_001DFAB0_[0].f130 = -1;
    D_001DFAB0_[0].f134 = -1;
    D_001DFAB0_[0].f138 = -1;
    D_001DFAB0_[0].f13C = -1;
}
