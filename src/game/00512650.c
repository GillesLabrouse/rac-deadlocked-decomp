#include "common.h"

s32 func_00512610(u8 *);
u8 *func_00548D28(s32, s32);
extern s32 D_0021EDD4 __attribute__((section(".sdata")));

s32 func_00512650(u8 *player, s32 option) {
    s32 weapon;
    if (option != 0 && player[0x2658] != 0 && player[0x2660] == 0) {
        weapon = -1;
        if (*(s32 *)(player + 0x2314) == 2) weapon = *(s32 *)(player + 0x2318);
        if (weapon == -1) return 0;
        return *(s16 *)(func_00548D28(weapon, 0) + 0x30);
    }
    if (func_00512610(player) != 0 && D_0021EDD4 != 1) return 101;
    weapon = *(s32 *)(player + 0x2318);
    if (weapon == 5) return 66;
    if (weapon == 2) return 64;
    if (weapon == 4 || weapon == 10) return 68;
    if ((u32)((u32)weapon - 6U) < 2U || weapon == 3 || weapon == 8 || weapon == 9 || weapon == 13) return 78;
    return 0;
}
