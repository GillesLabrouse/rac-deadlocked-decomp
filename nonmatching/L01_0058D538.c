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

/* Retail hand-written allocation span 0058D538..0058D6A8 includes the empty
 * and failure exits beyond the splitter's initial 0x140-byte extent. The
 * shared algorithm below preserves the descriptor packing and first-fit order.
 * Host validation rejects unrepresentable/corrupt arenas before mutation. */
static inline u32 payload_allocate_impl(u32 descriptorBase, u32 *words, u32 count,
                                u32 payloadBase, u32 capacity,
                                DreadzonePayloadState *state, u32 bytes) {
    u32 blocks = (bytes + 63U) >> 6;
    u32 cursor = state->cursor;
    u32 end = descriptorBase + count * 4U;
    u32 current, previous, previousEnd, word, start, next;
    if (cursor == end) return 0;
    if (state->head == 0) {
        words[0] = 0xFFF00000U | blocks;
        state->head = descriptorBase;
        state->tail = descriptorBase;
        state->cursor = descriptorBase + 4U;
        return payloadBase;
    }
    current = state->head;
    previous = 0;
    previousEnd = 0;
    for (;;) {
        word = words[(current - descriptorBase) >> 2];
        start = (word >> 8) & 0xFFFU;
        if (start >= previousEnd + blocks) {
            words[(cursor - descriptorBase) >> 2] = blocks | (previousEnd << 8)
                | (((current - descriptorBase) >> 2) << 20);
            if (previous != 0) {
                words[(previous - descriptorBase) >> 2] =
                    (words[(previous - descriptorBase) >> 2] & 0xFFFFFU)
                    | (((cursor - descriptorBase) >> 2) << 20);
            } else {
                state->head = cursor;
            }
            break;
        }
        previous = current;
        previousEnd = start + (word & 0xFFU);
        next = word >> 20;
        if (next & 0x800U) {
            if (capacity < previousEnd + blocks) return 0;
            words[(state->tail - descriptorBase) >> 2] = (word & 0xFFFFFU)
                | (((cursor - descriptorBase) >> 2) << 20);
            words[(cursor - descriptorBase) >> 2] = 0xFFF00000U
                | (previousEnd << 8) | blocks;
            state->tail = cursor;
            break;
        }
        current = descriptorBase + next * 4U;
    }
    cursor += 4U;
    while (cursor != end && words[(cursor - descriptorBase) >> 2] != 0) cursor += 4U;
    state->cursor = cursor;
    return payloadBase + previousEnd * 64U;
}

#ifndef DEADLOCKED_NATIVE
extern u32 D_00222D3C;
extern u32 D_00222D40;
extern u32 D_00222D44;
extern u32 D_00222D48;
extern u32 D_00222D4C;
extern u32 D_00222D50;
extern u32 D_00222D54;

u8 *func_L01_0058D538(u32 bytes) {
    DreadzonePayloadState state;
    u32 result;
    state.head = D_00222D48;
    state.tail = D_00222D4C;
    state.cursor = D_00222D50;
    result = payload_allocate_impl(D_00222D44, (u32 *)D_00222D44,
        (D_00222D54 - D_00222D44) >> 2, D_00222D3C, D_00222D40, &state, bytes);
    D_00222D48 = state.head;
    D_00222D4C = state.tail;
    D_00222D50 = state.cursor;
    return (u8 *)result;
}
#else
#include <stdint.h>
#include <string.h>

u8 *dreadzone_payload_translate(const DreadzonePayloadArena *arena, u32 address, u32 bytes) {
    u32 offset;
    if (arena == NULL || arena->payload == NULL || address < arena->payload_base
        || arena->payload_base > 0xFFFFFFFFU - arena->payload_bytes) return NULL;
    offset = address - arena->payload_base;
    if (offset > arena->payload_bytes || bytes > arena->payload_bytes - offset) return NULL;
    return arena->payload + offset;
}

int dreadzone_payload_validate_arena(const DreadzonePayloadArena *arena,
                               const DreadzonePayloadState *state, u32 bytes) {
    u32 end, address, previousEnd, steps, word, start, blocks, next;
    if (arena == NULL || state == NULL || arena->descriptors == NULL || arena->payload == NULL)
        return 0;
    if (arena->descriptor_base == 0 || (arena->descriptor_base & 3U) != 0
        || ((uintptr_t)arena->descriptors & 3U) != 0
        || arena->descriptor_count == 0 || arena->descriptor_count > 2048U
        || arena->descriptor_base > 0xFFFFFFFFU - arena->descriptor_count * 4U
        || arena->payload_base == 0 || (arena->payload_base & 63U) != 0
        || (arena->payload_bytes & 63U) != 0 || arena->payload_bytes > 4096U * 64U
        || arena->payload_base > 0xFFFFFFFFU - arena->payload_bytes
        || bytes == 0 || bytes > 255U * 64U) return 0;
    end = arena->descriptor_base + arena->descriptor_count * 4U;
    if (state->cursor < arena->descriptor_base || state->cursor > end
        || ((state->cursor - arena->descriptor_base) & 3U) != 0) return 0;
    if (state->head == 0) {
        return state->tail == 0 && ((bytes + 63U) >> 6) <= arena->payload_bytes / 64U;
    }
    if (state->cursor != end && arena->descriptors[(state->cursor - arena->descriptor_base) >> 2] != 0)
        return 0;
    address = state->head;
    previousEnd = 0;
    for (steps = 0; steps < arena->descriptor_count; steps++) {
        if (address < arena->descriptor_base || address >= end
            || ((address - arena->descriptor_base) & 3U) != 0 || address == state->cursor) return 0;
        word = arena->descriptors[(address - arena->descriptor_base) >> 2];
        if (word == 0) return 0;
        start = (word >> 8) & 0xFFFU;
        blocks = word & 0xFFU;
        if (start < previousEnd || start + blocks > arena->payload_bytes / 64U) return 0;
        previousEnd = start + blocks;
        next = word >> 20;
        if (next & 0x800U) return address == state->tail;
        address = arena->descriptor_base + next * 4U;
    }
    return 0;
}

u32 dreadzone_payload_allocate(const DreadzonePayloadArena *arena,
                              DreadzonePayloadState *state, u32 bytes) {
    if (!dreadzone_payload_validate_arena(arena, state, bytes)) return 0;
    return payload_allocate_impl(arena->descriptor_base, arena->descriptors,
        arena->descriptor_count, arena->payload_base, arena->payload_bytes / 64U, state, bytes);
}

u32 dreadzone_payload_allocate_zeroed(const DreadzonePayloadArena *arena,
                                     DreadzonePayloadState *state, u32 bytes) {
    u32 result = dreadzone_payload_allocate(arena, state, bytes);
    u32 offset;
    const u32 zero = 0;
    if (result != 0) {
        u8 *destination = dreadzone_payload_translate(arena, result, (bytes + 3U) & ~3U);
        /* 0055D2A8 writes complete words. memcpy preserves its four-byte
         * footprint without requiring host backing to be word-aligned. */
        for (offset = 0; offset < bytes; offset += 4U) memcpy(destination + offset, &zero, 4);
    }
    return result;
}
#endif
