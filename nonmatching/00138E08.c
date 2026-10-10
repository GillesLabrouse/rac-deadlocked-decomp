#include "common.h"

/* Actual 00138E08..00138FEC, 480 instruction bytes and a trailing marker.
 * The 59-entry retail jump table at00194B30 gives the format groups below. */
#ifdef DEADLOCKED_NATIVE
#include "../native/texture_transfer_packets.h"
#define PACKET_STORE(off, value) texture_transfer_store(packet, off, value, 8)
int texture_transfer_build_upload(const TextureTransferPacketOutput *out,
    s32 base256, s32 buffer_width64, s32 format, s32 x, s32 y, s32 width, s32 height) {
    u8 *packet;
    if (!texture_transfer_output_valid(out, 96)) return TEXTURE_TRANSFER_INVALID_OUTPUT;
    packet = out->storage;
#else
extern char D_00194B00[];
s32 func_001294F0(const char *, ...);
#define PACKET_STORE(off, value) (*(u64 *)(packet + (off)) = (value))
s32 func_00138E08(u8 *packet, s32 base256, s32 buffer_width64, s32 format,
                 s32 x, s32 y, s32 width, s32 height) {
#endif
    s32 qwc = 0, product;
    u64 base, row, mode, xpos, ypos, w, h;
    base256 = (s16)base256; buffer_width64 = (s16)buffer_width64;
    format = (s16)format; x = (s16)x; y = (s16)y;
    width = (s16)width; height = (s16)height;
    product = (s32)((u32)width * (u32)height);
    if (format == 0 || format == 0x30) qwc = product >> 2;
    else if (format == 1 || format == 0x31) qwc = (s32)((u32)product * 3U) >> 4;
    else if (format == 2 || format == 0xA || format == 0x32 || format == 0x3A) qwc = product >> 3;
    else if (format == 0x13 || format == 0x1B) qwc = product >> 4;
    else if (format == 0x14 || format == 0x24 || format == 0x2C) qwc = product >> 5;
    if (qwc > 0x7FFF) {
#ifndef DEADLOCKED_NATIVE
        func_001294F0(D_00194B00);
#endif
        return 0;
    }
    base = (u64)(s64)base256; row = (u64)(s64)buffer_width64;
    mode = (u64)(s64)format; xpos = (u64)(s64)x; ypos = (u64)(s64)y;
    w = (u64)(s64)width; h = (u64)(s64)height;
    PACKET_STORE(0x00, ((u64)1 << 60) | 4);
    PACKET_STORE(0x08, 0xE);
    PACKET_STORE(0x10, (base << 32) | (row << 48) | (mode << 56));
    PACKET_STORE(0x18, 0x50);
    PACKET_STORE(0x20, (xpos << 32) | (ypos << 48));
    PACKET_STORE(0x28, 0x51);
    PACKET_STORE(0x30, w | (h << 32));
    PACKET_STORE(0x38, 0x52);
    PACKET_STORE(0x40, 0);
    PACKET_STORE(0x48, 0x53);
    PACKET_STORE(0x50, ((u64)1 << 59) | 0x8000 | ((u32)qwc & 0x7FFF));
    PACKET_STORE(0x58, 0);
#ifndef DEADLOCKED_NATIVE
    __asm__ volatile ("sync");
#endif
    return 6;
}
