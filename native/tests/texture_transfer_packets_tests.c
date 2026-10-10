#include "../texture_transfer_packets.h"
#include <stdio.h>
#include <string.h>

static unsigned checks, failures;
#define CHECK(test) do { checks++; if (!(test)) { failures++; fprintf(stderr, "line%d: %s\n", __LINE__, #test); } } while (0)
static u8 hex_digit(char x) { return (u8)(x <= '9' ? x - '0' : x - 'a' + 10); }
/* Byte images obtained by executing the original builders on the original
 * instruction image; these contain no instruction or asset bytes. */
static const char upload256[] =
    "04000000000000100e0000000000000000000000003f04135000000000000000"
    "0000000000000000510000000000000000010000000100005200000000000000"
    "0000000000000000530000000000000000900000000000080000000000000000";
static const char readback128[] =
    "0000000000800006000000130600005005800000000000100e00000000000000"
    "003f020000000000500000000000000000000000000000005100000000000000"
    "8000000080000000520000000000000000000000000000006100000000000000"
    "01000000000000005300000000000000";
static void golden(int readback, u32 offset) {
    u8 data[160];
    u32 i, bytes = readback ? 112 : 96;
    const char *expected = readback ? readback128 : upload256;
    TextureTransferPacketOutput out = {0x400010, bytes, data + offset};
    memset(data, 0xC7, sizeof(data));
    CHECK((readback ? texture_transfer_build_readback(&out, 0x3F00, 2, 0, 0, 0, 128, 128)
                    : texture_transfer_build_upload(&out, 0x3F00, 4, 0x13, 0, 0, 256, 256)) == (readback ? 7 : 6));
    for (i = 0; i < sizeof(data); i++) {
        u8 byte = i >= offset && i < offset + bytes ?
            (u8)((hex_digit(expected[(i-offset)*2]) << 4) | hex_digit(expected[(i-offset)*2+1])) : 0xC7;
        CHECK(data[i] == byte);
    }
}
static void refusals(void) {
    u8 data[128], before[128];
    TextureTransferPacketOutput out = {0x400000, 112, data};
    u32 i;
    memset(data, 0x93, sizeof(data)); memcpy(before, data, sizeof(data));
    CHECK(texture_transfer_build_upload(NULL, 0, 0, 0, 0, 0, 1, 1) == -1);
    CHECK(texture_transfer_build_readback(NULL, 0, 0, 0, 0, 0, 1, 1) == -1);
    for (i = 0; i < 112; i++) {
        out.bytes = i;
        CHECK(texture_transfer_build_readback(&out, 0, 1, 0, 0, 0, 16, 16) == -1);
        if (i < 96) CHECK(texture_transfer_build_upload(&out, 0, 1, 0, 0, 0, 16, 16) == -1);
        CHECK(memcmp(data, before, sizeof(data)) == 0);
    }
    out.bytes = 112; out.guest_address = 0xFFFFFFF0U;
    CHECK(texture_transfer_build_readback(&out, 0, 1, 0, 0, 0, 16, 16) == -1);
    out.guest_address = 0x400001;
    CHECK(texture_transfer_build_upload(&out, 0, 1, 0, 0, 0, 16, 16) == -1);
    out.guest_address = 0x400000; out.storage = NULL;
    CHECK(texture_transfer_build_upload(&out, 0, 1, 0, 0, 0, 16, 16) == -1);
    out.storage = (u8 *)(uintptr_t)(UINTPTR_MAX - 16);
    CHECK(texture_transfer_build_readback(&out, 0, 1, 0, 0, 0, 16, 16) == -1);
    out.storage = data;
    CHECK(texture_transfer_build_upload(&out, 0, 1, 0x13, 0, 0, 32767, 17) == 0);
    CHECK(memcmp(data, before, sizeof(data)) == 0);
    CHECK(texture_transfer_build_upload(&out, 0, 1, 0x13, 0, 0, 32767, 16) == 6);
    CHECK(data[0x50] == 0xFF && data[0x51] == 0xFF);
    out.bytes = 112; out.guest_address = 0xFFFFFF90U;
    CHECK(texture_transfer_build_readback(&out, 0, 1, 0, 0, 0, 16, 16) == 7);
}
static void scalar_widths(void) {
    u8 a[112], b[112];
    TextureTransferPacketOutput x = {0x400000, sizeof(a), a}, y = {0x400000, sizeof(b), b};
    int readback;
    u32 i;
    for (readback = 0; readback < 2; readback++) for (i = 0; i < 65536; i += 257) {
        int ra, rb;
        memset(a, 0x9A, sizeof(a)); memset(b, 0x9A, sizeof(b));
        if (readback) {
            ra = texture_transfer_build_readback(&x, (s32)i, (s32)i, 0x13, (s32)i, (s32)i, 16, 16);
            rb = texture_transfer_build_readback(&y, (s32)(i | 0x76540000U), (s32)(i | 0xFEDC0000U), 0xABCD0013U,
                (s32)(i | 0xDEAD0000U), (s32)(i | 0xBEEF0000U), 0x54320010, 0x12340010);
        } else {
            ra = texture_transfer_build_upload(&x, (s32)i, (s32)i, 0x13, (s32)i, (s32)i, 16, 16);
            rb = texture_transfer_build_upload(&y, (s32)(i | 0x76540000U), (s32)(i | 0xFEDC0000U), 0xABCD0013U,
                (s32)(i | 0xDEAD0000U), (s32)(i | 0xBEEF0000U), 0x54320010, 0x12340010);
        }
        CHECK(ra == (readback ? 7 : 6) && rb == ra);
        CHECK(memcmp(a, b, sizeof(a)) == 0);
    }
}
int main(void) {
    u32 offset;
    for (offset = 0; offset < 32; offset++) { golden(0, offset); golden(1, offset); }
    refusals(); scalar_widths();
    printf("Texture transfer packet builders: %u checks, %u failures\n", checks, failures);
    return failures != 0;
}
