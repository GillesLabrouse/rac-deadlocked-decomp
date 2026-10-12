#include "common.h"



extern s32 D_0021FB84;
s32 func_00465C10(s32, s32, void *, s32);
s32 func_0047F568(s32, s32, s32, s32, s32);
s32 func_004D3EF0();
extern s32 func_00469118();


void func_004D3AE0(void) {
    s32 temp_3;

    temp_3 = D_0021FB84;
    switch (temp_3) {
    case 0:
        func_00465C10(2, 0, &func_00469118, 0);
        /* fallthrough */
    case 1:
        func_0047F568(4, 1, 1, 0, 0);
        D_0021FB84 = (s32) (D_0021FB84 + 1);
        return;
    case 2:
        func_004D3EF0();
        break;
    }
}
