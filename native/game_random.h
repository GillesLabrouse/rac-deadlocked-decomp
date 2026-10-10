#ifndef DEADLOCKED_NATIVE_GAME_RANDOM_H
#define DEADLOCKED_NATIVE_GAME_RANDOM_H
#include "common.h"
typedef struct DeadlockedRandomState {
    u8 *storage;
    u32 bytes;
} DeadlockedRandomState;
enum { DEADLOCKED_RANDOM_COMPLETED=0, DEADLOCKED_RANDOM_INVALID=-1 };
/* Complete00163408 and0058FF38 algorithms with caller-owned mutable seed
 * corresponding to original0021DC80. Storage may be unaligned. All32 seed
 * bits and arbitrary signed bounds are supported, including wrapped widths
 * and the actual R5900 division-by-zero remainder. Output must be disjoint
 * from the four seed bytes and naturally four-byte aligned. Descriptors are
 * retained before either write.
 * Invalid numeric spans/null pointers are refused before mutation; valid
 * nonwrapping pointers must designate actual readable/writable allocations.
 * This advances the real RNG state; it does not construct a game actor. */
int deadlocked_random_00163408(const DeadlockedRandomState *,s32 *output);
int deadlocked_random_0058FF38(const DeadlockedRandomState *,s32 lower,s32 upper,s32 *output);
/* Shared preflight used only by these recovered entry points. */
int deadlocked_random_storage(const DeadlockedRandomState *,s32 *output,u8 **seed);
#endif
