#ifndef DEADLOCKED_TEXTURE_TRANSFER_PACKETS_H
#define DEADLOCKED_TEXTURE_TRANSFER_PACKETS_H
#include "common.h"

/* The address names the guest packet, never a converted host pointer. These
 * builders contain numeric GS/VIF/GIF fields and no DMA source pointer. */
typedef struct TextureTransferPacketOutput {
    u32 guest_address, bytes;
    u8 *storage;
} TextureTransferPacketOutput;
enum {
    TEXTURE_TRANSFER_INVALID_OUTPUT = -1,
    TEXTURE_TRANSFER_UPLOAD_TOO_LARGE = 0,
    TEXTURE_TRANSFER_UPLOAD_QWORDS = 6,
    TEXTURE_TRANSFER_READBACK_QWORDS = 7,
    TEXTURE_TRANSFER_UPLOAD_BYTES = 96,
    TEXTURE_TRANSFER_READBACK_BYTES = 112
};
/* Actual core 00138E08 / 00138FF0. Each scalar follows its retail signed16
 * truncation, including negative and unknown format values. This is packet
 * preparation, not GS range validation or GPU execution. Guest packet address
 * must be aligned16 and its complete declared range must fit in32 bits. Host
 * storage may be unaligned. Errors leave all storage bytes untouched. Upload
 * QWC>32767 returns0, as retail does after its diagnostic printer; this host
 * API does not execute that PS2 diagnostic. Success returns the real6 or7.
 */
int texture_transfer_build_upload(const TextureTransferPacketOutput *, s32 base256,
    s32 buffer_width64, s32 format, s32 x, s32 y, s32 width, s32 height);
int texture_transfer_build_readback(const TextureTransferPacketOutput *, s32 base256,
    s32 buffer_width64, s32 format, s32 x, s32 y, s32 width, s32 height);

#ifdef DEADLOCKED_NATIVE
#include <stdint.h>
static inline int texture_transfer_output_valid(const TextureTransferPacketOutput *out, u32 required) {
    return out != NULL && out->storage != NULL && out->bytes >= required &&
        (out->guest_address & 15U) == 0 &&
        (u64)out->guest_address + out->bytes <= ((u64)1 << 32) &&
        (uintptr_t)out->storage <= UINTPTR_MAX - out->bytes;
}
static inline void texture_transfer_store(u8 *out, u32 offset, u64 value, u32 bytes) {
    u32 i;
    for (i = 0; i < bytes; i++) out[offset + i] = (u8)(value >> (i * 8));
}
#endif
#endif
