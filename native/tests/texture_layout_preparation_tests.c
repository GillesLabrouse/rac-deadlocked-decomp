#include "../texture_layout_preparation.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned checks, failures;
#define CHECK(condition) do { checks++; if (!(condition)) { failures++; fprintf(stderr, "line %d: %s\n", __LINE__, #condition); } } while (0)
typedef union PixelStorage { u64 alignment; u8 bytes[65536 + 64]; } PixelStorage;
typedef union PacketStorage { u64 alignment; u8 bytes[192]; } PacketStorage;
typedef struct Fixture {
    PixelStorage pixels, scratch;
    PacketStorage upload, readback;
} Fixture;
static Fixture fixture, saved;
static u8 expected_pixels[65536], expected_scratch[65536];

static u64 word(const u8 *p) {
    u64 value = 0;
    u32 i;
    for (i = 0; i < 8; i++) value |= (u64)p[i] << (i * 8);
    return value;
}

static TextureLayoutPreparation seed(u32 width, u32 height) {
    u32 i, count = width * height;
    TextureLayoutPreparation r;
    memset(&fixture, 0xA7, sizeof(fixture));
    for (i = 0; i < count; i++) fixture.pixels.bytes[7 + i] = (u8)(i ^ (i >> 8) ^ (i >> 16));
    r.pixels.guest_address = 0x00400000;
    r.pixels.bytes = count + 9;
    r.pixels.storage = fixture.pixels.bytes + 7;
    r.upload_packet.guest_address = 0x00800000;
    r.upload_packet.bytes = 96 + 13;
    r.upload_packet.storage = fixture.upload.bytes + 16;
    r.readback_packet.guest_address = 0x00800100;
    r.readback_packet.bytes = 112 + 15;
    r.readback_packet.storage = fixture.readback.bytes + 11;
    r.scratch = fixture.scratch.bytes + 16;
    r.scratch_bytes = count + 17;
    return r;
}

static void successful(u32 width, u32 height, u32 descriptor_location, s32 ignored_format) {
    u32 i, count = width * height;
    TextureLayoutPreparation r = seed(width, height), *borrowed = &r;
    TexturePixelBuffer expected = {r.pixels.guest_address, count, expected_pixels};
    if (descriptor_location == 1) {
        borrowed = (TextureLayoutPreparation *)(fixture.scratch.bytes + 16);
        memcpy(borrowed, &r, sizeof(r));
    } else if (descriptor_location == 2) {
        borrowed = (TextureLayoutPreparation *)(fixture.upload.bytes + 16);
        memcpy(borrowed, &r, sizeof(r));
    } else if (descriptor_location == 3) {
        borrowed = (TextureLayoutPreparation *)(fixture.pixels.bytes + 16);
        memcpy(borrowed, &r, sizeof(r));
    }
    memcpy(expected_pixels, r.pixels.storage, count);
    CHECK(texture_pixel_convert_t8_to_ct32(&expected, width, height, expected_scratch, count) == TEXTURE_PIXEL_CONVERTED);
    saved = fixture;
    CHECK(texture_layout_prepare_cpu_effect(borrowed, ignored_format, (s32)width, (s32)height) == TEXTURE_LAYOUT_CPU_EFFECT_PREPARED);
    CHECK(memcmp(r.pixels.storage, expected_pixels, count) == 0);
    CHECK(memcmp(r.scratch, expected_pixels, count) == 0);
    CHECK(word(r.upload_packet.storage) == 0x1000000000000004ULL);
    CHECK(word(r.upload_packet.storage + 0x10) == ((u64)0x13 << 56 | (u64)(width >> 6) << 48 | (u64)((0x400000 - count) >> 8) << 32));
    CHECK(word(r.upload_packet.storage + 0x30) == ((u64)height << 32 | width));
    CHECK(word(r.upload_packet.storage + 0x50) == (0x0800000000008000ULL | (count >> 4)));
    CHECK(word(r.readback_packet.storage + 0x20) == ((u64)(width >> 7) << 16 | ((0x400000 - count) >> 8)));
    CHECK(word(r.readback_packet.storage + 0x40) == ((u64)(height / 2) << 32 | (width / 2)));
    CHECK(word(r.readback_packet.storage + 0x60) == 1);
    for (i = 0; i < sizeof(fixture.pixels.bytes); i++)
        if (i < 7 || i >= 7 + count) CHECK(fixture.pixels.bytes[i] == saved.pixels.bytes[i]);
    for (i = 0; i < sizeof(fixture.scratch.bytes); i++)
        if (i < 16 || i >= 16 + count) CHECK(fixture.scratch.bytes[i] == saved.scratch.bytes[i]);
    for (i = 0; i < sizeof(fixture.upload.bytes); i++)
        if (i < 16 || i >= 16 + 96) CHECK(fixture.upload.bytes[i] == saved.upload.bytes[i]);
    for (i = 0; i < sizeof(fixture.readback.bytes); i++)
        if (i < 11 || i >= 11 + 112) CHECK(fixture.readback.bytes[i] == saved.readback.bytes[i]);
}

static void invalid_buffers(void) {
    u32 i;
    for (i = 0; i < 18; i++) {
        TextureLayoutPreparation r = seed(128, 64);
        switch (i) {
        case 0: r.pixels.storage = NULL; break;
        case 1: r.scratch = NULL; break;
        case 2: r.pixels.bytes = 8191; break;
        case 3: r.scratch_bytes = 8191; break;
        case 4: r.upload_packet.bytes = 95; break;
        case 5: r.readback_packet.bytes = 111; break;
        case 6: r.scratch = r.pixels.storage; break;
        case 7: r.upload_packet.storage = r.pixels.storage; break;
        case 8: r.readback_packet.storage = r.scratch; break;
        case 9: r.readback_packet.storage = r.upload_packet.storage + 1; break;
        case 10: r.pixels.guest_address = r.upload_packet.guest_address; break;
        case 11: r.readback_packet.guest_address = r.upload_packet.guest_address + 16; break;
        case 12: r.upload_packet.guest_address += 1; break;
        case 13: r.pixels.guest_address = 0xffffe000U; break;
        case 14: r.readback_packet.guest_address = 0xfffffff0U; break;
        case 15: r.pixels.storage = (u8 *)(UINTPTR_MAX - 8191); break;
        case 16: r.scratch = (u8 *)(UINTPTR_MAX - 8191); break;
        case 17: r.upload_packet.storage = (u8 *)(UINTPTR_MAX - 95); break;
        }
        saved = fixture;
        CHECK(texture_layout_prepare_cpu_effect(&r, 0x13, 128, 64) == TEXTURE_LAYOUT_INVALID_BUFFERS);
        CHECK(memcmp(&fixture, &saved, sizeof(fixture)) == 0);
    }
    {
        TextureLayoutPreparation r = seed(128, 64);
        static const s32 shapes[][2] = {{-128, 64}, {128, -64}, {0, 64}, {64, 64}, {128, 32}, {128, 65}, {2048, 256}, {4096, 64}, {0x10080, 64}};
        saved = fixture;
        for (i = 0; i < sizeof(shapes) / sizeof(shapes[0]); i++) {
            CHECK(texture_layout_prepare_cpu_effect(&r, 0, shapes[i][0], shapes[i][1]) == TEXTURE_LAYOUT_UNSUPPORTED_RECTANGLE);
            CHECK(memcmp(&fixture, &saved, sizeof(fixture)) == 0);
        }
        CHECK(texture_layout_prepare_cpu_effect(NULL, 0, 128, 64) == TEXTURE_LAYOUT_INVALID_BUFFERS);
    }
}

int main(void) {
    successful(256, 256, 0, 0x13);
    successful(128, 128, 0, -1);
    successful(128, 64, 1, 0x7fffffff);
    successful(128, 64, 2, 0);
    successful(128, 64, 3, 0x10013);
    invalid_buffers();
    printf("%u checks, %u failures\n", checks, failures);
    return failures != 0;
}
