#include "common.h"

/* Actual 00138FF0..00139130, 320 bytes. These are the complete112 bytes
 * consumed by the real readback procedure001392B0. No image is transferred. */
#ifdef DEADLOCKED_NATIVE
#include "../native/texture_transfer_packets.h"
#define PACKET_STORE(off, value) texture_transfer_store(packet, off, value, 8)
int texture_transfer_build_readback(const TextureTransferPacketOutput *out,
    s32 base256, s32 buffer_width64, s32 format, s32 x, s32 y, s32 width, s32 height) {
    u8 *packet;
    if (!texture_transfer_output_valid(out, 112)) return TEXTURE_TRANSFER_INVALID_OUTPUT;
    packet = out->storage;
#else
#define PACKET_STORE(off, value) (*(u64 *)(packet + (off)) = (value))
s32 func_00138FF0(u8 *packet, s32 base256, s32 buffer_width64, s32 format,
                 s32 x, s32 y, s32 width, s32 height) {
#endif
    u64 base, row, mode, xpos, ypos, w, h;
    base = (u64)(s64)(s16)base256;
    row = (u64)(s64)(s32)((u32)buffer_width64 << 16);
    mode = (u64)(s64)(s16)format;
    xpos = (u64)(s64)(s16)x;
    ypos = (u64)(s64)(s32)((u32)y << 16);
    w = (u64)(s64)(s16)width; h = (u64)(s64)(s16)height;
    PACKET_STORE(0x00, (u64)0x06008000 << 32);
    PACKET_STORE(0x08, ((u64)0x50000006 << 32) | 0x13000000);
    PACKET_STORE(0x10, ((u64)1 << 60) | 0x8005);
    PACKET_STORE(0x18, 0xE);
    PACKET_STORE(0x20, base | row | (mode << 24));
    PACKET_STORE(0x28, 0x50);
    PACKET_STORE(0x30, xpos | ypos);
    PACKET_STORE(0x38, 0x51);
    PACKET_STORE(0x40, w | (h << 32));
    PACKET_STORE(0x48, 0x52);
    PACKET_STORE(0x50, 0);
    PACKET_STORE(0x58, 0x61);
    PACKET_STORE(0x60, 1);
    PACKET_STORE(0x68, 0x53);
#ifndef DEADLOCKED_NATIVE
    __asm__ volatile ("sync");
#endif
    return 7;
}
