#include "dreadzone_byte_fill.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u32 D_002437C4;
s32 D_0021DE10;
static unsigned checks;

_Static_assert(sizeof(D_002437C4) == 4, "The actual buffer pointer must remain guest32");
_Static_assert(DREADZONE_BYTE_FILL_BYTES == 262144, "Retail pre-loop fill count changed");

static void check(int condition, const char *message) {
    checks++;
    if (!condition) {
        fprintf(stderr, "Byte fill check failed: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

static void check_span(const u8 *storage, size_t size, size_t start, size_t count, u8 value) {
    size_t index;
    for (index = 0; index < size; index++) {
        u8 expected = index >= start && index - start < count ? value : 0xA5;
        check(storage[index] == expected, "every requested byte and unchanged boundary byte must match");
    }
}

static void test_core_alignment_and_counts(void) {
    u8 *allocation = (u8 *)malloc(544);
    u8 *aligned;
    unsigned alignment, count_index, value_index;
    const u32 extra_counts[] = {127, 128, 129, 255, 256, 257};
    const s32 values[] = {0, -1, 0xA5, 0x100, 0x12345678, -2147483647 - 1, 2147483647, -559038737};
    check(allocation != NULL, "alignment fixture must allocate");
    aligned = allocation + 16 + ((16U - ((uintptr_t)(allocation + 16) & 15U)) & 15U);
    check(((uintptr_t)aligned & 15U) == 0, "the reference pointer must be aligned to sixteen bytes");
    for (alignment = 0; alignment < 16; alignment++) {
        for (count_index = 0; count_index < 97 + sizeof(extra_counts) / sizeof(extra_counts[0]); count_index++) {
            u32 count = count_index < 97 ? count_index : extra_counts[count_index - 97];
            for (value_index = 0; value_index < sizeof(values) / sizeof(values[0]); value_index++) {
                u8 *destination = aligned + alignment;
                memset(allocation, 0xA5, 544);
                check(func_0011A518(destination, values[value_index], count) == destination,
                      "actual byte fill must return the exact original host pointer");
                check_span(allocation, 544, (size_t)(destination - allocation), count, (u8)values[value_index]);
            }
        }
    }
    check(func_0011A518(NULL, -1, 0) == NULL, "zero count must not dereference a null destination");
    free(allocation);
}

static void test_guest_alignment_with_unaligned_host(void) {
    u8 storage[352];
    DreadzoneByteArena arena = {0x4000, 320, storage + 17};
    unsigned alignment, count;
    check(dreadzone_byte_fill_bind(&arena) == 0, "host placement must not constrain guest alignment");
    for (alignment = 0; alignment < 16; alignment++) {
        for (count = 0; count <= 97; count++) {
            memset(storage, 0xA5, sizeof(storage));
            check(dreadzone_byte_fill_guest(0x4000 + alignment, 0x12EF, count) == 0,
                  "guest alignment and short remainders must use the shared real algorithm");
            check_span(storage, sizeof(storage), 17 + alignment, count, 0xEF);
        }
    }
    check(dreadzone_byte_fill_bind(NULL) == -1, "unbind stack backing before its lifetime ends");
}

static void test_actual_preloop_fill(void) {
    const size_t total = DREADZONE_BYTE_FILL_BYTES + 64;
    u8 *storage = (u8 *)malloc(total);
    DreadzoneByteArena arena;
    unsigned alignment;
    check(storage != NULL, "actual pre-loop backing must allocate its full extent");
    arena.guest_base = 0x00280000 - 16;
    arena.bytes = DREADZONE_BYTE_FILL_BYTES + 32;
    arena.storage = storage + 16;
    check(dreadzone_byte_fill_bind(&arena) == 0, "actual pre-loop backing must bind explicitly");
    for (alignment = 0; alignment < 16; alignment++) {
        memset(storage, 0xA5, total);
        D_002437C4 = 0x00280000 + alignment;
        func_L01_005D6CB0();
        check(D_002437C4 == 0x00280000 + alignment, "the guest pointer global must remain intact");
        check_span(storage, total, 32 + alignment, DREADZONE_BYTE_FILL_BYTES, 0xEF);
    }
    check(dreadzone_byte_fill_bind(NULL) == -1, "unbind actual backing before free");
    free(storage);
}

static void test_mapping_failures_and_guest32_boundary(void) {
    u8 storage[96];
    DreadzoneByteArena arena = {0x4000, 64, storage + 16};
    DreadzoneByteArena wrapped = {0xFFFFFFF0U, 32, storage + 16};
    DreadzoneByteArena missing = {0x4000, 64, NULL};
    memset(storage, 0xA5, sizeof(storage));
    check(dreadzone_byte_fill_bind(&arena) == 0, "valid small arena must bind");
    check(dreadzone_byte_fill_guest(0x4040, -1, 0) == 0, "empty range at one-past must remain a no-op");
    check(dreadzone_byte_fill_guest(0x3FFF, -1, 1) == -1, "guest address below the arena must be rejected");
    check(dreadzone_byte_fill_guest(0x4040, -1, 1) == -1, "guest range beyond the arena must be rejected");
    check(dreadzone_byte_fill_guest(0x4000, -1, 65) == -1, "every store must preflight against the bound extent");
    check(dreadzone_byte_fill_guest(0x4000, -1, 0xFFFFFFFFU) == -1, "unsigned count must not wrap the requested end");
    check_span(storage, sizeof(storage), 0, 0, 0);
    arena.guest_base = 0xFFFFFFC0U;
    check(dreadzone_byte_fill_bind(&arena) == 0, "an arena ending exactly at 2^32 is valid");
    check(dreadzone_byte_fill_guest(0xFFFFFFC0U, 0x13EF, 64) == 0, "the full top-of-guest-space range must fill");
    check_span(storage, sizeof(storage), 16, 64, 0xEF);
    memset(storage, 0xA5, sizeof(storage));
    check(dreadzone_byte_fill_guest(0xFFFFFFFFU, 0xAB, 1) == 0, "the final guest byte must map without wrapping");
    check_span(storage, sizeof(storage), 79, 1, 0xAB);
    check(dreadzone_byte_fill_guest(0xFFFFFFFFU, 0, 2) == -1, "a request beyond guest32 must be rejected");
    check_span(storage, sizeof(storage), 79, 1, 0xAB);
    check(dreadzone_byte_fill_bind(&wrapped) == -1, "an arena extending beyond guest32 must be rejected");
    check(dreadzone_byte_fill_guest(0xFFFFFFC0U, 0, 1) == -1, "failed bind must invalidate the old mapping");
    check(dreadzone_byte_fill_bind(&missing) == -1, "missing host storage must be rejected");
    check(dreadzone_byte_fill_bind(NULL) == -1, "a null descriptor must be rejected");
}

static void test_actual_branch_condition(void) {
    s32 level;
    const s32 extremes[] = {-2147483647 - 1, 2147483647};
    unsigned index;
    for (level = -10; level <= 80; level++) {
        D_0021DE10 = level;
        check(func_L01_00666600() == (level >= 21 && level <= 35),
              "actual branch decision must be true exactly for levels twenty-one through thirty-five");
        check(func_00510EB0() == func_L01_00666600(), "matching resident and L01 decisions must agree");
        check(D_0021DE10 == level, "the branch decision must preserve its input global");
    }
    for (index = 0; index < sizeof(extremes) / sizeof(extremes[0]); index++) {
        D_0021DE10 = extremes[index];
        check(func_L01_00666600() == 0, "guest unsigned subtraction must remain defined at signed extremes");
        check(func_00510EB0() == 0, "matching resident decision preserves defined signed boundaries");
    }
}

int main(void) {
    test_core_alignment_and_counts();
    test_guest_alignment_with_unaligned_host();
    test_actual_preloop_fill();
    test_mapping_failures_and_guest32_boundary();
    test_actual_branch_condition();
    printf("Dreadzone byte fill: %u checks passed; return values, all alignments/counts, real 0x40000-byte fill and branch decision verified.\n", checks);
    return EXIT_SUCCESS;
}
