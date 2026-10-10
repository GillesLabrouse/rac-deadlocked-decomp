#include "dreadzone_geometry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { HEADER = 0x00700000, PACKET = 0x00710000, TEXTURE = 0x00720100,
       REMAP = 0x00730000, TABLE_BACKING = 0x00269800, GUARD = 17 };
typedef struct Fixture {
    u8 headers[0x80 + 2 * GUARD], packets[0x800 + 2 * GUARD];
    u8 textures[0x1200 + 2 * GUARD], remap[0x100 + 2 * GUARD];
    u8 table[0x2000 + 2 * GUARD], config[0x48 + 2 * GUARD];
    DreadzoneGeometryView views[6];
    DreadzoneGeometryContext context;
} Fixture;
static unsigned checks;
static void check(int condition, const char *message) {
    checks++;
    if (!condition) { fprintf(stderr, "Geometry check failed: %s\n", message); exit(EXIT_FAILURE); }
}
static void put(u8 *p, u64 value, u32 bytes) {
    u32 i;
    for (i = 0; i < bytes; i++) p[i] = (u8)(value >> (i * 8));
}
static u64 get(const u8 *p, u32 bytes) {
    u64 value = 0;
    u32 i;
    for (i = 0; i < bytes; i++) value |= (u64)p[i] << (i * 8);
    return value;
}
static void compare_bytes(const u8 *actual, const u8 *expected, size_t bytes) {
    size_t i;
    for (i = 0; i < bytes; i++) check(actual[i] == expected[i], "every output, untouched field and guard byte matches");
}
static void compare_fixture(const Fixture *actual, const Fixture *expected) {
    compare_bytes(actual->headers, expected->headers, sizeof(actual->headers));
    compare_bytes(actual->packets, expected->packets, sizeof(actual->packets));
    compare_bytes(actual->textures, expected->textures, sizeof(actual->textures));
    compare_bytes(actual->remap, expected->remap, sizeof(actual->remap));
    compare_bytes(actual->table, expected->table, sizeof(actual->table));
    compare_bytes(actual->config, expected->config, sizeof(actual->config));
}
static void copy_storage(Fixture *destination, const Fixture *source) {
    memcpy(destination->headers, source->headers, sizeof(source->headers));
    memcpy(destination->packets, source->packets, sizeof(source->packets));
    memcpy(destination->textures, source->textures, sizeof(source->textures));
    memcpy(destination->remap, source->remap, sizeof(source->remap));
    memcpy(destination->table, source->table, sizeof(source->table));
    memcpy(destination->config, source->config, sizeof(source->config));
}
static void setup(Fixture *f, s32 index, s32 high, u32 low, s16 format) {
    u32 i;
    memset(f, 0xB7, sizeof(*f));
    f->views[0].guest_base = HEADER; f->views[0].bytes = 0x80; f->views[0].storage = f->headers + GUARD;
    f->views[1].guest_base = PACKET; f->views[1].bytes = 0x800; f->views[1].storage = f->packets + GUARD;
    f->views[2].guest_base = TEXTURE - 0x100; f->views[2].bytes = 0x1200; f->views[2].storage = f->textures + GUARD;
    f->views[3].guest_base = REMAP; f->views[3].bytes = 0x100; f->views[3].storage = f->remap + GUARD;
    f->views[4].guest_base = TABLE_BACKING; f->views[4].bytes = 0x2000; f->views[4].storage = f->table + GUARD;
    f->views[5].guest_base = DREADZONE_GEOMETRY_CONFIG; f->views[5].bytes = 0x48; f->views[5].storage = f->config + GUARD;
    f->context.views = f->views; f->context.view_count = 6;
    put(f->headers + GUARD, PACKET - (low - (u32)high) * 16, 4);
    put(f->headers + GUARD + 4, ((u32)high << 16) | low, 4);
    for (i = 0; i < 0x800; i += 64) {
        put(f->packets + GUARD + i, 0x12345678, 4);
        put(f->packets + GUARD + i + 4, 2, 4);
        put(f->packets + GUARD + i + 0x10, 0x35, 4);
        put(f->packets + GUARD + i + 0x14, 3, 4);
        put(f->packets + GUARD + i + 0x20, (u32)index, 4);
    }
    memset(f->remap + GUARD, 3, 0x100);
    put(f->config + GUARD + 0x10, 0x24680, 4);
    put(f->config + GUARD + 0x2C, 0x4000, 4);
    put(f->config + GUARD + 0x44, 0x12340000, 4);
    for (i = 0; i < 0x1200; i += 16) {
        u8 *record = f->textures + GUARD + i;
        put(record, 0x20, 4); put(record + 4, 128, 2); put(record + 6, 256, 2);
        put(record + 8, (u16)format, 2); put(record + 0xA, 0x10, 2);
        put(record + 0xC, 0x20, 2); put(record + 0xE, 0x30, 2);
    }
    put(f->table + GUARD + DREADZONE_GEOMETRY_TABLE - TABLE_BACKING + 3 * 24, 0x1122334455667788ULL, 8);
    put(f->table + GUARD + DREADZONE_GEOMETRY_TABLE - TABLE_BACKING + 3 * 24 + 8, 0xFFEEDDCCBBAA001CULL, 8);
    put(f->table + GUARD + DREADZONE_GEOMETRY_TABLE - TABLE_BACKING + 3 * 24 + 16, 0x99AABBCCDDEEFF00ULL, 8);
    put(f->table + GUARD + DREADZONE_GEOMETRY_FALLBACK - TABLE_BACKING, 0xDEEDBEAF55113377ULL, 8);
    put(f->table + GUARD + DREADZONE_GEOMETRY_FALLBACK - TABLE_BACKING + 16, 0xCAFEBABEFF001122ULL, 8);
    put(f->table + GUARD + DREADZONE_GEOMETRY_FALLBACK_MINUS_THREE - TABLE_BACKING, 0xABCDEF0123456789ULL, 8);
    put(f->table + GUARD + DREADZONE_GEOMETRY_FALLBACK_MINUS_THREE - TABLE_BACKING + 16, 0x109876543210FEDCULL, 8);
}
static void expected_packet(u64 words[4], int textured, s32 index, s16 format) {
    words[0] = 0x12345678000000A0ULL;
    words[1] = 0x0300003D;
    if (index >= 0 && !textured) {
        words[0] |= 0x1C;
        words[2] = 0x1122334455667788ULL; words[3] = 0x99AABBCCDDEEFF00ULL;
    } else if (index >= 0 && format == 0) {
        words[2] = 0x80000A061DB2B400ULL; words[3] = 0x0040000400004000ULL;
    } else if (index >= 0) {
        words[0] |= (u64)(s64)(format - 1) << 2;
        words[2] = 0x80000A061D308000ULL; words[3] = 0x0040700406004000ULL;
    } else {
        words[1] = 5;
        if (index == -3) { words[2] = 0xABCDEF0123456789ULL; words[3] = 0x109876543210FEDCULL; }
        else if (index < -1) { words[2] = 0xDEEDBEAF55113377ULL; words[3] = 0xCAFEBABEFF001122ULL; }
        else { words[2] = 0x80000004CC004246ULL; words[3] = 0; }
    }
}
static void test_packets(void) {
    const s32 indices[] = {0, 255, -1, -2, -3, -7};
    const s32 highs[] = {-32768, -1, 0, 1, 3, 4, 7, 8, 16};
    const s16 formats[] = {0, 1, 2, -1};
    u32 a, b, c, d;
    for (a = 0; a < 2; a++) for (b = 0; b < sizeof(indices) / sizeof(indices[0]); b++) {
        for (c = 0; c < sizeof(highs) / sizeof(highs[0]); c++) for (d = 0; d < sizeof(formats) / sizeof(formats[0]); d++) {
            Fixture actual, expected;
            u64 words[4];
            s32 quad;
            setup(&actual, indices[b], highs[c], 0xFFFF, formats[d]);
            expected = actual;
            put(expected.headers + GUARD + 4, 0xFFFF, 4);
            expected_packet(words, (int)a, indices[b], formats[d]);
            for (quad = 0; quad < highs[c]; quad += 4) {
                u32 j;
                for (j = 0; j < 4; j++) put(expected.packets + GUARD + quad * 16 + j * 16, words[j], 8);
            }
            check(dreadzone_geometry_patch(&actual.context, HEADER, a ? TEXTURE : 0, REMAP, 1) == 0, "all real signed/packed header and texture branches execute");
            compare_fixture(&actual, &expected);
        }
    }
}

static void test_prepare_and_apply(void) {
    Fixture original, expected, alternate;
    DreadzoneGeometryPlan *plan = NULL;
    DreadzoneGeometryView bad_views[6];
    DreadzoneGeometryContext bad_context;
    u64 packet[4], i;
    setup(&original, 0, 4, 4, 0); expected = original;
    expected_packet(packet, 0, 0, 0);
    put(expected.headers + GUARD + 4, 4, 4);
    for (i = 0; i < 4; i++) put(expected.packets + GUARD + i * 16, packet[i], 8);
    check(dreadzone_geometry_prepare(&original.context, HEADER, 0, REMAP, 1, &plan) == 0 && plan != NULL, "complete plan prepares without writes");
    setup(&alternate, 0, 4, 4, 0);
    compare_fixture(&original, &alternate);
    check(dreadzone_geometry_write_count(plan) == 5, "one masked header and four quad words are planned");
    for (i = 0; i < 5; i++) {
        const u32 expected_addresses[] = {HEADER + 4, PACKET, PACKET + 0x10, PACKET + 0x30, PACKET + 0x20};
        u32 address, bytes; u64 value;
        check(dreadzone_geometry_write_at(plan, i, &address, &bytes, &value) == 1, "every write can join the enclosing class journal");
        check(address == expected_addresses[i] && bytes == (i == 0 ? 4U : 8U), "the exported write order matches actual SD instructions");
        check(value == get(expected.headers + GUARD + 4, 4) || i != 0, "masked header value is exported");
    }
    memcpy(bad_views, original.views, sizeof(bad_views)); bad_views[1].bytes = 0x20;
    bad_context.views = bad_views; bad_context.view_count = 6;
    check(dreadzone_geometry_apply(plan, &bad_context) == DREADZONE_GEOMETRY_UNMAPPED, "missing final target rejects apply before the first write");
    compare_fixture(&original, &alternate);
    check(dreadzone_geometry_apply(plan, &alternate.context) == 0, "a plan applies to equivalent guest backing without retained host targets");
    compare_fixture(&alternate, &expected);
    setup(&expected, 0, 4, 4, 0); compare_fixture(&original, &expected);
    check(dreadzone_geometry_apply(plan, &original.context) == DREADZONE_GEOMETRY_INVALID, "an applied plan cannot be replayed");
    compare_fixture(&original, &expected);
    dreadzone_geometry_destroy(plan);
    plan = NULL;
    check(dreadzone_geometry_prepare(&original.context, HEADER, 0, REMAP, 1, &plan) == 0, "an unapplied plan may be abandoned");
    dreadzone_geometry_destroy(plan); compare_fixture(&original, &expected);
}

typedef struct ReaderFixture { Fixture *fixture; u32 calls, fail_address; } ReaderFixture;
static int transaction_read(void *user, u32 address, u32 bytes, u64 *value) {
    ReaderFixture *reader = (ReaderFixture *)user;
    u32 i;
    reader->calls++;
    if (address == reader->fail_address) return 0;
    /* These two values represent preceding, unapplied class relocation writes. */
    if (address == HEADER && bytes == 4) { *value = PACKET; return 1; }
    if (address == HEADER + 4 && bytes == 4) { *value = 0x00040004; return 1; }
    for (i = 0; i < 6; i++) {
        DreadzoneGeometryView *view = reader->fixture->views + i;
        if (address >= view->guest_base && (u64)address + bytes <= (u64)view->guest_base + view->bytes) {
            *value = get(view->storage + (address - view->guest_base), bytes); return 1;
        }
    }
    return 0;
}
static void test_transaction_reader(void) {
    Fixture actual, expected;
    ReaderFixture reader;
    DreadzoneGeometryPlan *plan;
    u64 words[4];
    u32 i;
    setup(&actual, 0, 4, 4, 0);
    put(actual.headers + GUARD, 0x1000, 4); put(actual.headers + GUARD + 4, 0xEEEEEEEE, 4);
    expected = actual;
    reader.fixture = &actual; reader.calls = 0; reader.fail_address = 0;
    check(dreadzone_geometry_prepare_with_reader(&actual.context, transaction_read, &reader, HEADER, 0, REMAP, 1, &plan) == 0,
          "geometry sees preceding class relocation journal values");
    check(reader.calls > 0 && dreadzone_geometry_write_count(plan) == 5, "all reads still use validated guest ranges");
    compare_fixture(&actual, &expected);
    put(expected.headers + GUARD + 4, 4, 4);
    expected_packet(words, 0, 0, 0);
    for (i = 0; i < 4; i++) put(expected.packets + GUARD + i * 16, words[i], 8);
    check(dreadzone_geometry_apply(plan, &actual.context) == 0, "class relocation values and geometry writes remain separate until commit");
    compare_fixture(&actual, &expected); dreadzone_geometry_destroy(plan);
    setup(&actual, 0, 4, 4, 0); expected = actual;
    reader.calls = 0; reader.fail_address = DREADZONE_GEOMETRY_TABLE + 3 * 24 + 16;
    check(dreadzone_geometry_prepare_with_reader(&actual.context, transaction_read, &reader, HEADER, 0, REMAP, 1, &plan) == DREADZONE_GEOMETRY_UNMAPPED && plan == NULL,
          "a transaction read refusal rejects the whole preparation");
    compare_fixture(&actual, &expected);
}

static void test_atomic_failures(void) {
    Fixture actual, expected;
    DreadzoneGeometryPlan *plan;
    setup(&actual, -3, 4, 4, 0); expected = actual;
    actual.views[4].storage += DREADZONE_GEOMETRY_TABLE - TABLE_BACKING;
    actual.views[4].bytes -= DREADZONE_GEOMETRY_TABLE - TABLE_BACKING;
    actual.views[4].guest_base = DREADZONE_GEOMETRY_TABLE;
    check(dreadzone_geometry_patch(&actual.context, HEADER, 0, REMAP, 1) == DREADZONE_GEOMETRY_UNMAPPED,
          "negative table index still requires the three unconditional retail loads");
    compare_fixture(&actual, &expected);
    setup(&actual, -1, 4, 4, 0); expected = actual;
    actual.views[2].storage += 0x100; actual.views[2].bytes -= 0x100; actual.views[2].guest_base = TEXTURE;
    check(dreadzone_geometry_patch(&actual.context, HEADER, TEXTURE, REMAP, 1) == DREADZONE_GEOMETRY_UNMAPPED,
          "negative texture index still requires the original signed texture reads");
    compare_fixture(&actual, &expected);
    setup(&actual, -2, 4, 4, 0); expected = actual;
    actual.views[4].bytes = DREADZONE_GEOMETRY_FALLBACK + 16 - TABLE_BACKING;
    check(dreadzone_geometry_patch(&actual.context, HEADER, 0, REMAP, 1) == DREADZONE_GEOMETRY_UNMAPPED,
          "a missing last fallback word cannot partially update the packet or header");
    compare_fixture(&actual, &expected);
    setup(&actual, 0, 4, 4, 0); expected = actual;
    actual.views[1].bytes = 0x30;
    check(dreadzone_geometry_patch(&actual.context, HEADER, 0, REMAP, 1) == DREADZONE_GEOMETRY_UNMAPPED,
          "the last quad lower word must fit, even when all earlier writes are mapped");
    compare_fixture(&actual, &expected);
    setup(&actual, 256, 4, 4, 0); expected = actual;
    check(dreadzone_geometry_patch(&actual.context, HEADER, 0, REMAP, 1) == DREADZONE_GEOMETRY_UNMAPPED, "remap end boundary is checked");
    compare_fixture(&actual, &expected);
    setup(&actual, 0, 4, 4, 0); expected = actual;
    put(actual.headers + GUARD + 16, 0x00DE0000, 4); put(actual.headers + GUARD + 20, 0x00040004, 4); expected = actual;
    check(dreadzone_geometry_patch(&actual.context, HEADER, 0, REMAP, 2) == DREADZONE_GEOMETRY_UNMAPPED,
          "a later malformed header rejects the earlier valid header and packet");
    compare_fixture(&actual, &expected);
    setup(&actual, 0, 4, 4, 0); expected = actual;
    actual.views[0].guest_base = 0xFFFFFFF0U; actual.views[0].bytes = 17;
    check(dreadzone_geometry_prepare(&actual.context, HEADER, 0, REMAP, 1, &plan) == DREADZONE_GEOMETRY_INVALID && plan == NULL,
          "guest view overflow cannot escape the mapped backing");
    compare_fixture(&actual, &expected);
    setup(&actual, 0, 4, 4, 0); expected = actual;
    actual.views[1].storage = actual.views[0].storage;
    check(dreadzone_geometry_patch(&actual.context, HEADER, 0, REMAP, 1) == DREADZONE_GEOMETRY_INVALID,
          "distinct guest views cannot secretly alias their host buffers");
    compare_fixture(&actual, &expected);
    setup(&actual, 0, 4, 4, 0); expected = actual;
    put(actual.headers + GUARD, PACKET + 1, 4); expected = actual;
    check(dreadzone_geometry_patch(&actual.context, HEADER, 0, REMAP, 1) == DREADZONE_GEOMETRY_INVALID,
          "unaligned guest LW/SD addresses reject the whole plan; host alignment is independent");
    compare_fixture(&actual, &expected);
}

static void test_conditional_reads(void) {
    Fixture actual, expected;
    u64 words[4]; u32 i;
    setup(&actual, 0, 4, 4, 1); expected = actual; actual.views[5].bytes = 0x30;
    expected_packet(words, 1, 0, 1); put(expected.headers + GUARD + 4, 4, 4);
    for (i = 0; i < 4; i++) put(expected.packets + GUARD + i * 16, words[i], 8);
    check(dreadzone_geometry_patch(&actual.context, HEADER, TEXTURE, REMAP, 1) == 0,
          "nonzero format does not read configuration field 0x44");
    compare_fixture(&actual, &expected);
    setup(&actual, 0, 4, 4, 0); expected = actual; actual.views[5].bytes = 0x30;
    check(dreadzone_geometry_patch(&actual.context, HEADER, TEXTURE, REMAP, 1) == DREADZONE_GEOMETRY_UNMAPPED,
          "zero format does read configuration field 0x44");
    compare_fixture(&actual, &expected);
    setup(&actual, 0, 4, 4, 0); expected = actual; actual.context.view_count = 5;
    expected_packet(words, 0, 0, 0); put(expected.headers + GUARD + 4, 4, 4);
    for (i = 0; i < 4; i++) put(expected.packets + GUARD + i * 16, words[i], 8);
    check(dreadzone_geometry_patch(&actual.context, HEADER, 0, REMAP, 1) == 0, "positive precomputed-table path needs no config mapping");
    compare_fixture(&actual, &expected);
    setup(&actual, 0, 4, 4, 0); expected = actual; actual.views[1].bytes = 0x38;
    expected_packet(words, 0, 0, 0); put(expected.headers + GUARD + 4, 4, 4);
    for (i = 0; i < 4; i++) put(expected.packets + GUARD + i * 16, words[i], 8);
    check(dreadzone_geometry_patch(&actual.context, HEADER, 0, REMAP, 1) == 0,
          "the true footprint stops after the lower word of the last quad");
    compare_fixture(&actual, &expected);
    setup(&actual, -1, 4, 4, 0); expected = actual; actual.context.view_count = 5;
    check(dreadzone_geometry_patch(&actual.context, HEADER, 0, REMAP, 1) == DREADZONE_GEOMETRY_UNMAPPED,
          "minus-one table path does read the config fallback word");
    compare_fixture(&actual, &expected);
    check(dreadzone_geometry_patch(NULL, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0) == 0,
          "zero count is a no-op without any guest read");
    check(dreadzone_geometry_patch(NULL, 0, 0, 0, (-2147483647 - 1)) == 0, "negative count is also a no-op");
}

int main(void) {
    test_packets();
    test_prepare_and_apply();
    test_transaction_reader();
    test_atomic_failures();
    test_conditional_reads();
    printf("Dreadzone geometry: %u checks passed\n", checks);
    return EXIT_SUCCESS;
}
