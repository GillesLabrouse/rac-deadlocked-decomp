#ifndef DEADLOCKED_TEXTURE_LAYOUT_PREPARATION_H
#define DEADLOCKED_TEXTURE_LAYOUT_PREPARATION_H

#include "texture_pixel_permutation.h"
#include "texture_transfer_packets.h"

typedef struct TextureLayoutPreparation {
    TexturePixelBuffer pixels;
    TextureTransferPacketOutput upload_packet;
    TextureTransferPacketOutput readback_packet;
    u8 *scratch;
    u32 scratch_bytes;
} TextureLayoutPreparation;

enum {
    TEXTURE_LAYOUT_INVALID_BUFFERS = -1,
    TEXTURE_LAYOUT_UNSUPPORTED_RECTANGLE = 0,
    TEXTURE_LAYOUT_CPU_EFFECT_PREPARED = 1
};

/* Native preparation for the supported memory effect of L01 00575180.
 * Produces its genuine upload/readback packet bytes and changes the original
 * pixel backing with the separately verified GS-model permutation. The input
 * format is ignored, as in retail. The rectangle domain is that of
 * texture_pixel_convert_t8_to_ct32; unsupported input is refused atomically.
 *
 * Return 1 means CPU packet preparation and pixel-memory effect only. It is
 * deliberately distinct from the PS2 routine's return 0. No syscall, DMA,
 * readback FIFO, hardware wait, or Direct3D upload has run. A graphics backend
 * can consume the exposed packets and numeric pixels.guest_address separately.
 *
 * All four complete host backing ranges must be representable and disjoint;
 * the three guest ranges must be representable, aligned to 16 bytes and
 * disjoint. The request and its descriptors are copied before writes, so they
 * may reside inside a backing range. Success changes only the first 96 / 112
 * packet bytes and width * height pixel/scratch bytes. Failure changes none.
 */
int texture_layout_prepare_cpu_effect(const TextureLayoutPreparation *request,
    s32 unused_format, s32 width, s32 height);

#endif
