#include "dreadzone_actor_ground_step.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { BASE = 0x368F60, SIZE = 0x31F0 };
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "Captured side-offset failure at line %d\n", __LINE__); free(bytes); free(before); return 1; } } while (0)
static void real(u8 *bytes, u32 offset, f32 value) { memcpy(bytes + offset, &value, 4); }
int main(void) {
    u8 *bytes = (u8 *)calloc(SIZE, 1), *before = (u8 *)malloc(SIZE);
    DreadzoneGroundRequest q = {0}; DreadzoneGroundInfo info, info_before;
    u32 quad[4] = {0x7FC00000U, 1, 0xFFFFFFFFU, 0x40000000U};
    u32 previous[4], output[4]; f32 x, y, z, w; unsigned i;
    if (!bytes || !before) { free(bytes); free(before); return 1; }
    for (i = 0; i < 4; ++i) real(bytes, 0x20 + i * 20, 1.f);
    real(bytes, 0xA0, 10.f); real(bytes, 0xA4, 20.f); real(bytes, 0xA8, 30.f);
    memcpy(before, bytes, SIZE);
    q.storage = bytes; q.guest_base = BASE; q.storage_bytes = SIZE; q.record_guest = BASE;
    q.operation = DREADZONE_GROUND_SIDE_OFFSET; q.integer0 = 1;
    q.scalar[0] = .5f; q.scalar[1] = 0.f; q.scalar[2] = .7f;
    for (i = 0; i < 5; ++i) q.incoming_vf25_29_w[i] = 0xA0000000U + i;
    CHECK(dreadzone_side_offset_captured_quad(&q, quad, quad, &info) == 1);
    memcpy(&x, quad, 4); memcpy(&y, quad + 1, 4); memcpy(&z, quad + 2, 4); memcpy(&w, quad + 3, 4);
    CHECK(x == 10.5f && y == 20.f && fabsf(z - 30.7f) < 0.00001f && w == 2.f);
    CHECK(memcmp(bytes, before, SIZE) == 0);
    CHECK(memcmp(info.outgoing_vf25_29_w, q.incoming_vf25_29_w, 20) == 0);
    CHECK(!info.has_integer_return && !info.has_float_return && !info.probe_calls);
    memcpy(previous, quad, 16); info_before = info;
    q.scalar[1] = NAN;
    CHECK(dreadzone_side_offset_captured_quad(&q, quad, quad, &info) == -3);
    CHECK(memcmp(quad, previous, 16) == 0 && memcmp(&info, &info_before, sizeof(info)) == 0);
    q.scalar[1] = 0.f; bytes[0x2659] = 1;
    CHECK(dreadzone_side_offset_captured_quad(&q, quad, quad, &info) == -2);
    CHECK(memcmp(quad, previous, 16) == 0 && memcmp(&info, &info_before, sizeof(info)) == 0);
    bytes[0x2659] = 0;
    CHECK(dreadzone_side_offset_captured_quad(&q, quad, (u32 *)(void *)(bytes + 0x20), &info) == -1);
    CHECK(memcmp(bytes, before, SIZE) == 0);
    q.storage_bytes = 0x31EF;
    CHECK(dreadzone_side_offset_captured_quad(&q, quad, output, &info) == -1);
    CHECK(memcmp(&info, &info_before, sizeof(info)) == 0);
    free(bytes); free(before);
    puts("Captured side-offset aliasing, preserved RAM and refusals passed");
    return 0;
}
