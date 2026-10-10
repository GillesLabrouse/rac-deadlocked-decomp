/* Actual core byte fill, including aligned quad/double and byte-tail paths.
 * Retail's handwritten R5900 vector construction is represented in C here.
 */
#include "common.h"

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_byte_fill.h"
#include <string.h>
static DreadzoneByteArena fill_arena;
static int fill_is_bound;
#else
typedef unsigned int ByteFillQuad __attribute__((mode(TI)));
typedef union ByteFillPattern {
    u64 words[2];
    ByteFillQuad quad;
} ByteFillPattern;
#endif

static __inline void *byte_fill_core(void *destination, s32 value, u32 count, u32 alignment) {
    u8 *cursor = (u8 *)destination;
    if (count >= 8 && alignment == 0) {
#ifdef DEADLOCKED_NATIVE
        u8 pattern[16];
        unsigned index;
        for (index = 0; index < 16; index++) {
            pattern[index] = (u8)value;
        }
#define FILL_QUAD(pointer) memcpy((pointer), pattern, 16)
#define FILL_DOUBLE(pointer) memcpy((pointer), pattern, 8)
#else
        ByteFillPattern pattern;
        u64 word = (u8)value;
        word |= word << 8;
        word |= word << 16;
        word |= word << 32;
        pattern.words[0] = word;
        pattern.words[1] = word;
#define FILL_QUAD(pointer) (*(ByteFillQuad *)(pointer) = pattern.quad)
#define FILL_DOUBLE(pointer) (*(u64 *)(pointer) = word)
#endif
        if (count >= 32) {
            do {
                FILL_QUAD(cursor);
                count -= 32;
                cursor += 16;
                FILL_QUAD(cursor);
                cursor += 16;
            } while (count >= 32);
        }
        while (count >= 8) {
            FILL_DOUBLE(cursor);
            count -= 8;
            cursor += 8;
        }
#undef FILL_DOUBLE
#undef FILL_QUAD
    }
    while (count != 0) {
        *cursor++ = (u8)value;
        count--;
    }
    return destination;
}

void *func_0011A518(void *destination, s32 value, u32 count) {
#ifdef DEADLOCKED_NATIVE
    return byte_fill_core(destination, value, count, (u32)((uintptr_t)destination & 15U));
#else
    return byte_fill_core(destination, value, count, (u32)destination & 15U);
#endif
}

#ifdef DEADLOCKED_NATIVE
int dreadzone_byte_fill_bind(const DreadzoneByteArena *arena) {
    fill_is_bound = 0;
    if (arena == NULL || arena->storage == NULL ||
        (u64)arena->guest_base + arena->bytes > 0x100000000ULL) {
        return -1;
    }
    fill_arena = *arena;
    fill_is_bound = 1;
    return 0;
}

int dreadzone_byte_fill_guest(u32 address, s32 value, u32 count) {
    u64 end = (u64)address + count;
    u64 arena_end = (u64)fill_arena.guest_base + fill_arena.bytes;
    u8 *destination;
    if (!fill_is_bound || address < fill_arena.guest_base ||
        end > arena_end || end > 0x100000000ULL) {
        return -1;
    }
    destination = fill_arena.storage + ((u64)address - fill_arena.guest_base);
    /* Branch on guest alignment while memcpy keeps any host placement safe. */
    byte_fill_core(destination, value, count, address & 15U);
    return 0;
}
#endif
