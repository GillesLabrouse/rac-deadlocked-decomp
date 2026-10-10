#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ABI fixtures use the same compatible C tags as the recovered translation unit.
 * There are no pointers or platform handles in these serialized scalar layouts.
 */
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

_Static_assert(sizeof(StartupSource)==0xB8, "Startup source must preserve its game layout");
_Static_assert(offsetof(StartupSource, rows)==0x18, "Startup values offset changed");
_Static_assert(offsetof(StartupSource, fA8)==0xA8, "Startup byte offset changed");
_Static_assert(offsetof(StartupSource, fB4)==0xB4, "Startup byte offset changed");
_Static_assert(sizeof(StartupSlots)==0x140, "Startup slots must preserve their game layout");

StartupSource D_00171D38_[1];
StartupSlots D_001DFAB0_[1];
extern void func_0045CB88(void);

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr,"Startup check failed at line %d: %s\n",__LINE__,#condition); \
    return EXIT_FAILURE; } } while (0)

int main(void) {
    const s32 values[9]={INT32_MIN,INT32_MAX,-1,0,17,-29,1024,-65536,1234567};
    StartupSource saved;
    memset(D_00171D38_,0xC3,sizeof(D_00171D38_));
    memset(D_001DFAB0_,0x5A,sizeof(D_001DFAB0_));
    for (unsigned row=0;row<9;row++) D_00171D38_[0].rows[row].value=values[row];
    D_00171D38_[0].fA8=0xA5; D_00171D38_[0].fB4=0xE3;
    memcpy(&saved,D_00171D38_,sizeof(saved));
    func_0045CB88();
    for (unsigned slot=0;slot<8;slot++) {
        for (unsigned row=0;row<9;row++) CHECK(D_001DFAB0_[0].rows[row][slot]==values[row]);
        CHECK(D_001DFAB0_[0].f120[slot]==0xA5);
        CHECK(D_001DFAB0_[0].f128[slot]==0xE3);
    }
    CHECK(D_001DFAB0_[0].f130==-1 && D_001DFAB0_[0].f134==-1);
    CHECK(D_001DFAB0_[0].f138==-1 && D_001DFAB0_[0].f13C==-1);
    CHECK(memcmp(&saved,D_00171D38_,sizeof(saved))==0);
    puts("Recovered startup parameters initialize all eight native slots correctly.");
    return EXIT_SUCCESS;
}
