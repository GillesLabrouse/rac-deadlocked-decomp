#include "common.h"



extern s32 D_00220DD8;
s32 func_0011A518(void *, s32, s32);
extern void *D_3367B8_[];
#define D_3367B8 (D_3367B8_[0])


s32 func_0053FAC8(void) {
    u8 local_buffer[16];
    s32 temp_3;
    s32 var_5;
    void **var_6;
    void *temp_4;

    if (D_00220DD8 == 0) {
        func_0011A518(local_buffer, 0, 0xB);
        var_6 = &D_3367B8;
        var_5 = 9;
        do {
            temp_4 = *var_6;
            if (temp_4 != NULL) {
                temp_3 = (*(s32 *)((u8 *)(temp_4) + 12052));
                if ((temp_3 < 0xB) && (*(local_buffer + temp_3) == 0)) {
                    D_00220DD8 = (s32) (D_00220DD8 + 1);
                    *(local_buffer + (*(s32 *)((u8 *)(temp_4) + 12052))) = 1;
                }
            }
            var_5 -= 1;
            var_6 = (void **)((u8 *)var_6 + 4);
        } while (var_5 >= 0);
        if (D_00220DD8 < 2) {
            D_00220DD8 = 2;
        }
    }
    return D_00220DD8;
}
