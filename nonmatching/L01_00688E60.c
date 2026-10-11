#include "common.h"
#ifndef DEADLOCKED_NATIVE
/* Complete384-byte numeric predicate. a0 is not consumed by retail. */
s32 func_L01_00688E60(u8 *player,s32 clip) {
    (void)player;
    if(
        clip==0 || clip==6 || clip==7 || clip==5 || clip==8 ||
        clip==0xB || clip==0x13 || clip==0x14 || clip==0x17 || clip==0x18 ||
        clip==0x19 || clip==0x1C || clip==0x1D || clip==0x1E || clip==0x69 ||
        clip==0x6A || clip==0x6B || clip==0x6C || clip==0x20 || clip==0x6E ||
        clip==0x6F || clip==0x24 || clip==0x25 || clip==0x26 || clip==0x32 ||
        clip==0x36 || clip==0x39 || clip==0x47 || clip==0x49 || clip==0x4A ||
        clip==0x4C || clip==0x44 || clip==0x40 || clip==0x4E || clip==0x42 ||
        clip==0x56 || clip==0x57 || clip==0x65 || clip==0x5B || clip==0x5C ||
        clip==0x5D || clip==0x5E || clip==0x5F || clip==0x41 || clip==0x60 ||
        clip==0x61) return 1;
    return 0;

}
#endif
