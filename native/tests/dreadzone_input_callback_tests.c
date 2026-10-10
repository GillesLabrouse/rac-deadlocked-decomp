#include "dreadzone_input_callback.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(sizeof(DreadzoneInputContext1E0) == 0x1E0, "Callback context stride changed");
_Static_assert(offsetof(DreadzoneInputContext1E0, f3C) == 0x3C, "Bit cursor moved");
_Static_assert(offsetof(DreadzoneInputContext1E0, f1D8) == 0x1D8, "Frame count moved");
_Static_assert(sizeof(((DreadzoneInputContext1E0 *)0)->f0[0]) == 2, "Snapshots must remain u16");

s32 D_002208A8;
s32 D_002208AC;
u8 D_0021DE31[1];
s32 D_0021DDB4_[1];
s32 D_0021E8A0[1];
static unsigned checks;

static void check(int condition, const char *message) {
    checks++;
    if (!condition) {
        fprintf(stderr, "Dreadzone input callback check failed: %s\n", message);
        exit(1);
    }
}

/* Independent bit-at-a-time oracle, unlike the recovered byte-chunk writer. */
static void write_bits(u8 *buffer, unsigned *cursor, u32 value, unsigned width) {
    unsigned i;
    for (i = 0; i < width; i++) {
        unsigned target = *cursor + i;
        unsigned bit = (value >> (width - 1 - i)) & 1U;
        u8 mask = (u8)(1U << (7 - target % 8));
        buffer[target / 8] = (u8)((buffer[target / 8] & ~mask) | (bit ? mask : 0));
    }
    *cursor += width;
}

static void test_bit_writer(void) {
    static const u32 values[] = {0, 0xFFFFFFFFU, 0x9B70D35AU, 0x80000001U};
    u8 actual[12], expected[12];
    unsigned offset, width, value, cursor;
    s32 actual_cursor;
    for (offset = 0; offset < 16; offset++) {
        for (width = 1; width <= 32; width++) {
            for (value = 0; value < sizeof(values)/sizeof(values[0]); value++) {
                memset(actual, 0xA5, sizeof(actual));
                memcpy(expected, actual, sizeof(expected));
                cursor = offset;
                actual_cursor = (s32)offset;
                write_bits(expected, &cursor, values[value], width);
                func_L01_006B59C8(actual, (s32)values[value], &actual_cursor, (u8)width);
                check(actual_cursor == (s32)cursor, "actual bit writer advances the exact requested count");
                check(memcmp(actual, expected, sizeof(actual)) == 0,
                      "actual bit writer preserves surrounding bits and writes MSB first across byte boundaries");
            }
        }
    }
    memset(actual, 0xA5, sizeof(actual));
    memcpy(expected, actual, sizeof(expected));
    actual_cursor = 5;
    func_L01_006B59C8(actual, 0x1234, &actual_cursor, 0);
    check(actual_cursor == 5 && memcmp(actual, expected, sizeof(actual)) == 0,
          "zero-width write preserves count and output");
}

static void packet(u8 bytes[20], u16 raw, unsigned frame) {
    memset(bytes, 0, 20);
    bytes[1] = 0x79;
    /* Callback snapshots are a byte copy into native little-endian u16;
     * the later decoder independently interprets bytes2/3 big endian.
     */
    bytes[2] = (u8)raw;
    bytes[3] = (u8)(raw >> 8);
    bytes[4] = (u8)(127 + frame);
    bytes[5] = (u8)(117 + frame);
    bytes[6] = (u8)(137 + frame);
    bytes[7] = (u8)(147 + frame);
}

/* Format oracle: unchanged frames carry no payload. Up to three toggles carry
 * ascending four-bit indices with zero separators and a final one. Four or
 * more toggles carry the complete raw mask as four nibbles with zero separators.
 * This oracle never emits the callback's discarded tentative toggle encoding.
 */
static void oracle(DreadzoneInputContext1E0 *context, const u8 bytes[20],
                   s32 length, int blocked, s32 *frames, s32 *bits) {
    unsigned slot = (unsigned)context->f1D8 % 4;
    unsigned raw = blocked ? 0xFFFF : bytes[2] | ((unsigned)bytes[3] << 8);
    unsigned previous = slot == 0 ? 0xFFFF : context->f0[slot - 1];
    unsigned difference = raw ^ previous;
    unsigned changed[16], count = 0, index, cursor = (unsigned)context->f3C;
    u8 *stream = (u8 *)context + 0x19;
    context->f0[slot] = (u16)raw;
    context->f8[slot] = blocked || length < 8 ? 0x8080 :
        (u16)(bytes[6] | ((unsigned)bytes[7] << 8));
    context->f10[slot] = blocked || length < 8 ? 0x8080 :
        (u16)(bytes[4] | ((unsigned)bytes[5] << 8));
    for (index = 0; index < 16; index++)
        if (difference & (1U << index)) changed[count++] = index;
    if (count == 0) {
        context->f18 &= ~(1U << slot);
    } else {
        context->f18 |= 1U << slot;
        if (count <= 3) {
            for (index = 0; index < count; index++) {
                write_bits(stream, &cursor, changed[index], 4);
                write_bits(stream, &cursor, index + 1 == count ? 1 : 0, 1);
            }
        } else {
            for (index = 0; index < 4; index++) {
                write_bits(stream, &cursor, (raw >> (12 - 4 * index)) & 15U, 4);
                if (index != 3) write_bits(stream, &cursor, 0, 1);
            }
        }
    }
    context->f1D8++;
    context->f3C = (s32)cursor;
    if (slot == 3) {
        context->f1DC = (s32)cursor;
        *frames += 4;
        *bits += 4 + (s32)cursor;
        context->f3C = 0;
    } else context->f1DC = -1;
}

static void test_callback_sequences(void) {
    static const u16 sequences[][8] = {
        {0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF},
        {0xFFFE,0xFFFC,0xFFFD,0xFFFF,0xFFFE,0xFFFE,0xFFFC,0xFFFF},
        {0xFFFC,0xFFF8,0xFFF8,0xFFFF,0xF0FF,0x0FFF,0xFFFF,0xFFFF},
        {0xFFF8,0xFFF0,0xF000,0x0000,0x1234,0xABCD,0xFFFF,0x0000}
    };
    struct { u32 before; DreadzoneInputContext1E0 context; u32 after; } guarded;
    DreadzoneInputContext1E0 expected;
    InputRecord5C0 record, original_record;
    u8 bytes[20], original_packet[20];
    s32 expected_frames, expected_bits;
    unsigned sequence, frame;
    for (sequence = 0; sequence < sizeof(sequences)/sizeof(sequences[0]); sequence++) {
        memset(&record, 0, sizeof(record));
        original_record = record;
        /* Nonzero backing checks masked writes preserve unused stream bits. */
        memset(&guarded.context, 0xA5, sizeof(guarded.context));
        guarded.context.f1D8 = 0;
        guarded.context.f3C = 0;
        expected = guarded.context;
        guarded.before = 0xCAFEBABEU;
        guarded.after = 0x12345678U;
        D_002208AC = expected_frames = 8;
        D_002208A8 = expected_bits = 17;
        for (frame = 0; frame < 8; frame++) {
            packet(bytes, sequences[sequence][frame], frame);
            memcpy(original_packet, bytes, sizeof(bytes));
            oracle(&expected, bytes, 20, 0, &expected_frames, &expected_bits);
            func_L01_006B5CB8(&record, bytes, 20, &guarded.context);
            check(memcmp(&guarded.context, &expected, sizeof(expected)) == 0,
                  "actual callback matches independent four-frame encoding and all preserved context bytes");
            check(D_002208AC == expected_frames && D_002208A8 == expected_bits,
                  "actual callback publishes exact completed-frame and bit statistics");
            check(memcmp(bytes, original_packet, sizeof(bytes)) == 0 &&
                  memcmp(&record, &original_record, sizeof(record)) == 0,
                  "actual callback captures packet state without mutating packet or input record");
            check(guarded.before == 0xCAFEBABEU && guarded.after == 0x12345678U,
                  "actual callback preserves both context boundaries");
        }
    }
}

static void test_short_and_blocked_packet(void) {
    InputRecord5C0 record;
    DreadzoneInputContext1E0 context, expected;
    u8 bytes[20];
    s32 frames, bits;
    unsigned mode, length;
    for (mode = 0; mode < 3; mode++) {
        for (length = 4; length <= 8; length++) {
            memset(&record, 0, sizeof(record));
            record.f1CC = (s32)mode;
            memset(&context, 0, sizeof(context));
            expected = context;
            frames = bits = D_002208AC = D_002208A8 = 0;
            packet(bytes, 0x1234, 0);
            oracle(&expected, bytes, (s32)length, mode != 0, &frames, &bits);
            func_L01_006B5CB8(&record, bytes, (s32)length, &context);
            check(memcmp(&context, &expected, sizeof(expected)) == 0,
                  "short packets keep raw buttons while using neutral sticks; modes use the actual neutral packet");
        }
    }
}

static void test_actual_callback_before_decoder(void) {
    InputRecord5C0 record;
    DreadzoneInputContext1E0 context;
    u8 bytes[20];
    unsigned frame;
    memset(&record, 0, sizeof(record));
    memset(&context, 0, sizeof(context));
    D_002208AC = D_002208A8 = 0;
    for (frame = 0; frame < 4; frame++) {
        packet(bytes, frame < 2 ? 0xF7FF : 0xFFFF, frame);
        /* Actual poll order: preserve masks, invoke the actual callback, then
         * invoke actual packet/math decoding. No controller transport is run.
         */
        record.f1AC = record.f348;
        record.f1BC = record.f1B0;
        record.f1A0 = record.f1B0 = 0;
        func_L01_006B5CB8(&record, bytes, 20, &context);
        func_L01_005BDA40(&record, bytes, 20);
        check(record.f1A0 == (frame < 2 ? 0x8 : 0),
              "big-endian decoder consumes the same bytes after the actual little-endian snapshot callback");
        check(record.f1A4 == (frame == 0 ? 0x8 : 0) && record.f1A8 == (frame == 2 ? 0x8 : 0),
              "real callback-before-decoder ordering preserves press/hold/release semantics");
    }
    check(context.f1D8 == 4 && context.f1DC == 10 && D_002208AC == 4 && D_002208A8 == 14,
          "actual callback completes its group while the actual decoder completes four histories");
    check(record.f190 == 4 && record.f18E == 4, "actual decoder history advances alongside callback snapshots");
}

int main(void) {
    test_bit_writer();
    test_callback_sequences();
    test_short_and_blocked_packet();
    test_actual_callback_before_decoder();
    printf("Dreadzone actual packet callback/bit writer: %u checks passed.\n", checks);
    return 0;
}
