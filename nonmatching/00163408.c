#include "common.h"
/* Complete00163408..00163440,56 bytes. The following instruction is outside
 * the listed routine. Draft-only source, with no accepted matching claim. */
#ifdef DEADLOCKED_NATIVE
#include "../native/game_random.h"
int deadlocked_random_storage(const DeadlockedRandomState *view,s32 *output,u8 **seed) {
    uintptr_t a,b;
    if(!view || (uintptr_t)view>UINTPTR_MAX-(sizeof(*view)-1)) return -1;
    if(!view->storage || view->bytes<4 || !output || ((uintptr_t)output&3) ||
       (uintptr_t)view->storage>UINTPTR_MAX-(view->bytes-1) ||
       (uintptr_t)output>UINTPTR_MAX-(sizeof(*output)-1)) return -1;
    a=(uintptr_t)view->storage;b=(uintptr_t)output;
    if(a>=b?a-b<sizeof(*output):b-a<4) return -1;
    *seed=view->storage;
    return 0;
}
int deadlocked_random_00163408(const DeadlockedRandomState *view,s32 *output) {
    u8 *seed;
    u32 state;
    if(deadlocked_random_storage(view,output,&seed)) return DEADLOCKED_RANDOM_INVALID;
    state=(u32)seed[0]|((u32)seed[1]<<8)|((u32)seed[2]<<16)|((u32)seed[3]<<24);
    state=state*3359741U+2531011U;
    seed[0]=(u8)state;seed[1]=(u8)(state>>8);seed[2]=(u8)(state>>16);seed[3]=(u8)(state>>24);
    *output=(s32)((state>>16)&32767U);
    return DEADLOCKED_RANDOM_COMPLETED;
}
#else
extern u32 D_0021DC80;
s32 func_00163408(void) {
    u32 state=D_0021DC80*3359741U+2531011U;
    D_0021DC80=state;
    return (s32)((state>>16)&32767U);
}
#endif
