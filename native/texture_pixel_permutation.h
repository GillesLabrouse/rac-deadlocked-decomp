#ifndef DEADLOCKED_TEXTURE_PIXEL_PERMUTATION_H
#define DEADLOCKED_TEXTURE_PIXEL_PERMUTATION_H

#include "common.h"

typedef struct TexturePixelBuffer {
    u32 guest_address;
    u32 bytes;
    u8 *storage;
} TexturePixelBuffer;

enum {
    TEXTURE_PIXEL_INVALID_BUFFER = -1,
    TEXTURE_PIXEL_UNSUPPORTED_RECTANGLE = 0,
    TEXTURE_PIXEL_CONVERTED = 1
};

/* CPU replacement for the successful pixel-memory effect of L01 00575180:
 * upload linear PSMT8, then read back PSMCT32 at half width and half height.
 * This implementation follows the GS addressing model; it has not been
 * validated against a hardware capture. It performs no DMA, synchronization,
 * palette conversion, graphics API upload, or emulated hardware-status writes.
 *
 * The supported domain has width divisible by 128, height divisible by 64,
 * both dimensions below 4096, and width * height / 16 <= 32767. Those page
 * rectangles cover identical GS byte addresses in both formats, so no initial
 * VRAM contents are needed. Other dimensions are explicitly unsupported.
 *
 * The guest pixel address must be aligned to 16 bytes, as required by the
 * original DMA transfer; the host backing may be unaligned. The complete
 * declared guest and host pixel ranges must be representable.
 * Caller-owned scratch must hold width * height bytes and its complete declared
 * host range must be disjoint from the complete pixel range. All validation
 * happens before any writes. Success changes only the first width * height
 * bytes of pixels and scratch. Failure leaves both buffers untouched.
 */
int texture_pixel_convert_t8_to_ct32(const TexturePixelBuffer *pixels,
    u32 width, u32 height, u8 *scratch, u32 scratch_bytes);

#endif
