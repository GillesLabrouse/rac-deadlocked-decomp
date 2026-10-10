/* Dreadzone's real pre-loop clear, guest range 00222900..003A0B40.
 * The quad-zero helper is expressed in C; its retail implementation uses
 * handwritten R5900 stores. Neither routine is counted as matching yet.
 */
#include "common.h"

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_bulk_clear.h"
#include <stdlib.h>
#include <string.h>
#else
typedef unsigned int StartupClearQuad __attribute__((mode(TI)));
extern u8 D_00222900_[];
extern u8 D_003A0B40_[];
#endif

/* Retail writes the first quad before checking the count, and writes a second
 * quad before repeating. Counts used by the game here are positive/aligned.
 * The native store uses memcpy so host placement cannot create alignment UB.
 */
void func_L01_0055D2D0(u8 *destination, s32 bytes) {
#ifdef DEADLOCKED_NATIVE
    const u8 zero[16] = {0};
#define CLEAR_QUAD(pointer) memcpy((pointer), zero, 16)
#else
#define CLEAR_QUAD(pointer) (*(StartupClearQuad *)(pointer) = 0)
#endif
    for (;;) {
        CLEAR_QUAD(destination);
        bytes -= 16;
        destination += 16;
        if (bytes <= 0) {
            return;
        }
        CLEAR_QUAD(destination);
        bytes -= 16;
        destination += 16;
        if (bytes <= 0) {
            return;
        }
    }
#undef CLEAR_QUAD
}

#ifdef DEADLOCKED_NATIVE
static DreadzoneBulkArena bound_arena;
static int arena_is_bound;

u8 *dreadzone_bulk_translate(const DreadzoneBulkArena *arena, u32 address, u32 bytes) {
    u64 arena_end;
    u64 request_end;
    if (arena == NULL || arena->storage == NULL) {
        return NULL;
    }
    arena_end = (u64)arena->guest_base + arena->bytes;
    request_end = (u64)address + bytes;
    if (arena_end > 0x100000000ULL || address < arena->guest_base || request_end > arena_end) {
        return NULL;
    }
    return arena->storage + ((u64)address - arena->guest_base);
}

int dreadzone_bulk_bind(const DreadzoneBulkArena *arena) {
    arena_is_bound = 0;
    if (dreadzone_bulk_translate(arena, DREADZONE_BULK_START, DREADZONE_BULK_BYTES) == NULL) {
        return -1;
    }
    bound_arena = *arena;
    arena_is_bound = 1;
    return 0;
}

int dreadzone_bulk_clear_range(const DreadzoneBulkArena *arena, u32 start, u32 end) {
    u32 length;
    u32 footprint;
    u8 *destination;
    s32 index;
    int quad_path;
    if (end < start || (u64)end - start > 0x7FFFFFFFULL) {
        return -1;
    }
    length = end - start;
    quad_path = (length & 15U) == 0 && (start & 15U) == 0;
    /* An empty aligned range still invokes the unconditional first store.
     * Validate that actual footprint rather than assuming an empty no-op.
     */
    footprint = quad_path && length == 0 ? 16 : length;
    destination = dreadzone_bulk_translate(arena, start, footprint);
    if (destination == NULL) {
        return -1;
    }
    if (quad_path) {
        func_L01_0055D2D0(destination, (s32)length);
    } else {
        for (index = 0; index < (s32)length; index++) {
            destination[index] = 0;
        }
    }
    return 0;
}

void func_L01_00529280(void) {
    if (!arena_is_bound ||
        dreadzone_bulk_clear_range(&bound_arena, DREADZONE_BULK_START, DREADZONE_BULK_END) != 0) {
        abort();
    }
}
#else
void func_L01_00529280(void) {
    s32 index;
    if ((((u32)D_003A0B40_ - (u32)D_00222900_) & 15U) == 0 &&
        ((u32)D_00222900_ & 15U) == 0) {
        func_L01_0055D2D0(D_00222900_, (s32)((u32)D_003A0B40_ - (u32)D_00222900_));
    } else {
        for (index = 0; index < (s32)((u32)D_003A0B40_ - (u32)D_00222900_); index++) {
            D_00222900_[index] = 0;
        }
    }
}
#endif
