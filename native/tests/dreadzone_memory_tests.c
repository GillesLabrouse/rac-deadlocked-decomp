#include "dreadzone_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(sizeof(DreadzoneWorkBlock) == 0x38, "Work block layout changed");
_Static_assert(offsetof(DreadzoneWorkBlock, f30) == 0x30, "Work cursor moved");
_Static_assert(offsetof(DreadzoneWorkBlock, f34) == 0x34, "Work count moved");
_Static_assert(sizeof(DreadzoneScratchRecord) == 0x10, "Scratch stride changed");
_Static_assert(sizeof(DreadzonePoolRecord) == 0x5C, "Pool stride changed");
_Static_assert(offsetof(DreadzonePoolRecord, f58) == 0x58, "Guest link moved");
_Static_assert(sizeof(((DreadzonePoolRecord *)0)->f58) == 4, "Guest link must remain 32 bits");

DreadzoneWorkBlock D_001DFA70;
DreadzoneScratchRecord D_0034D320[DREADZONE_SCRATCH_COUNT];
DreadzonePoolRecord D_00375720[DREADZONE_POOL_COUNT];
u32 D_00220828;

static unsigned checks;

static void check(int condition, const char *message) {
    checks++;
    if (!condition) {
        fprintf(stderr, "Dreadzone memory check failed: %s\n", message);
        exit(1);
    }
}

static void check_reset_record(const DreadzonePoolRecord *record, u32 untouched_link) {
    unsigned word;
    check(record->f0 == 0 && record->f4 == 0 && record->f8 == 0,
          "record's first three words must be cleared");
    for (word = 0; word < 16; word++) {
        check(record->fC[word] == 0xCDCDCDCDU, "all 64 payload bytes must have the recovered pattern");
    }
    check(record->f4C == 0, "record field 4C must be cleared");
    check(record->f50 == -1, "record field 50 must have the recovered sentinel");
    check(record->f54 == 0 && record->f55 == 0, "record bytes 54 and 55 must be cleared");
    check(record->gap56[0] == 0xA5 && record->gap56[1] == 0xA5,
          "record bytes 56 and 57 must remain intact");
    check(record->f58 == untouched_link, "the reset helper must preserve the guest link at 58");
}

static void test_word_filling(void) {
    struct { u32 before; u32 words[16]; u32 after; } buffer;
    unsigned index;
    memset(&buffer, 0xA5, sizeof(buffer));
    func_L01_0055D2A8(buffer.words, 0x12345678U, 0x40);
    for (index = 0; index < 16; index++) {
        check(buffer.words[index] == 0x12345678U, "word filling must store the full u32 pattern");
    }
    check(buffer.before == 0xA5A5A5A5U && buffer.after == 0xA5A5A5A5U,
          "word filling must respect both boundaries of its positive aligned count");
}

static void test_work_and_scratch(void) {
    DreadzoneWorkBlock expected;
    DreadzoneScratchRecord original[DREADZONE_SCRATCH_COUNT];
    unsigned index;
    memset(&D_001DFA70, 0xA5, sizeof(D_001DFA70));
    D_001DFA70.f0 = 0xDEADBEEFU;
    expected = D_001DFA70;
    expected.f30 = expected.f0;
    expected.f34 = 0;
    for (index = 0; index < DREADZONE_SCRATCH_COUNT; index++) {
        D_0034D320[index].f0 = 0x10000000U + index;
        D_0034D320[index].f4 = 0x20000000U + index;
        D_0034D320[index].f8 = 0x30000000U + index;
        D_0034D320[index].fC = 0x40000000U + index;
    }
    memcpy(original, D_0034D320, sizeof(original));
    func_L01_00628020();
    check(memcmp(&D_001DFA70, &expected, sizeof(expected)) == 0,
          "only fields 30 and 34 may change in the work block; f0 is a raw guest value");
    for (index = 0; index < DREADZONE_SCRATCH_COUNT; index++) {
        check(D_0034D320[index].f0 == 0 && D_0034D320[index].f4 == 0,
              "every one of the 64 scratch records must be reset");
        check(D_0034D320[index].f8 == original[index].f8 && D_0034D320[index].fC == original[index].fC,
              "scratch fields 8 and C must remain intact");
    }
    func_L01_00628020();
    check(memcmp(&D_001DFA70, &expected, sizeof(expected)) == 0,
          "work reset must preserve its observed result when repeated");
}

static void test_record_reset_boundaries(void) {
    struct { u32 before; DreadzonePoolRecord record; u32 after; } guarded;
    memset(&guarded, 0xA5, sizeof(guarded));
    guarded.record.f58 = 0xFEDCBA98U;
    func_L01_006A9AE0(&guarded.record);
    check_reset_record(&guarded.record, 0xFEDCBA98U);
    check(guarded.before == 0xA5A5A5A5U && guarded.after == 0xA5A5A5A5U,
          "single-record reset must not touch adjacent storage");
}

static void prepare_pool(u32 last_link) {
    memset(D_00375720, 0xA5, sizeof(D_00375720));
    D_00375720[DREADZONE_POOL_COUNT - 1].f58 = last_link;
    D_00220828 = 0xFFFFFFFFU;
}

static void test_pool_all_records_and_links(void) {
    unsigned index;
    u32 address;
    unsigned visited[DREADZONE_POOL_COUNT] = {0};
    /* Retail's final link is not initialized here. Zero-filled initial storage
     * is an explicit fixture precondition for a null-terminated free list.
     */
    prepare_pool(0);
    func_L01_006A9640();
    check(D_00220828 == DREADZONE_POOL_GUEST_BASE, "the head must be a guest address, not a host pointer");
    for (index = 0; index < DREADZONE_POOL_COUNT; index++) {
        u32 expected_link = index + 1 < DREADZONE_POOL_COUNT
                            ? DREADZONE_POOL_GUEST_BASE + (index + 1) * DREADZONE_POOL_RECORD_SIZE : 0;
        check_reset_record(&D_00375720[index], expected_link);
    }
    address = D_00220828;
    for (index = 0; index < DREADZONE_POOL_COUNT; index++) {
        u32 offset;
        unsigned record_index;
        check(address >= DREADZONE_POOL_GUEST_BASE, "every traversed link must lie at or after the guest pool base");
        offset = address - DREADZONE_POOL_GUEST_BASE;
        check(offset % DREADZONE_POOL_RECORD_SIZE == 0, "each link must identify a record start");
        record_index = offset / DREADZONE_POOL_RECORD_SIZE;
        check(record_index < DREADZONE_POOL_COUNT, "each link must stay within the 256-record pool");
        check(record_index == index && visited[record_index] == 0, "the complete chain must visit each record once in order");
        visited[record_index] = 1;
        address = D_00375720[record_index].f58;
    }
    check(address == 0, "a zero initial final link must terminate after exactly 256 records");
}

static void test_final_link_is_not_fabricated(void) {
    prepare_pool(0xBAADF00DU);
    func_L01_006A9640();
    check(D_00375720[DREADZONE_POOL_COUNT - 1].f58 == 0xBAADF00DU,
          "the retail initializer preserves an arbitrary final link; it does not force null termination");
    check(D_00375720[0].f58 == DREADZONE_POOL_GUEST_BASE + DREADZONE_POOL_RECORD_SIZE,
          "the first link must still be regenerated when the last link is nonzero");
}

int main(void) {
    test_word_filling();
    test_work_and_scratch();
    test_record_reset_boundaries();
    test_pool_all_records_and_links();
    test_final_link_is_not_fabricated();
    printf("Dreadzone memory initialization: %u checks passed; 64 scratch and 256 pool records verified.\n", checks);
    return 0;
}
