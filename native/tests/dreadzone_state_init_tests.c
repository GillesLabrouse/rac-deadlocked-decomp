#include "dreadzone_state_init.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

s32 D_0021DE10;
u8 D_0021E868;
u32 D_0021DE98[DREADZONE_STATE_SLOTS];
u32 D_0021DEA8[DREADZONE_STATE_SLOTS];
u32 D_0021DEB8[DREADZONE_STATE_SLOTS];
u32 D_0021DE88, D_0021DE8C, D_0021DE90;
s32 D_0021DEC4;
u32 D_0021E734, D_0021E738;

_Static_assert(sizeof(DreadzoneStateEntry) == 8, "Retail record stride must stay eight bytes");
_Static_assert(offsetof(DreadzoneStateEntry, f4) == 4, "The preserved second word must stay at offset four");
_Static_assert(sizeof(D_0021DE98[0]) == 4, "Published guest addresses must stay 32-bit");

static unsigned checks;
static void check(int condition, const char *message) {
    checks++;
    if (!condition) {
        fprintf(stderr, "State init check failed: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

static void put_word(u8 *p, u32 word) {
    p[0] = (u8)word; p[1] = (u8)(word >> 8);
    p[2] = (u8)(word >> 16); p[3] = (u8)(word >> 24);
}

static void check_bytes(const u8 *actual, const u8 *expected, size_t size) {
    size_t index;
    for (index = 0; index < size; index++) {
        check(actual[index] == expected[index], "all record words, object bytes, gaps and guards must match");
    }
}

static void seed_globals(void) {
    unsigned index;
    for (index = 0; index < DREADZONE_STATE_SLOTS; index++) {
        D_0021DE98[index] = 0xA0000000U + index;
        D_0021DEA8[index] = 0xB0000000U + index;
        D_0021DEB8[index] = 0xC0000000U + index;
    }
    D_0021DE88 = 0xD0000000U;
    D_0021DE8C = 0xD0000001U;
    D_0021DE90 = 0xD0000002U;
    D_0021E734 = 0xD0000003U;
    D_0021E738 = 0xD0000004U;
    D_0021DEC4 = -23;
}

/* Independently authored records cover zero/nonzero object flags, both low
 * nibble and high-word preservation, the largest encoded object offset, and
 * every record in a table with the retail fixed-table count (207).
 */
static void prepare_table(u8 *actual, u8 *expected, u32 arena_base, u32 table, unsigned count) {
    u32 first = table - arena_base;
    u32 end = first + 4 + count * 8;
    unsigned index;
    put_word(actual + first, count);
    put_word(expected + first, count);
    for (index = 0; index < count; index++) {
        u32 offset = index + 1 == count ? 0x3FFC : index * 4;
        u32 word = 0xA0000000U | (index << 16) | (offset << 2) | (index & 15);
        u32 record = first + 4 + index * 8;
        u8 flag = index % 3 == 0 ? 0 : 0xE7;
        put_word(actual + record, word);
        put_word(expected + record, flag == 0 ? word : (word & 0xFFFF000F));
        put_word(actual + record + 4, 0x71820000U + index);
        put_word(expected + record + 4, 0x71820000U + index);
        actual[end + offset + 3] = expected[end + offset + 3] = flag;
    }
}

static void test_actual_startup_branches(void) {
    const u32 guest_base = 0x00173000;
    const u32 arena_bytes = 0x00100000;
    const size_t total = arena_bytes + 32;
    u8 *allocation = (u8 *)malloc(total);
    u8 *expected = (u8 *)malloc(total);
    DreadzoneStateArena arena;
    const struct { s32 level; u8 enabled; unsigned dynamic_count; } cases[] = {
        {0, 0, 5}, {0, 1, 5}, {-1, 0, 5}, {-1, 1, 5}, {17, 0, 5}, {17, 0xFF, 5},
        {-1, 0, 0}, {0, 1, 0}
    };
    size_t case_index;
    check(allocation != NULL && expected != NULL, "both complete fixture arenas must allocate");
    arena.guest_base = guest_base;
    arena.bytes = arena_bytes;
    arena.storage = allocation + 16;
    for (case_index = 0; case_index < sizeof(cases) / sizeof(cases[0]); case_index++) {
        int dynamic = cases[case_index].level < 0 || cases[case_index].enabled != 0;
        memset(allocation, 0xA5, total);
        memset(expected, 0xA5, total);
        prepare_table(allocation + 16, expected + 16, guest_base, 0x00173C00, 207);
        prepare_table(allocation + 16, expected + 16, guest_base, 0x0017C740, 207);
        if (dynamic) {
            prepare_table(allocation + 16, expected + 16, guest_base, 0x0021E1D4, cases[case_index].dynamic_count);
        } else {
            /* This table must remain entirely unread and unchanged in the
             * disabled branch, despite an invalid huge count in its header.
             */
            put_word(allocation + 16 + 0x0021E1D4 - guest_base, 0x7FFFFFFF);
            put_word(expected + 16 + 0x0021E1D4 - guest_base, 0x7FFFFFFF);
        }
        seed_globals();
        D_0021DE10 = cases[case_index].level;
        D_0021E868 = cases[case_index].enabled;
        check(dreadzone_state_bind(&arena) == 0, "actual guest tables must bind to owned host storage");
        func_L01_0055DFE8();
        check(D_0021DE98[0] == 0x001743C0 && D_0021DE98[1] == 0x0017CF80,
              "startup must publish both fixed guest bases");
        check(D_0021DEA8[0] == 0x00173C04 && D_0021DEA8[1] == 0x0017C744,
              "record arrays must begin four bytes after their signed count");
        check(D_0021DEB8[0] == 0x0017427C && D_0021DEB8[1] == 0x0017CDBC,
              "207 records must use the verified eight-byte stride");
        check(D_0021DE98[2] == (dynamic ? 0x0021E1D0U : 0), "slot two base must obey both condition globals");
        check(D_0021DEA8[2] == (dynamic ? 0x0021E1D8U : 0), "slot two records must obey the same branch");
        check(D_0021DEB8[2] == (dynamic ? 0x0021E1D8U + cases[case_index].dynamic_count * 8 : 0),
              "slot two end must obey the same branch, including the retail initially empty table");
        check(D_0021DE88 == D_0021DE98[0] && D_0021DE8C == D_0021DEA8[0] &&
              D_0021DE90 == D_0021DEB8[0] && D_0021DEC4 == 0,
              "the actual entry must select slot zero after all initialization");
        check(D_0021E734 == 0x003B6DB0 && D_0021E738 == 0x003B6E70,
              "selection must publish actual guest buffer addresses");
        check_bytes(allocation, expected, total);
    }
    check(dreadzone_state_bind(NULL) == -1, "unbind before releasing host storage");
    free(expected);
    free(allocation);
}

static void test_signed_counts_and_null_object(void) {
    u8 actual[128], expected[128];
    DreadzoneStateArena arena = {0x4000, 96, actual + 16};
    const u32 counts[] = {0, 0xFFFFFFFFU, 0x80000000U};
    size_t index;
    for (index = 0; index < sizeof(counts) / sizeof(counts[0]); index++) {
        memset(actual, 0xA5, sizeof(actual));
        put_word(actual + 16, counts[index]);
        memcpy(expected, actual, sizeof(actual));
        seed_globals();
        check(dreadzone_state_bind(&arena) == 0, "nonpositive-count table must bind");
        func_L01_0055DCF8(1, 0x87654321U, 0x4000);
        check(D_0021DE98[1] == 0x87654321U && D_0021DEA8[1] == 0x4004,
              "nonpositive counts still publish base and first-record addresses");
        check(D_0021DEB8[1] == 0x4004U + (counts[index] << 3),
              "end address arithmetic must preserve guest32 shift and wrap");
        check(D_0021DE98[0] == 0xA0000000U && D_0021DE98[2] == 0xA0000002U,
              "the helper must preserve other slots");
        check_bytes(actual, expected, sizeof(actual));
    }
    /* A table ending exactly at 2^32 gives a null computed object when its
     * encoded offset is zero. Retail skips the byte read in that case.
     */
    arena.guest_base = 0xFFFFFFC0U;
    arena.bytes = 64;
    memset(actual, 0xA5, sizeof(actual));
    put_word(actual + 16 + 52, 1);
    put_word(actual + 16 + 56, 0xFEDC0003U);
    put_word(actual + 16 + 60, 0x87654321U);
    memcpy(expected, actual, sizeof(actual));
    check(dreadzone_state_bind(&arena) == 0, "an arena ending exactly at 2^32 is valid");
    check(dreadzone_state_init_slot(2, 0, 0xFFFFFFF4U) == 0, "null computed objects must skip dereference");
    check(D_0021DEB8[2] == 0 && D_0021DEA8[2] == 0xFFFFFFF8U, "published end must wrap as guest32");
    check_bytes(actual, expected, sizeof(actual));
    check(dreadzone_state_bind(NULL) == -1, "stack arena must unbind before its lifetime ends");
}

static void test_selector(void) {
    const s32 levels[] = {-1, 0, 23};
    unsigned level, flag, slot;
    for (level = 0; level < 3; level++) {
        for (flag = 0; flag < 2; flag++) {
            for (slot = 0; slot < DREADZONE_STATE_SLOTS; slot++) {
                int blocked = levels[level] >= 0 && flag == 0 && slot == 2;
                seed_globals();
                D_0021DE10 = levels[level];
                D_0021E868 = (u8)flag;
                func_L01_0055E0A0((s32)slot);
                check(D_0021DE88 == (blocked ? 0xD0000000U : 0xA0000000U + slot), "selector base must match the enabled slot");
                check(D_0021DE8C == (blocked ? 0xD0000001U : 0xB0000000U + slot), "selector record address must remain guest32");
                check(D_0021DE90 == (blocked ? 0xD0000002U : 0xC0000000U + slot), "selector end must remain guest32");
                check(D_0021E734 == (blocked ? 0xD0000003U : 0x003B6DB0U + slot * 0x40), "first buffer stride must be 0x40");
                check(D_0021E738 == (blocked ? 0xD0000004U : 0x003B6E70U + slot * 0x40), "second buffer stride must be 0x40");
                check(D_0021DEC4 == (blocked ? -23 : (s32)slot), "a blocked selection must preserve the current index");
            }
        }
    }
}

static void test_preflight_failures(void) {
    u8 actual[96], expected[96];
    DreadzoneStateArena arena = {0x4000, 64, actual + 16};
    DreadzoneStateArena wrapped = {0xFFFFFFF0U, 32, actual + 16};
    DreadzoneStateArena missing = {0x4000, 64, NULL};
    memset(actual, 0xA5, sizeof(actual));
    put_word(actual + 16, 2);
    put_word(actual + 20, 0x00000010); /* Mapped object at table end + 4. */
    put_word(actual + 28, 0x0000FFF0); /* Unmapped object at table end + 3FFC. */
    memcpy(expected, actual, sizeof(actual));
    seed_globals();
    check(dreadzone_state_bind(&arena) == 0, "valid small arena must bind");
    check(dreadzone_state_init_slot(1, 0, 0x4000) == -1, "all object reads must preflight before the first record is changed");
    check(dreadzone_state_init_slot(-1, 0, 0x4000) == -1, "negative host slot must be rejected");
    check(dreadzone_state_init_slot(3, 0, 0x4000) == -1, "host slot beyond three recovered slots must be rejected");
    check(dreadzone_state_init_slot(0, 0, 0x4001) == -1, "retail word reads require a word-aligned table");
    check(dreadzone_state_init_slot(0, 0, 0x3FFC) == -1, "unmapped count must be rejected");
    check(dreadzone_state_init_slot(0, 0, 0x4040) == -1, "one-past count must be rejected");
    check(D_0021DE98[1] == 0xA0000001U && D_0021DEA8[1] == 0xB0000001U &&
          D_0021DEB8[1] == 0xC0000001U, "failed preflight must not publish partial slot state");
    check_bytes(actual, expected, sizeof(actual));
    put_word(actual + 16, 0x7FFFFFFF);
    check(dreadzone_state_init_slot(0, 0, 0x4000) == -1, "huge positive count must fail extent validation before looping");
    check(dreadzone_state_bind(&wrapped) == -1, "an arena extending beyond guest32 must be rejected");
    check(dreadzone_state_init_slot(0, 0, 0x4000) == -1, "failed bind must invalidate the previous arena");
    check(dreadzone_state_bind(&missing) == -1, "missing host storage must be rejected");
    check(dreadzone_state_bind(NULL) == -1, "a null descriptor must be rejected");
}

int main(void) {
    test_actual_startup_branches();
    test_signed_counts_and_null_object();
    test_selector();
    test_preflight_failures();
    printf("Dreadzone state init: %u checks passed; actual entry, all 207 fixed-table records, branches and guest32 bounds verified.\n", checks);
    return EXIT_SUCCESS;
}
