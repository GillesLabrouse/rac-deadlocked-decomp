#include "common.h"

s32 func_005004E0();
/* Two local functions share the canonical shape; this call targets 0x5004C8. */
s32 func_L15_005004C8();
extern s32 D_0023C170_[];
#define D_0023C170 (D_0023C170_[0])

void func_L15_00500430(void) {
    switch (D_0023C170) {
    case 0:
        func_L15_005004C8();
        return;
    case 1:
        func_005004E0();
        break;
    }
}
