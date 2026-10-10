/* cflags: -mno-split-addresses */
#include "common.h"
/* Actual L01 button repeat pass: 00614EE8..00614FD8, 240 bytes.
 * This is not an actor update pass or a controller transport implementation.
 */
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_input_repeat.h"
#include <string.h>
static u32 repeat_word(const u8 *p) { u32 v; memcpy(&v,p,4); return v; }
#else
typedef struct RepeatActor31F0 { u8 bytes[0x31F0]; } RepeatActor31F0;
typedef struct InputRecord5C0 { u8 bytes[0x1C0]; u32 f1C0, f1C4; } InputRecord5C0;
extern s32 *D_0021DDA4 __attribute__((section(".data")));
extern u32 D_0021F580;
extern s32 D_0021F588[4];
extern u32 D_0021F598[4];
RepeatActor31F0 *func_L01_006658C0(s32);
#endif

static inline void repeat_apply(InputRecord5C0 *input, u32 *previous, s32 *countdown, u32 mask) {
    if (*previous != (u32)input->f1C0 || *previous == 0) {
        *countdown = 20;
    } else {
        /* MIPS addiu wraps at 32 bits; preserve this even at INT_MIN. */
        *countdown = (s32)((u32)*countdown - 1U);
        if (*countdown <= 0) {
            *countdown = 5;
            input->f1C4 = input->f1C0 & mask;
        }
    }
    *previous = (u32)input->f1C0;
}

#ifdef DEADLOCKED_NATIVE
int dreadzone_input_repeat(DreadzoneRepeatContext *host) {
    InputRecord5C0 *resolved[4] = {NULL,NULL,NULL,NULL};
    u32 i,j,address;
    DreadzoneActorInit31F0 *actor;
    if (!host) return DREADZONE_REPEAT_INVALID;
    if (host->player_count <= 0) return DREADZONE_REPEAT_COMPLETE;
    if (host->player_count > 4 || (u32)host->player_count > host->actor_count ||
        !host->actors || host->input_count > 8 || (host->input_count && !host->inputs))
        return DREADZONE_REPEAT_INVALID;
    /* Resolve the full operation before mutating any input or countdown. */
    for (i=0; i<(u32)host->player_count; i++) {
        actor = dreadzone_repeat_actor_at(host,i);
        if (!actor) return DREADZONE_REPEAT_INVALID;
        address = repeat_word((const u8 *)actor + 0x2F00);
        if (!address) continue;
        if (address > 0xFFFFFFFFU - (sizeof(*resolved[i]) - 1)) return DREADZONE_REPEAT_INVALID;
        for (j=0; j<host->input_count; j++) {
            if (host->inputs[j].guest_address == address) {
                if (resolved[i] || !host->inputs[j].storage) return DREADZONE_REPEAT_INVALID;
                resolved[i] = host->inputs[j].storage;
            }
        }
        if (!resolved[i]) return DREADZONE_REPEAT_NEEDS_INPUT;
    }
    for (i=0; i<(u32)host->player_count; i++) {
        if (resolved[i]) repeat_apply(resolved[i],&host->previous_0021F598[i],
            &host->countdown_0021F588[i],host->mask_0021F580);
    }
    return DREADZONE_REPEAT_COMPLETE;
}
#else
void func_L01_00614EE8(void) {
    s32 i;
    InputRecord5C0 *input;
    for (i=0; i<*D_0021DDA4; i++) {
        input = *(InputRecord5C0 **)(func_L01_006658C0(i)->bytes + 0x2F00);
        if (input) repeat_apply(input,&D_0021F598[i],&D_0021F588[i],D_0021F580);
    }
}
#endif
