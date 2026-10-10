#include "../texture_pixel_permutation.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks, failures;
#define CHECK(condition) do { checks++; if (!(condition)) { failures++; fprintf(stderr, "line %d: %s\n", __LINE__, #condition); } } while (0)

/* Authored single-column coordinate markers. These are the four components
 * read at each CT32 pixel after a T8 16-by-4 column is uploaded. Source values
 * name T8(x,y) as x + y * 16; there are no game pixels or instructions here. */
static const u8 column_markers[64] = {
     0, 36,  8, 44,  1, 37,  9, 45,  2, 38, 10, 46,  3, 39, 11, 47,
     4, 32, 12, 40,  5, 33, 13, 41,  6, 34, 14, 42,  7, 35, 15, 43,
    16, 52, 24, 60, 17, 53, 25, 61, 18, 54, 26, 62, 19, 55, 27, 63,
    20, 48, 28, 56, 21, 49, 29, 57, 22, 50, 30, 58, 23, 51, 31, 59
};

static void column_and_guards(u32 alignment) {
    u8 pixels[8192 + 64], scratch[8192 + 64];
    u32 x, y, i, changed = 0;
    unsigned before[256] = {0}, after[256] = {0};
    TexturePixelBuffer view = {0x00400000, 8192 + 9, pixels + alignment};
    memset(pixels, 0xD7, sizeof(pixels));
    memset(scratch, 0xB5, sizeof(scratch));
    for (y = 0; y < 64; y++)
        for (x = 0; x < 128; x++) {
            u8 value = (u8)((x & 15) + (y & 15) * 16);
            pixels[alignment + y * 128 + x] = value;
            before[value]++;
        }
    CHECK(texture_pixel_convert_t8_to_ct32(&view, 128, 64, scratch + alignment, 8192 + 11) == TEXTURE_PIXEL_CONVERTED);
    for (i = 0; i < 64; i++) {
        u32 offset = (i / 32) * 256 + i % 32;
        CHECK(view.storage[offset] == column_markers[i]);
    }
    for (i = 0; i < 8192; i++) {
        CHECK(view.storage[i] == scratch[alignment + i]);
        after[view.storage[i]]++;
        changed += view.storage[i] != (u8)(((i % 128) & 15) + ((i / 128) & 15) * 16);
    }
    CHECK(changed != 0);
    for (i = 0; i < 256; i++) CHECK(before[i] == after[i]);
    for (i = 0; i < sizeof(pixels); i++) {
        if (i < alignment || i >= alignment + 8192) CHECK(pixels[i] == 0xD7);
        if (i < alignment || i >= alignment + 8192) CHECK(scratch[i] == 0xB5);
    }
}

static void refusal(void) {
    u8 pixels[8192 + 32], scratch[8192 + 32], saved_pixels[sizeof(pixels)], saved_scratch[sizeof(scratch)];
    TexturePixelBuffer view = {0x00400000, 8192, pixels};
    u32 i;
    static const u32 dimensions[][2] = {{0, 64}, {128, 0}, {64, 64}, {128, 32}, {129, 64}, {128, 65}, {4096, 64}, {128, 4096}, {2048, 256}, {0xffffffffU, 64}, {128, 0xffffffffU}};
    memset(pixels, 0x8A, sizeof(pixels));
    memset(scratch, 0xE2, sizeof(scratch));
    memcpy(saved_pixels, pixels, sizeof(pixels));
    memcpy(saved_scratch, scratch, sizeof(scratch));
    for (i = 0; i < sizeof(dimensions) / sizeof(dimensions[0]); i++)
        CHECK(texture_pixel_convert_t8_to_ct32(&view, dimensions[i][0], dimensions[i][1], scratch, sizeof(scratch)) == TEXTURE_PIXEL_UNSUPPORTED_RECTANGLE);
    CHECK(texture_pixel_convert_t8_to_ct32(NULL, 128, 64, scratch, sizeof(scratch)) == TEXTURE_PIXEL_INVALID_BUFFER);
    CHECK(texture_pixel_convert_t8_to_ct32(&view, 128, 64, NULL, sizeof(scratch)) == TEXTURE_PIXEL_INVALID_BUFFER);
    CHECK(texture_pixel_convert_t8_to_ct32(&view, 128, 64, scratch, 8191) == TEXTURE_PIXEL_INVALID_BUFFER);
    CHECK(texture_pixel_convert_t8_to_ct32(&view, 128, 64, pixels, 8192) == TEXTURE_PIXEL_INVALID_BUFFER);
    CHECK(texture_pixel_convert_t8_to_ct32(&view, 128, 64, pixels + 1, 8192) == TEXTURE_PIXEL_INVALID_BUFFER);
    view.bytes = 8191;
    CHECK(texture_pixel_convert_t8_to_ct32(&view, 128, 64, scratch, sizeof(scratch)) == TEXTURE_PIXEL_INVALID_BUFFER);
    view.bytes = 8192;
    view.guest_address = 0x00400001;
    CHECK(texture_pixel_convert_t8_to_ct32(&view, 128, 64, scratch, sizeof(scratch)) == TEXTURE_PIXEL_INVALID_BUFFER);
    view.guest_address = 0xffffe001U;
    CHECK(texture_pixel_convert_t8_to_ct32(&view, 128, 64, scratch, sizeof(scratch)) == TEXTURE_PIXEL_INVALID_BUFFER);
    view.guest_address = 0x00400000;
    view.storage = (u8 *)(UINTPTR_MAX - 8191);
    CHECK(texture_pixel_convert_t8_to_ct32(&view, 128, 64, scratch, sizeof(scratch)) == TEXTURE_PIXEL_INVALID_BUFFER);
    view.storage = NULL;
    CHECK(texture_pixel_convert_t8_to_ct32(&view, 128, 64, scratch, sizeof(scratch)) == TEXTURE_PIXEL_INVALID_BUFFER);
    view.storage = pixels;
    CHECK(texture_pixel_convert_t8_to_ct32(&view, 128, 64, (u8 *)(UINTPTR_MAX - 8191), 8192) == TEXTURE_PIXEL_INVALID_BUFFER);
    CHECK(memcmp(pixels, saved_pixels, sizeof(pixels)) == 0);
    CHECK(memcmp(scratch, saved_scratch, sizeof(scratch)) == 0);
    /* The last declared guest byte is allowed; host pixels may be unaligned. */
    view.guest_address = 0xffffe000U;
    CHECK(texture_pixel_convert_t8_to_ct32(&view, 128, 64, scratch, sizeof(scratch)) == TEXTURE_PIXEL_CONVERTED);
}

static void descriptor_inside_scratch(void) {
    union { uintptr_t alignment; u8 bytes[8192+64]; } scratch;
    TexturePixelBuffer view, *borrowed = (TexturePixelBuffer *)(scratch.bytes+16);
    u8 pixels[8192], expected[8192], ordinary_scratch[8192];
    u32 i;
    for(i=0;i<8192;i++) pixels[i]=(u8)(i*13+(i>>8));
    view.guest_address=0x400000; view.bytes=8192; view.storage=pixels;
    CHECK(texture_pixel_convert_t8_to_ct32(&view,128,64,ordinary_scratch,8192)==1);
    memcpy(expected,pixels,8192);
    for(i=0;i<8192;i++) pixels[i]=(u8)(i*13+(i>>8));
    memset(scratch.bytes,0xB5,sizeof(scratch.bytes));
    *borrowed=view;
    CHECK(texture_pixel_convert_t8_to_ct32(borrowed,128,64,scratch.bytes,8192)==1);
    for(i=0;i<8192;i++) { CHECK(pixels[i]==expected[i]); CHECK(scratch.bytes[i]==expected[i]); }
    for(i=8192;i<sizeof(scratch.bytes);i++) CHECK(scratch.bytes[i]==0xB5);
}

int main(void) {
    u32 i;
    for (i = 0; i < 32; i++) column_and_guards(i);
    refusal();
    descriptor_inside_scratch();
    printf("%u checks, %u failures\n", checks, failures);
    return failures != 0;
}
