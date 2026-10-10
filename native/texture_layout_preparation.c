#include "texture_layout_preparation.h"
#include <stdint.h>
#include <string.h>

static int host_ranges_disjoint(const void *a, u32 a_bytes, const void *b, u32 b_bytes) {
    uintptr_t a_start = (uintptr_t)a, b_start = (uintptr_t)b;
    return a_start + a_bytes <= b_start || b_start + b_bytes <= a_start;
}

static int guest_ranges_disjoint(u32 a, u32 a_bytes, u32 b, u32 b_bytes) {
    return (u64)a + a_bytes <= b || (u64)b + b_bytes <= a;
}

int texture_layout_prepare_cpu_effect(const TextureLayoutPreparation *request,
    s32 unused_format, s32 width, s32 height) {
    TextureLayoutPreparation inputs;
    TextureTransferPacketOutput upload, readback;
    u8 upload_bytes[96], readback_bytes[112];
    u32 count;
    s32 base256, upload_width64, readback_width64;
    int pixel_result;
    (void)unused_format;

    if (width <= 0 || height <= 0 || ((u32)width & 127) != 0 ||
        ((u32)height & 63) != 0 || width >= 4096 || height >= 4096 ||
        (u64)(u32)width * (u32)height > 32767U * 16U)
        return TEXTURE_LAYOUT_UNSUPPORTED_RECTANGLE;
    if (request == NULL) return TEXTURE_LAYOUT_INVALID_BUFFERS;
    /* Snapshot every borrowed descriptor before writing any caller backing. */
    inputs = *request;
    count = (u32)width * (u32)height;
    if (!texture_transfer_output_valid(&inputs.upload_packet, 96) ||
        !texture_transfer_output_valid(&inputs.readback_packet, 112) ||
        inputs.pixels.storage == NULL || inputs.scratch == NULL ||
        inputs.pixels.bytes < count || inputs.scratch_bytes < count ||
        (inputs.pixels.guest_address & 15U) != 0 ||
        (u64)inputs.pixels.guest_address + inputs.pixels.bytes > ((u64)1 << 32) ||
        (uintptr_t)inputs.pixels.storage > UINTPTR_MAX - inputs.pixels.bytes ||
        (uintptr_t)inputs.scratch > UINTPTR_MAX - inputs.scratch_bytes)
        return TEXTURE_LAYOUT_INVALID_BUFFERS;


    if (!host_ranges_disjoint(inputs.pixels.storage, inputs.pixels.bytes, inputs.scratch, inputs.scratch_bytes) ||
        !host_ranges_disjoint(inputs.pixels.storage, inputs.pixels.bytes, inputs.upload_packet.storage, inputs.upload_packet.bytes) ||
        !host_ranges_disjoint(inputs.pixels.storage, inputs.pixels.bytes, inputs.readback_packet.storage, inputs.readback_packet.bytes) ||
        !host_ranges_disjoint(inputs.scratch, inputs.scratch_bytes, inputs.upload_packet.storage, inputs.upload_packet.bytes) ||
        !host_ranges_disjoint(inputs.scratch, inputs.scratch_bytes, inputs.readback_packet.storage, inputs.readback_packet.bytes) ||
        !host_ranges_disjoint(inputs.upload_packet.storage, inputs.upload_packet.bytes, inputs.readback_packet.storage, inputs.readback_packet.bytes) ||
        !guest_ranges_disjoint(inputs.pixels.guest_address, inputs.pixels.bytes, inputs.upload_packet.guest_address, inputs.upload_packet.bytes) ||
        !guest_ranges_disjoint(inputs.pixels.guest_address, inputs.pixels.bytes, inputs.readback_packet.guest_address, inputs.readback_packet.bytes) ||
        !guest_ranges_disjoint(inputs.upload_packet.guest_address, inputs.upload_packet.bytes, inputs.readback_packet.guest_address, inputs.readback_packet.bytes))
        return TEXTURE_LAYOUT_INVALID_BUFFERS;


    base256 = (s16)((0x00400000U - count) >> 8);
    upload_width64 = (s16)(width >> 6);
    readback_width64 = (s16)(width >> 7);
    upload = inputs.upload_packet;
    readback = inputs.readback_packet;
    upload.storage = upload_bytes;
    upload.bytes = sizeof(upload_bytes);
    readback.storage = readback_bytes;
    readback.bytes = sizeof(readback_bytes);
    if (texture_transfer_build_upload(&upload, base256, upload_width64, 0x13, 0, 0, (s16)width, (s16)height) != 6 ||
        texture_transfer_build_readback(&readback, base256, readback_width64, 0, 0, 0,
            (s32)((u32)width << 15) >> 16, (s32)((u32)height << 15) >> 16) != 7)
        return TEXTURE_LAYOUT_INVALID_BUFFERS;
    pixel_result = texture_pixel_convert_t8_to_ct32(&inputs.pixels, (u32)width, (u32)height,
        inputs.scratch, inputs.scratch_bytes);
    if (pixel_result != TEXTURE_PIXEL_CONVERTED)
        return pixel_result == TEXTURE_PIXEL_UNSUPPORTED_RECTANGLE ?
            TEXTURE_LAYOUT_UNSUPPORTED_RECTANGLE : TEXTURE_LAYOUT_INVALID_BUFFERS;
    memcpy(inputs.upload_packet.storage, upload_bytes, sizeof(upload_bytes));
    memcpy(inputs.readback_packet.storage, readback_bytes, sizeof(readback_bytes));
    return TEXTURE_LAYOUT_CPU_EFFECT_PREPARED;
}
