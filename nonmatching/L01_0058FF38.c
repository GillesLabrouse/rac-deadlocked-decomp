#include "common.h"
/* Complete0058FF38..0058FF7C,68 instruction bytes plus4 padding bytes.
 * Executes the real RNG dependency. Draft-only, outside matching coverage. */
#ifdef DEADLOCKED_NATIVE
#include "../native/game_random.h"
int deadlocked_random_0058FF38(const DeadlockedRandomState *view,s32 lower,s32 upper,s32 *output) {
    u8 *seed;
    DeadlockedRandomState retained;
    s32 random;
    u32 width,magnitude,remainder;
    if(deadlocked_random_storage(view,output,&seed)) return DEADLOCKED_RANDOM_INVALID;
    retained.storage=seed;retained.bytes=4;
    /* Real scalar SUBU/ADDIU width wraps to32 bits; DIV uses signed width.
     * Numerator is nonnegative15-bit, so its remainder uses divisor magnitude.
     * A zero divisor leaves the numerator in HI on the R5900. */
    width=(u32)upper-(u32)lower+1U;
    magnitude=(width&0x80000000U)?0U-width:width;
    (void)deadlocked_random_00163408(&retained,&random);
    remainder=magnitude?(u32)random%magnitude:(u32)random;
    *output=(s32)((u32)lower+remainder);
    return DEADLOCKED_RANDOM_COMPLETED;
}
#else
s32 func_00163408(void);
s32 func_L01_0058FF38(s32 lower,s32 upper) {
    s32 random=func_00163408();
    s32 width=(s32)((u32)upper-(u32)lower+1U);
    return (s32)((u32)(random%width)+(u32)lower);
}
#endif
