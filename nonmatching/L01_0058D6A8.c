#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_pool.h"
#else
typedef struct DreadzonePayloadState {
    u32 head;
    u32 tail;
    u32 cursor;
} DreadzonePayloadState;
typedef char payload_state_size_check[sizeof(DreadzonePayloadState) == 12 ? 1 : -1];
#endif

/* L01 descriptor release, including the head/tail/empty exits through
 * 0058D77C. Valid retail inputs are allocation starts. Retail traps on a
 * missing block; this draft and bounded host interface report failure. */
static inline int payload_release_impl(u32 descriptorBase, u32 *words, u32 payloadBase,
                                DreadzonePayloadState *state, u32 address) {
    u32 block = (address - payloadBase) >> 6;
    u32 current = state->head;
    u32 previous = 0;
    u32 word, next;
    if (current == 0) return 0;
    for (;;) {
        word = words[(current - descriptorBase) >> 2];
        if (((word >> 8) & 0xFFFU) == block) break;
        next = word >> 20;
        if (next & 0x800U) return 0;
        previous = current;
        current = descriptorBase + next * 4U;
    }
    words[(current - descriptorBase) >> 2] = 0;
    if (current < state->cursor) state->cursor = current;
    if (state->head == state->tail) {
        state->head = 0;
        state->tail = 0;
        state->cursor = descriptorBase;
    } else if (state->head == current) {
        state->head = descriptorBase + (word >> 20) * 4U;
    } else {
        words[(previous - descriptorBase) >> 2] =
            (words[(previous - descriptorBase) >> 2] & 0xFFFFFU) | (word & 0xFFF00000U);
        if (state->tail == current) state->tail = previous;
    }
    return 1;
}

#ifndef DEADLOCKED_NATIVE
extern u32 D_00222D3C;
extern u32 D_00222D44;
extern u32 D_00222D48;
extern u32 D_00222D4C;
extern u32 D_00222D50;
void func_L01_0058D6A8(u8 *address) {
    DreadzonePayloadState state;
    state.head = D_00222D48;
    state.tail = D_00222D4C;
    state.cursor = D_00222D50;
    payload_release_impl(D_00222D44, (u32 *)D_00222D44, D_00222D3C, &state, (u32)address);
    D_00222D48 = state.head;
    D_00222D4C = state.tail;
    D_00222D50 = state.cursor;
}
#else
int dreadzone_payload_validate_arena(const DreadzonePayloadArena *,
                                    const DreadzonePayloadState *, u32);
int dreadzone_payload_release(const DreadzonePayloadArena *arena,
                             DreadzonePayloadState *state, u32 address) {
    if (!dreadzone_payload_validate_arena(arena, state, 1)
        || address < arena->payload_base || ((address - arena->payload_base) & 63U) != 0
        || dreadzone_payload_translate(arena, address, 1) == NULL) return 0;
    return payload_release_impl(arena->descriptor_base, arena->descriptors,
                                arena->payload_base, state, address);
}
#endif
