#include "dreadzone_bulk_clear.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(DREADZONE_BULK_BYTES == 0x17E240, "Retail clear extent changed");
_Static_assert(sizeof(((DreadzoneBulkArena *)0)->guest_base) == 4, "Guest base must remain 32 bits");
_Static_assert(sizeof(((DreadzoneBulkArena *)0)->bytes) == 4, "Guest extent must remain 32 bits");

static unsigned checks;

static void check(int condition, const char *message) {
    checks++;
    if (!condition) {
        fprintf(stderr, "Dreadzone bulk clear check failed: %s\n", message);
        exit(1);
    }
}

static void check_bytes(const u8 *bytes, size_t total, size_t start, size_t count, u8 original) {
    size_t index;
    for (index = 0; index < total; index++) {
        u8 expected = index >= start && index - start < count ? 0 : original;
        check(bytes[index] == expected, "every cleared byte and every unchanged boundary byte must match");
    }
}

static void test_real_entry_extent(void) {
    size_t total = DREADZONE_BULK_BYTES + 64;
    u8 *allocation = (u8 *)malloc(total);
    DreadzoneBulkArena arena;
    check(allocation != NULL, "the fixture arena must allocate its full backing storage");
    memset(allocation, 0xA5, total);
    arena.guest_base = DREADZONE_BULK_START - 16;
    arena.bytes = DREADZONE_BULK_BYTES + 32;
    arena.storage = allocation + 16;
    check(dreadzone_bulk_translate(&arena, DREADZONE_BULK_START, DREADZONE_BULK_BYTES) == allocation + 32,
          "actual endpoints must map into the owned host allocation");
    check(dreadzone_bulk_bind(&arena) == 0, "the real clear span must bind successfully");
    func_L01_00529280();
    check_bytes(allocation, total, 32, DREADZONE_BULK_BYTES, 0xA5);
    memset(allocation + 32, 0xCC, DREADZONE_BULK_BYTES);
    func_L01_00529280();
    check_bytes(allocation, total, 32, DREADZONE_BULK_BYTES, 0xA5);
    /* Remove the binding before any later caller could reuse the freed arena. */
    check(dreadzone_bulk_bind(NULL) == -1, "null binding must be rejected");
    free(allocation);
}

static void test_alignment_branches(void) {
    u8 buffer[160];
    DreadzoneBulkArena arena = {0x8000, 128, buffer + 16};
    const struct { u32 start, end; size_t first, count; } cases[] = {
        {0x8000, 0x8020, 16, 32},
        {0x8001, 0x8021, 17, 32},
        {0x8010, 0x8025, 32, 21},
        {0x8011, 0x8026, 33, 21},
        {0x8030, 0x8030, 64, 16},
        {0x8031, 0x8031, 65, 0}
    };
    size_t index;
    for (index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        memset(buffer, 0xA5, sizeof(buffer));
        check(dreadzone_bulk_clear_range(&arena, cases[index].start, cases[index].end) == 0,
              "aligned/bytewise fixture range must be fully mapped before clearing");
        check_bytes(buffer, sizeof(buffer), cases[index].first, cases[index].count, 0xA5);
    }
    /* Host alignment is independent of the logical guest alignment. */
    arena.storage = buffer + 17;
    memset(buffer, 0xA5, sizeof(buffer));
    check(dreadzone_bulk_clear_range(&arena, 0x8000, 0x8020) == 0,
          "aligned guest quads must remain safe with unaligned host placement");
    check_bytes(buffer, sizeof(buffer), 17, 32, 0xA5);
}

static void test_genuine_quad_footprints(void) {
    u8 buffer[96];
    const struct { s32 bytes; size_t footprint; } cases[] = {
        {-1, 16}, {0, 16}, {1, 16}, {16, 16}, {17, 32}, {32, 32}, {48, 48}, {64, 64}
    };
    size_t index;
    /* The helper's unconditional first store and round-up are directly visible
     * in retail. Avoid INT_MIN subtraction overflow; game startup uses 17E240.
     */
    for (index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        memset(buffer, 0xA5, sizeof(buffer));
        func_L01_0055D2D0(buffer + 16, cases[index].bytes);
        check_bytes(buffer, sizeof(buffer), 16, cases[index].footprint, 0xA5);
    }
}

static void test_translation_and_preflight_failures(void) {
    u8 buffer[96];
    DreadzoneBulkArena arena = {0x1000, 64, buffer + 16};
    DreadzoneBulkArena wrapped = {0xFFFFFFF0U, 32, buffer + 16};
    DreadzoneBulkArena missing = {0x1000, 64, NULL};
    memset(buffer, 0xA5, sizeof(buffer));
    check(dreadzone_bulk_translate(&arena, 0x1000, 64) == buffer + 16, "full arena mapping must be valid");
    check(dreadzone_bulk_translate(&arena, 0x1040, 0) == buffer + 80, "an empty translation at the end may be one-past");
    check(dreadzone_bulk_translate(&arena, 0x0FFF, 1) == NULL, "translation must reject addresses below the arena");
    check(dreadzone_bulk_translate(&arena, 0x1040, 1) == NULL, "translation must reject bytes beyond the end");
    check(dreadzone_bulk_translate(&arena, 0xFFFFFFFFU, 2) == NULL, "request arithmetic must not wrap at 32 bits");
    check(dreadzone_bulk_translate(&wrapped, 0xFFFFFFF0U, 1) == NULL, "an overflowing arena extent must be rejected");
    check(dreadzone_bulk_translate(&missing, 0x1000, 1) == NULL, "missing storage must be rejected");
    check(dreadzone_bulk_translate(NULL, 0x1000, 1) == NULL, "a missing descriptor must be rejected");
    check(dreadzone_bulk_clear_range(&arena, 0x1000, 0x1041) == -1, "clear must reject unmapped end before any store");
    check(dreadzone_bulk_clear_range(&arena, 0x1010, 0x1000) == -1, "the host interface rejects reversed endpoints");
    check(dreadzone_bulk_clear_range(&arena, 0x1000, 0x90000000U) == -1, "the host interface rejects a count outside signed range");
    check(dreadzone_bulk_clear_range(&arena, 0x1040, 0x1040) == -1,
          "empty aligned ranges need the helper's actual 16-byte first-store footprint");
    check(dreadzone_bulk_bind(&arena) == -1, "binding must cover the entire actual Dreadzone clear range");
    check_bytes(buffer, sizeof(buffer), 0, 0, 0xA5);
}

int main(void) {
    test_real_entry_extent();
    test_alignment_branches();
    test_genuine_quad_footprints();
    test_translation_and_preflight_failures();
    printf("Dreadzone bulk clear: %u checks passed; all %u actual-range bytes and boundaries verified.\n",
           checks, (unsigned)DREADZONE_BULK_BYTES);
    return 0;
}
