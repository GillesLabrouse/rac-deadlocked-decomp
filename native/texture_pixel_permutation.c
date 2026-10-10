#include "texture_pixel_permutation.h"
#include <stdint.h>
#include <string.h>

/* Independently expressed byte-coordinate relation, rather than a copied GS
 * table or a Wrench swizzler. Public evidence and the independent VRAM oracle
 * are described in private/startup-agent-pixel-permutation-report.md.
 *
 * CT32 and T8 share block numbering after halving the dimensions. Within the
 * 256-byte block, the CT32 byte address selects the following T8 coordinates:
 * x[0:1] = CT32.x[0:1], x[2] = CT32.x[2] ^ component[0] ^ CT32.y[1],
 * x[3] = component[1], y[0] = CT32.y[0], y[1] = component[0],
 * y[2:3] = CT32.y[1:2]. The XOR incorporates the alternating GS columns.
 */
static u32 source_byte(u32 x, u32 y, u32 component, u32 width) {
    u32 source_x = (x / 8) * 16 + (x & 3) +
        (((x >> 2) ^ component ^ (y >> 1)) & 1) * 4 + (component >> 1) * 8;
    u32 source_y = (y / 8) * 16 + (y & 1) + (component & 1) * 2 + (y & 6) * 2;
    return source_y * width + source_x;
}

int texture_pixel_convert_t8_to_ct32(const TexturePixelBuffer *pixels,
    u32 width, u32 height, u8 *scratch, u32 scratch_bytes) {
    u32 count, x, y, component, output = 0;
    uintptr_t pixel_start, scratch_start;
    u8 *storage;

    if (width == 0 || height == 0 || (width & 127) != 0 || (height & 63) != 0 ||
        width >= 4096 || height >= 4096 || (u64)width * height > 32767U * 16U)
        return TEXTURE_PIXEL_UNSUPPORTED_RECTANGLE;
    count = width * height;
    if (pixels == NULL || pixels->storage == NULL || scratch == NULL ||
        pixels->bytes < count || scratch_bytes < count ||
        (pixels->guest_address & 15U) != 0 ||
        (u64)pixels->guest_address + pixels->bytes > ((u64)1 << 32))
        return TEXTURE_PIXEL_INVALID_BUFFER;
    pixel_start = (uintptr_t)pixels->storage;
    scratch_start = (uintptr_t)scratch;
    if (pixel_start > UINTPTR_MAX - pixels->bytes ||
        scratch_start > UINTPTR_MAX - scratch_bytes ||
        (pixel_start < scratch_start + scratch_bytes &&
         scratch_start < pixel_start + pixels->bytes))
        return TEXTURE_PIXEL_INVALID_BUFFER;

    /* The descriptor may be inside caller scratch. Retain its pointer before
     * writes overwrite descriptor bytes; no metadata is read after this point. */
    storage = pixels->storage;

    for (y = 0; y < height / 2; y++)
        for (x = 0; x < width / 2; x++)
            for (component = 0; component < 4; component++)
                scratch[output++] = storage[source_byte(x, y, component, width)];
    memcpy(storage, scratch, count);
    return TEXTURE_PIXEL_CONVERTED;
}
