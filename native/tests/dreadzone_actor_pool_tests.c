#include "../dreadzone_actor_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "Check failed at line %d: %s\n", __LINE__, #condition); \
    exit(EXIT_FAILURE); } } while (0)

enum { DESCRIPTOR_BASE = 0x00300000, PAYLOAD_BASE = 0x00400000 };
typedef struct Fixture {
    u32 words[10];
    u8 bytes[640];
    DreadzonePayloadArena arena;
    DreadzonePayloadState state;
} Fixture;

static void setup(Fixture *fixture, u32 count, u32 capacity) {
    memset(fixture, 0, sizeof(*fixture));
    fixture->words[0] = 0x13579BDF;
    fixture->words[9] = 0x2468ACE0;
    memset(fixture->bytes, 0xA5, sizeof(fixture->bytes));
    fixture->arena.descriptor_base = DESCRIPTOR_BASE;
    fixture->arena.descriptor_count = count;
    fixture->arena.descriptors = fixture->words + 1;
    fixture->arena.payload_base = PAYLOAD_BASE;
    fixture->arena.payload_bytes = capacity;
    fixture->arena.payload = fixture->bytes + 64;
    fixture->state.cursor = DESCRIPTOR_BASE;
}

static void check_guards(const Fixture *fixture) {
    u32 i;
    CHECK(fixture->words[0] == 0x13579BDF);
    CHECK(fixture->words[9] == 0x2468ACE0);
    for (i = 0; i < 64; i++) CHECK(fixture->bytes[i] == 0xA5);
    for (i = 576; i < sizeof(fixture->bytes); i++) CHECK(fixture->bytes[i] == 0xA5);
}

static void allocation_failure(Fixture *fixture, u32 bytes) {
    u32 words[10];
    u8 payload[640];
    DreadzonePayloadState state = fixture->state;
    memcpy(words, fixture->words, sizeof(words));
    memcpy(payload, fixture->bytes, sizeof(payload));
    CHECK(dreadzone_payload_allocate_zeroed(&fixture->arena, &fixture->state, bytes) == 0);
    CHECK(memcmp(words, fixture->words, sizeof(words)) == 0);
    CHECK(memcmp(payload, fixture->bytes, sizeof(payload)) == 0);
    CHECK(memcmp(&state, &fixture->state, sizeof(state)) == 0);
}

static void release_failure(Fixture *fixture, u32 address) {
    u32 words[10];
    u8 payload[640];
    DreadzonePayloadState state = fixture->state;
    memcpy(words, fixture->words, sizeof(words));
    memcpy(payload, fixture->bytes, sizeof(payload));
    CHECK(dreadzone_payload_release(&fixture->arena, &fixture->state, address) == 0);
    CHECK(memcmp(words, fixture->words, sizeof(words)) == 0);
    CHECK(memcmp(payload, fixture->bytes, sizeof(payload)) == 0);
    CHECK(memcmp(&state, &fixture->state, sizeof(state)) == 0);
}

static void check_rounding_and_clear(void) {
    Fixture fixture;
    u32 i;
    setup(&fixture, 8, 512);
    CHECK(dreadzone_payload_allocate_zeroed(&fixture.arena, &fixture.state, 240) == PAYLOAD_BASE);
    CHECK(fixture.words[1] == 0xFFF00004U);
    CHECK(fixture.state.head == DESCRIPTOR_BASE && fixture.state.tail == DESCRIPTOR_BASE);
    CHECK(fixture.state.cursor == DESCRIPTOR_BASE + 4);
    CHECK(dreadzone_payload_allocate_zeroed(&fixture.arena, &fixture.state, 65) == PAYLOAD_BASE + 256);
    CHECK(fixture.words[1] == 0x00100004U && fixture.words[2] == 0xFFF00402U);
    CHECK(fixture.state.tail == DESCRIPTOR_BASE + 4 && fixture.state.cursor == DESCRIPTOR_BASE + 8);
    for (i = 0; i < 512; i++) {
        u8 expected = (i < 240 || (i >= 256 && i < 324)) ? 0 : 0xA5;
        CHECK(fixture.arena.payload[i] == expected);
    }
    allocation_failure(&fixture, 200);
    CHECK(dreadzone_payload_allocate(&fixture.arena, &fixture.state, 128) == PAYLOAD_BASE + 384);
    CHECK(fixture.words[2] == 0x00200402U && fixture.words[3] == 0xFFF00602U);
    allocation_failure(&fixture, 1);
    check_guards(&fixture);
}

static void check_word_clear_footprints(void) {
    static const u32 requests[] = {1, 3, 4, 5, 65};
    static const u32 footprints[] = {4, 4, 4, 8, 68};
    static const u32 descriptors[] = {0xFFF00001U, 0xFFF00001U,
        0xFFF00001U, 0xFFF00001U, 0xFFF00002U};
    Fixture fixture;
    u32 test, i;
    for (test = 0; test < 5; test++) {
        setup(&fixture, 8, 512);
        CHECK(dreadzone_payload_allocate_zeroed(&fixture.arena, &fixture.state, requests[test]) == PAYLOAD_BASE);
        CHECK(fixture.words[1] == descriptors[test]);
        for (i = 0; i < 512; i++) CHECK(fixture.arena.payload[i] == (i < footprints[test] ? 0 : 0xA5));
        check_guards(&fixture);
    }
    /* Host backing may be unaligned while the serialized guest address is
     * aligned. Word copies must still preserve the exact four-byte footprint. */
    setup(&fixture, 8, 512);
    fixture.arena.payload++;
    CHECK(dreadzone_payload_allocate_zeroed(&fixture.arena, &fixture.state, 5) == PAYLOAD_BASE);
    CHECK(fixture.bytes[64] == 0xA5 && fixture.bytes[73] == 0xA5);
    for (i = 65; i < 73; i++) CHECK(fixture.bytes[i] == 0);
    check_guards(&fixture);
}

static void check_descriptor_exhaustion(void) {
    Fixture fixture;
    setup(&fixture, 2, 512);
    CHECK(dreadzone_payload_allocate(&fixture.arena, &fixture.state, 64) == PAYLOAD_BASE);
    CHECK(dreadzone_payload_allocate(&fixture.arena, &fixture.state, 64) == PAYLOAD_BASE + 64);
    CHECK(fixture.state.cursor == DESCRIPTOR_BASE + 8);
    allocation_failure(&fixture, 64);
    CHECK(dreadzone_payload_release(&fixture.arena, &fixture.state, PAYLOAD_BASE) == 1);
    CHECK(fixture.state.head == DESCRIPTOR_BASE + 4 && fixture.state.cursor == DESCRIPTOR_BASE);
    CHECK(fixture.words[1] == 0 && fixture.words[2] == 0xFFF00101U);
    CHECK(dreadzone_payload_allocate(&fixture.arena, &fixture.state, 64) == PAYLOAD_BASE);
    CHECK(fixture.words[1] == 0x00100001U);
    CHECK(fixture.state.head == DESCRIPTOR_BASE && fixture.state.tail == DESCRIPTOR_BASE + 4);
    CHECK(fixture.state.cursor == DESCRIPTOR_BASE + 8);
    check_guards(&fixture);
}

static void check_first_fit_and_free(void) {
    Fixture fixture;
    u8 payload[512];
    setup(&fixture, 8, 512);
    fixture.words[1] = 0x00100002U;
    fixture.words[2] = 0x00200202U;
    fixture.words[3] = 0xFFF00402U;
    fixture.state.head = DESCRIPTOR_BASE;
    fixture.state.tail = DESCRIPTOR_BASE + 8;
    fixture.state.cursor = DESCRIPTOR_BASE + 12;
    memcpy(payload, fixture.arena.payload, sizeof(payload));
    CHECK(dreadzone_payload_release(&fixture.arena, &fixture.state, PAYLOAD_BASE + 128) == 1);
    CHECK(fixture.words[1] == 0x00200002U && fixture.words[2] == 0);
    CHECK(fixture.state.cursor == DESCRIPTOR_BASE + 4);
    CHECK(dreadzone_payload_allocate(&fixture.arena, &fixture.state, 65) == PAYLOAD_BASE + 128);
    CHECK(fixture.words[1] == 0x00100002U && fixture.words[2] == 0x00200202U);
    CHECK(fixture.state.cursor == DESCRIPTOR_BASE + 12);
    CHECK(dreadzone_payload_release(&fixture.arena, &fixture.state, PAYLOAD_BASE + 256) == 1);
    CHECK(fixture.words[2] == 0xFFF00202U && fixture.words[3] == 0);
    CHECK(fixture.state.tail == DESCRIPTOR_BASE + 4 && fixture.state.cursor == DESCRIPTOR_BASE + 8);
    CHECK(dreadzone_payload_release(&fixture.arena, &fixture.state, PAYLOAD_BASE) == 1);
    CHECK(fixture.state.head == DESCRIPTOR_BASE + 4 && fixture.state.cursor == DESCRIPTOR_BASE);
    CHECK(dreadzone_payload_allocate(&fixture.arena, &fixture.state, 64) == PAYLOAD_BASE);
    CHECK(fixture.words[1] == 0x00100001U && fixture.words[2] == 0xFFF00202U);
    CHECK(dreadzone_payload_allocate(&fixture.arena, &fixture.state, 64) == PAYLOAD_BASE + 64);
    CHECK(fixture.words[1] == 0x00200001U && fixture.words[3] == 0x00100101U);
    CHECK(fixture.state.cursor == DESCRIPTOR_BASE + 12);
    CHECK(dreadzone_payload_release(&fixture.arena, &fixture.state, PAYLOAD_BASE + 128) == 1);
    CHECK(fixture.state.tail == DESCRIPTOR_BASE + 8 && fixture.words[3] == 0xFFF00101U);
    CHECK(dreadzone_payload_release(&fixture.arena, &fixture.state, PAYLOAD_BASE) == 1);
    CHECK(dreadzone_payload_release(&fixture.arena, &fixture.state, PAYLOAD_BASE + 64) == 1);
    CHECK(fixture.state.head == 0 && fixture.state.tail == 0 && fixture.state.cursor == DESCRIPTOR_BASE);
    CHECK(fixture.words[1] == 0 && fixture.words[2] == 0 && fixture.words[3] == 0);
    CHECK(memcmp(payload, fixture.arena.payload, sizeof(payload)) == 0);
    CHECK(dreadzone_payload_allocate(&fixture.arena, &fixture.state, 64) == PAYLOAD_BASE);
    CHECK(fixture.words[1] == 0xFFF00001U);
    check_guards(&fixture);
}

static void check_cursor_skip(void) {
    Fixture fixture;
    setup(&fixture, 8, 512);
    fixture.words[1] = 0x00200001U;
    fixture.words[3] = 0xFFF00301U;
    fixture.words[4] = 0xABCDEF12U;
    fixture.state.head = DESCRIPTOR_BASE;
    fixture.state.tail = DESCRIPTOR_BASE + 8;
    fixture.state.cursor = DESCRIPTOR_BASE + 4;
    CHECK(dreadzone_payload_allocate(&fixture.arena, &fixture.state, 64) == PAYLOAD_BASE + 64);
    CHECK(fixture.words[1] == 0x00100001U && fixture.words[2] == 0x00200101U);
    CHECK(fixture.words[4] == 0xABCDEF12U && fixture.state.cursor == DESCRIPTOR_BASE + 16);
    check_guards(&fixture);
}

static void check_bounded_failures(void) {
    Fixture fixture;
    setup(&fixture, 8, 512);
    allocation_failure(&fixture, 0);
    allocation_failure(&fixture, 255U * 64U + 1);
    allocation_failure(&fixture, 0xFFFFFFFFU);
    allocation_failure(&fixture, 513);
    release_failure(&fixture, PAYLOAD_BASE);
    CHECK(dreadzone_payload_allocate(&fixture.arena, &fixture.state, 64) == PAYLOAD_BASE);
    release_failure(&fixture, PAYLOAD_BASE + 1);
    release_failure(&fixture, PAYLOAD_BASE + 64);
    release_failure(&fixture, PAYLOAD_BASE - 64);
    release_failure(&fixture, PAYLOAD_BASE + 512);
    CHECK(dreadzone_payload_translate(&fixture.arena, PAYLOAD_BASE + 511, 1) == fixture.arena.payload + 511);
    CHECK(dreadzone_payload_translate(&fixture.arena, PAYLOAD_BASE + 512, 0) == fixture.arena.payload + 512);
    CHECK(dreadzone_payload_translate(&fixture.arena, PAYLOAD_BASE + 512, 1) == NULL);
    CHECK(dreadzone_payload_translate(&fixture.arena, 0xFFFFFFFFU, 2) == NULL);
    fixture.words[1] = 0x00100001U;
    fixture.words[2] = 0x00000101U;
    allocation_failure(&fixture, 64); /* Cycle, not a signed terminal link. */
    release_failure(&fixture, PAYLOAD_BASE);
    fixture.words[1] = 0x00700001U;
    fixture.words[2] = 0;
    allocation_failure(&fixture, 64); /* Link reaches an empty descriptor. */
    fixture.words[1] = 0xFFF00901U;
    allocation_failure(&fixture, 64); /* Payload offset outside the arena. */
    fixture.words[1] = 0xFFF00001U;
    fixture.state.tail = DESCRIPTOR_BASE + 4;
    allocation_failure(&fixture, 64);
    fixture.state.tail = DESCRIPTOR_BASE;
    fixture.state.cursor = DESCRIPTOR_BASE + 1;
    allocation_failure(&fixture, 64);
    fixture.state.cursor = DESCRIPTOR_BASE + 4;
    fixture.arena.descriptor_base = 0xFFFFFFFCU;
    allocation_failure(&fixture, 64);
    CHECK(dreadzone_payload_allocate(NULL, &fixture.state, 64) == 0);
    CHECK(dreadzone_payload_allocate(&fixture.arena, NULL, 64) == 0);
    CHECK(dreadzone_payload_translate(NULL, PAYLOAD_BASE, 1) == NULL);
    check_guards(&fixture);
}

int main(void) {
    CHECK(sizeof(u32) == 4 && sizeof(DreadzonePayloadState) == 12);
    check_rounding_and_clear();
    check_word_clear_footprints();
    check_descriptor_exhaustion();
    check_first_fit_and_free();
    check_cursor_skip();
    check_bounded_failures();
    puts("Dreadzone payload allocation, release and guest bounds checks passed.");
    return EXIT_SUCCESS;
}
