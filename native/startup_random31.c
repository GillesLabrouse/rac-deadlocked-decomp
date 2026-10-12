#include "game_random.h"

int deadlocked_random_001633D0(const DeadlockedRandomState *view,s32 *output)
{
    u8 *seed;
    u32 state;
    if (deadlocked_random_storage(view,output,&seed)) return DEADLOCKED_RANDOM_INVALID;
    state=(u32)seed[0]|((u32)seed[1]<<8)|((u32)seed[2]<<16)|((u32)seed[3]<<24);
    state=state*3359741U+2531011U;
    seed[0]=(u8)state;seed[1]=(u8)(state>>8);seed[2]=(u8)(state>>16);seed[3]=(u8)(state>>24);
    *output=(s32)(state&0x7FFFFFFFU);
    return DEADLOCKED_RANDOM_COMPLETED;
}
