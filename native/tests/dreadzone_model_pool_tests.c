#include "dreadzone_model_pool.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(sizeof(DreadzoneModelDescriptor) == 8, "The descriptor is a retail eight-byte record");
_Static_assert(offsetof(DreadzoneModelDescriptor, first_record) == 4, "The record index stays at offset four");
_Static_assert(offsetof(DreadzoneModelDescriptor, released) == 7, "The release flag stays at offset seven");

enum { MATRIX_BASE = 0x00800000, DESCRIPTOR_BASE = 0x00900000, ACTOR_BASE = 0x00A00000,
       ACTOR_STRIDE = 0x100, ACTOR_BYTES = (DREADZONE_MODEL_DESCRIPTORS + 1) * ACTOR_STRIDE, GUARD = 17 };
static u8 matrices[DREADZONE_MODEL_MATRIX_BYTES + 2 * GUARD];
static u8 descriptors[DREADZONE_MODEL_DESCRIPTOR_BYTES + 2 * GUARD];
static u8 actors[ACTOR_BYTES + 2 * GUARD];
static u8 config[0x20 + 2 * GUARD], animation[0x100 + 2 * GUARD];
static u8 expected_matrices[sizeof(matrices)], expected_descriptors[sizeof(descriptors)], expected_actors[sizeof(actors)];
static unsigned checks;

static void check(int condition, const char *message) {
    checks++;
    if (!condition) { fprintf(stderr, "Model pool check failed: %s\n", message); exit(EXIT_FAILURE); }
}
static void compare(const u8 *actual, const u8 *expected, size_t bytes) {
    size_t i;
    for (i = 0; i < bytes; i++) check(actual[i] == expected[i], "every byte, stale field and guard must match");
}
static u32 word(const u8 *p) {
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}
static void put(u8 *p, u32 value) {
    p[0] = (u8)value; p[1] = (u8)(value >> 8); p[2] = (u8)(value >> 16); p[3] = (u8)(value >> 24);
}
static u32 actor_address(u32 i) { return ACTOR_BASE + i * ACTOR_STRIDE; }
static u8 *actor_bytes(u32 i) { return actors + GUARD + i * ACTOR_STRIDE; }
static void prepare(u32 i, u8 count) {
    put(actor_bytes(i) + 0x24, actor_address(i) + 0x80);
    actor_bytes(i)[0x88] = count;
    put(actor_bytes(i) + 0x58, 0);
}
static void fixture(void) {
    DreadzoneModelView views[] = {
        {MATRIX_BASE, DREADZONE_MODEL_MATRIX_BYTES, matrices + GUARD},
        {DESCRIPTOR_BASE, DREADZONE_MODEL_DESCRIPTOR_BYTES, descriptors + GUARD},
        {ACTOR_BASE, ACTOR_BYTES, actors + GUARD},
        {DREADZONE_MODEL_CONFIG, 0x20, config + GUARD},
        {DREADZONE_MODEL_ANIMATION, 0x100, animation + GUARD}
    };
    size_t i;
    for (i = 0; i < sizeof(matrices); i++) matrices[i] = (u8)((i * 73 + (i >> 7)) ^ 0xB9);
    memset(descriptors, 0xCB, sizeof(descriptors));
    memset(actors, 0xD7, sizeof(actors));
    memset(config, 0xE3, sizeof(config));
    memset(animation, 0xA9, sizeof(animation));
    put(config + GUARD + 0x18, MATRIX_BASE);
    put(config + GUARD + 0x1C, DESCRIPTOR_BASE);
    check(dreadzone_model_pool_bind(views, 5) == 0, "bounded disjoint guest views bind");
    memcpy(expected_matrices, matrices, sizeof(matrices));
    memcpy(expected_descriptors, descriptors, sizeof(descriptors));
    check(dreadzone_model_pool_init() == 0, "actual configuration initializes the pool");
    compare(matrices, expected_matrices, sizeof(matrices));
    compare(descriptors, expected_descriptors, sizeof(descriptors));
    check(D_00222D80 == MATRIX_BASE && D_00222D84 == 0x3094 && D_00222D8C == 0 &&
          D_00222D88 == 0 && D_00222D90 == 0 && D_00222D94 == 0x7FFFFFFF, "retail pool counters reset");
    check(D_00222D98 == 0 && D_00222D9C == 1, "animation counters reset");
    for (i = 0; i < sizeof(animation); i++) {
        size_t offset = i >= GUARD ? i - GUARD : 0x100;
        u8 expected = 0xA9;
        if (offset == 0 || offset == 1 || offset == 2 || offset == 4) expected = 0;
        if (offset == 3) expected = 2;
        if (offset < 0x100 && offset % 8 == 5) expected = offset / 8 == 0 || offset / 8 == 31 ? 0xFF : (u8)(offset / 8 + 1);
        check(animation[i] == expected, "all animation fields and free-list links match; other bytes remain intact");
    }
    check(dreadzone_model_pool_validate() == 0, "empty initialized pool is valid");
}
static u32 allocate(u32 i, u8 count) {
    u32 result = 0xDEADBEEF;
    u32 cursor = D_00222D80;
    prepare(i, count);
    check(dreadzone_model_pool_allocate(actor_address(i), &result) == 0, "real nonzero allocation succeeds");
    check(result == cursor && word(actor_bytes(i) + 0x58) == result, "actual actor pointer and return value agree");
    check(result != 0 && D_00222D80 == cursor + (u32)count * 64, "matrix stride and cursor are real");
    return result;
}

/* Independent expected effects use a stable pre-operation matrix snapshot.
 * Updating the source descriptor's index also changes stale trailing records,
 * as the retail compacting loop does before copying each eight-byte descriptor.
 */
static void compact_and_compare(void) {
    u8 *before = (u8 *)malloc(sizeof(matrices));
    u32 next, target, i, live;
    check(before != NULL, "oracle backing allocation succeeds");
    memcpy(before, matrices, sizeof(matrices));
    memcpy(expected_matrices, matrices, sizeof(matrices));
    memcpy(expected_descriptors, descriptors, sizeof(descriptors));
    memcpy(expected_actors, actors, sizeof(actors));
    target = (u32)D_00222D94;
    next = (u32)descriptors[GUARD + target * 8 + 4] | ((u32)descriptors[GUARD + target * 8 + 5] << 8);
    live = target;
    for (i = target + 1; i < (u32)D_00222D8C; i++) {
        u8 *record = expected_descriptors + GUARD + i * 8;
        if (record[7] == 0) {
            u32 actor = word(record), count = record[6];
            u32 actor_offset = actor - ACTOR_BASE;
            u32 source = word(actors + GUARD + actor_offset + 0x58) - MATRIX_BASE;
            put(expected_actors + GUARD + actor_offset + 0x58, MATRIX_BASE + next * 64);
            record[4] = (u8)next; record[5] = (u8)(next >> 8);
            memcpy(expected_matrices + GUARD + next * 64, before + GUARD + source, count * 64);
            memcpy(expected_descriptors + GUARD + live * 8, record, 8);
            next += count; live++;
        }
    }
    check(dreadzone_model_pool_compact() == 0, "real compaction succeeds");
    compare(matrices, expected_matrices, sizeof(matrices));
    compare(descriptors, expected_descriptors, sizeof(descriptors));
    compare(actors, expected_actors, sizeof(actors));
    check(D_00222D80 == MATRIX_BASE + next * 64 && D_00222D84 == 0x3094 - (s32)next &&
          D_00222D8C == (s32)live && D_00222D90 == 0 && D_00222D88 == 0 && D_00222D94 == 0x7FFFFFFF,
          "all compacted counters and endpoints match");
    check(dreadzone_model_pool_validate() == 0, "compacted pool remains valid");
    free(before);
}

static void test_matrix_capacity(void) {
    u32 i = 0, remaining = DREADZONE_MODEL_RECORDS, result;
    fixture();
    while (remaining != 0) {
        u8 count = (u8)(remaining > 255 ? 255 : remaining);
        allocate(i++, count); remaining -= count;
    }
    check(D_00222D84 == 0 && D_00222D80 == MATRIX_BASE + DREADZONE_MODEL_MATRIX_BYTES, "all actual matrix records can be allocated");
    compare(matrices, expected_matrices, sizeof(matrices));
    prepare(i, 1); result = 0xCAFE;
    check(dreadzone_model_pool_allocate(actor_address(i), &result) == DREADZONE_MODEL_CAPACITY && result == 0xCAFE,
          "exhaustion fails before a write or output change");
    for (remaining = 0; remaining < i; remaining += 2) {
        u32 old = word(actor_bytes(remaining) + 0x58);
        check(dreadzone_model_pool_release(actor_address(remaining)) == 0, "binary search finds each released matrix block");
        check(word(actor_bytes(remaining) + 0x58) == old, "release keeps the retail stale actor pointer");
    }
    compact_and_compare();
}

static void test_descriptor_capacity(void) {
    u32 i, result;
    fixture();
    for (i = 0; i < DREADZONE_MODEL_DESCRIPTORS; i++) allocate(i, 1);
    check(D_00222D8C == 0x400, "all 1024 descriptor slots are used");
    prepare(i, 1); result = 0x1234;
    check(dreadzone_model_pool_allocate(actor_address(i), &result) == DREADZONE_MODEL_CAPACITY && result == 0x1234,
          "descriptor exhaustion is detected independently of remaining matrices");
    for (i = 0; i < DREADZONE_MODEL_DESCRIPTORS; i += 3) check(dreadzone_model_pool_release(actor_address(i)) == 0, "release searches the entire descriptor table");
    compact_and_compare();
}

static void test_threshold_and_release(void) {
    u32 result, old;
    fixture();
    allocate(0, 1); allocate(1, 255); allocate(2, 7);
    check(dreadzone_model_pool_release(actor_address(1)) == 0, "middle block releases");
    memcpy(expected_matrices, matrices, sizeof(matrices));
    memcpy(expected_descriptors, descriptors, sizeof(descriptors));
    memcpy(expected_actors, actors, sizeof(actors));
    check(dreadzone_model_pool_compact() == 0, "retail threshold can defer compaction");
    compare(matrices, expected_matrices, sizeof(matrices));
    compare(descriptors, expected_descriptors, sizeof(descriptors));
    compare(actors, expected_actors, sizeof(actors));
    check(D_00222D8C == 3 && D_00222D90 == 1 && D_00222D88 == 255 && D_00222D94 == 1, "deferred counters stay intact");
    check(dreadzone_model_pool_release(actor_address(1)) == DREADZONE_MODEL_CORRUPT, "double release rejects a retail invalid state");
    prepare(3, 0);
    check(dreadzone_model_pool_release(actor_address(3)) == 0, "null matrix release is a no-op");
    put(actor_bytes(3) + 0x58, 0x1234); put(actor_bytes(3) + 0x24, 0);
    check(dreadzone_model_pool_release(actor_address(3)) == 0, "null definition release is a no-op");
    prepare(3, 0); put(actor_bytes(3) + 0x58, 0x1234);
    check(dreadzone_model_pool_release(actor_address(3)) == 0, "zero definition count release is a no-op");
    result = 0xFEDC;
    check(dreadzone_model_pool_allocate(actor_address(3), &result) == DREADZONE_MODEL_CORRUPT && result == 0xFEDC,
          "allocation requires the actual nonzero caller domain");
    old = word(actor_bytes(0) + 0x58);
    put(actor_bytes(0) + 0x58, old + 1);
    check(dreadzone_model_pool_release(actor_address(0)) == DREADZONE_MODEL_CORRUPT, "unaligned actor matrix link is rejected");
    put(actor_bytes(0) + 0x58, old);
    check(dreadzone_model_pool_allocate(actor_address(0), &result) == DREADZONE_MODEL_CORRUPT, "a live actor cannot be allocated twice");
    D_00222D94 = 3;
    check(dreadzone_model_pool_compact() == DREADZONE_MODEL_CORRUPT, "invalid earliest release cannot write outside the table");
    D_00222D94 = 1;
    descriptors[GUARD + 4] = 1;
    check(dreadzone_model_pool_compact() == DREADZONE_MODEL_CORRUPT, "noncontiguous descriptor offsets are rejected");
    descriptors[GUARD + 4] = 0;
    check(dreadzone_model_pool_validate() == 0, "restored state validates");
}

static void test_automatic_compaction(void) {
    u32 result = 0, i;
    fixture();
    for (i = 0; i < 48; i++) allocate(i, 255);
    allocate(48, 196);
    check(dreadzone_model_pool_release(actor_address(0)) == 0, "first released block becomes the compaction hole");
    prepare(49, 255);
    check(dreadzone_model_pool_allocate(actor_address(49), &result) == 0, "allocator invokes the genuine compacting path when matrix space runs out");
    check(result == MATRIX_BASE + (DREADZONE_MODEL_RECORDS - 255) * 64 && D_00222D84 == 0 && D_00222D8C == 49,
          "new matrices occupy exactly the recovered free span");
    check(word(actor_bytes(1) + 0x58) == MATRIX_BASE, "a live actor moves to the first hole");
    check(dreadzone_model_pool_validate() == 0, "automatically compacted pool validates");
    /* Release all live descriptors in reverse order: earliest tracking, binary
     * search upper endpoint and the all-freed compaction termination are covered.
     */
    for (i = 49; i != 0; i--) check(dreadzone_model_pool_release(actor_address(i)) == 0, "reverse release finds every active descriptor");
    compact_and_compare();
    check(D_00222D8C == 0 && D_00222D84 == 0x3094 && D_00222D80 == MATRIX_BASE, "all-freed compaction restores the full capacity");
}

static void test_mapping_failures(void) {
    DreadzoneModelView invalid[2];
    u32 result = 0xFACE;
    fixture();
    put(config + GUARD + 0x18, MATRIX_BASE + 1);
    check(dreadzone_model_pool_init() == DREADZONE_MODEL_CORRUPT, "unaligned configured matrices fail before any state change");
    check(D_00222D7C == MATRIX_BASE && D_00222D80 == MATRIX_BASE && D_00222D84 == 0x3094,
          "failed init preserves all initialized state");
    put(config + GUARD + 0x18, MATRIX_BASE);
    put(config + GUARD + 0x1C, MATRIX_BASE);
    check(dreadzone_model_pool_init() == DREADZONE_MODEL_CORRUPT, "matrix and descriptor ranges cannot alias");
    put(config + GUARD + 0x1C, DESCRIPTOR_BASE);
    put(config + GUARD + 0x18, MATRIX_BASE + 16);
    check(dreadzone_model_pool_init() == DREADZONE_MODEL_UNMAPPED, "configured matrix span must include its full actual capacity");
    put(config + GUARD + 0x18, 0);
    check(dreadzone_model_pool_init() == DREADZONE_MODEL_CORRUPT, "a null configured matrix base cannot become a successful allocation");
    put(config + GUARD + 0x18, MATRIX_BASE);
    compare(matrices, expected_matrices, sizeof(matrices));
    compare(descriptors, expected_descriptors, sizeof(descriptors));
    check(dreadzone_model_pool_allocate(ACTOR_BASE + ACTOR_BYTES - 0x5B, &result) == DREADZONE_MODEL_UNMAPPED && result == 0xFACE,
          "actor end boundary is checked before reading a field");
    prepare(0, 1); put(actor_bytes(0) + 0x24, ACTOR_BASE + ACTOR_BYTES - 8);
    check(dreadzone_model_pool_allocate(ACTOR_BASE, &result) == DREADZONE_MODEL_UNMAPPED, "definition must include its actual count byte");
    invalid[0].guest_base = 0xFFFFFFF0U; invalid[0].bytes = 17; invalid[0].storage = config;
    check(dreadzone_model_pool_bind(invalid, 1) == DREADZONE_MODEL_UNMAPPED, "guest wrap beyond 2^32 is rejected");
    invalid[0].bytes = 16;
    check(dreadzone_model_pool_bind(invalid, 1) == 0, "a backing can terminate exactly at 2^32");
    check(dreadzone_model_resolve(0xFFFFFFFFU, 1) == config + 15 && dreadzone_model_resolve(0xFFFFFFFFU, 2) == NULL,
          "last guest byte translates safely without overflow");
    invalid[0].guest_base = ACTOR_BASE; invalid[0].bytes = 16;
    invalid[1] = invalid[0]; invalid[1].storage = animation;
    check(dreadzone_model_pool_bind(invalid, 2) == DREADZONE_MODEL_CORRUPT, "overlapping guest views are rejected");
    invalid[1].guest_base = ACTOR_BASE + 32; invalid[1].storage = config + 1;
    check(dreadzone_model_pool_bind(invalid, 2) == DREADZONE_MODEL_CORRUPT, "host aliasing between separate guest regions is rejected");
    check(dreadzone_model_pool_init() == DREADZONE_MODEL_UNMAPPED, "failed bind invalidates stale host mappings");
}

int main(void) {
    test_matrix_capacity();
    test_descriptor_capacity();
    test_threshold_and_release();
    test_automatic_compaction();
    test_mapping_failures();
    printf("Dreadzone model pool: %u checks passed\n", checks);
    return EXIT_SUCCESS;
}
