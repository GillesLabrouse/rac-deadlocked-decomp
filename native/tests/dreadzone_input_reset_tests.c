#include "dreadzone_input_reset.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Native C static storage supplies the observed core.bss zero precondition.
 * This is separate from any recovered function and is not a driver stub.
 */
InputRecord5C0 D_001EE600[DREADZONE_INPUT_RECORD_COUNT];
u8 D_0021DE31[1];
s32 D_0021DDB4_[1];
s32 D_0021E8A0[1];
static unsigned checks;

static void check(int condition, const char *message) {
    checks++;
    if (!condition) {
        fprintf(stderr, "Dreadzone input reset check failed: %s\n", message);
        exit(1);
    }
}

static void packet_neutral(u8 packet[20]) {
    memset(packet, 0, 20);
    packet[1] = 0x79;
    packet[2] = packet[3] = 255;
    packet[4] = packet[5] = packet[6] = packet[7] = 127;
}

static void decode(InputRecord5C0 *record, const u8 packet[20]) {
    record->f1A0 = 0;
    record->f1AC = record->f348;
    record->f1BC = record->f1B0;
    record->f1B0 = 0;
    func_L01_005BDA40(record, packet, 20);
}

static void test_initial_pool_and_real_decoder(void) {
    unsigned record_index;
    u8 packet[20];
    packet_neutral(packet);
    for (record_index = 0; record_index < DREADZONE_INPUT_RECORD_COUNT; record_index++) {
        InputRecord5C0 *record = &D_001EE600[record_index];
        check(record->f34C == 0 && record->f350 == 0 && record->f568 == 0,
              "core.bss starts all sample indices/counts/delays at zero");
        check(record->f18E == 0 && record->f190 == 0 && record->f1CC == 0,
              "core.bss starts history and mode at zero");
        check(record->f54D == 0 && record->f54E == 0 && record->f54F == 0,
              "repeat starts disabled through BSS storage, not the selective reset");
        func_L01_005BD950(record);
        func_L01_005BD710(record);
        decode(record, packet);
        check(record->f1A0 == 0 && record->f1A4 == 0 && record->f1A8 == 0,
              "real reset/history/decoder chain accepts the first neutral packet");
        check(record->f2D0[0] == 0 && record->f258[0] == 0 && record->f18E == 1,
              "first neutral packet executes the real magnitude and zero-angle paths");
    }
}

/* Byte-level oracle from the inspected store ranges. It independently checks
 * every preserved byte, including untouched histories/options and the ABI's
 * callback/context padding, instead of treating reset as memset(record,0).
 */
static void test_selective_reset_ranges(void) {
    static const unsigned zero_words[] = {
        0x1A0,0x1A4,0x1A8,0x1B0,0x1B4,0x1B8,0x1C0,0x1C4,0x1C8,0x1D8
    };
    struct { u32 before; InputRecord5C0 record; u32 after; } guarded;
    u8 expected[sizeof(InputRecord5C0)];
    unsigned write_index, pending, index, position;
    for (write_index = 0; write_index < 7; write_index++) {
        for (pending = 0; pending <= 6; pending++) {
            memset(&guarded.record, 0xA5, sizeof(guarded.record));
            guarded.before = 0x01234567U;
            guarded.after = 0x89ABCDEFU;
            guarded.record.f34C = (s32)write_index;
            guarded.record.f350 = (s32)pending;
            guarded.record.f18E = 17;
            guarded.record.f190 = 23;
            guarded.record.f568 = 4;
            guarded.record.f1CC = 2;
            memcpy(expected, &guarded.record, sizeof(expected));
            memset(expected + 0x100, 0, 0x80);
            for (index = 0; index < sizeof(zero_words)/sizeof(zero_words[0]); index++)
                memset(expected + zero_words[index], 0, 4);
            {
                s32 one = 1;
                memcpy(expected + 0x1D0, &one, 4);
                memcpy(expected + 0x1D4, &one, 4);
            }
            for (position = 0; position < 7; position++) {
                /* The circular backward distance is independent of the
                 * implementation's (write_index - sample + 7) remainder.
                 */
                unsigned backward_distance = write_index >= position ?
                    write_index - position : write_index + 7 - position;
                if (backward_distance < pending) {
                    memset(expected + 0x354 + position * 4, 0, 4);
                    memset(expected + 0x370 + position * 4, 0, 4);
                    memset(expected + 0x38C + position * 0x40, 0, 0x40);
                }
            }
            func_L01_005BD950(&guarded.record);
            check(memcmp(expected, &guarded.record, sizeof(expected)) == 0,
                  "selective reset changes exactly its observed byte ranges for every ring/count pair");
            check(guarded.before == 0x01234567U && guarded.after == 0x89ABCDEFU,
                  "selective reset preserves adjacent records");
        }
    }
}

static void test_history_reset_preservation(void) {
    InputRecord5C0 record, expected;
    InputRecord5C0 pool_expected[DREADZONE_INPUT_RECORD_COUNT];
    memset(&record, 0x5A, sizeof(record));
    expected = record;
    expected.f18E = 0;
    expected.f190 = 0;
    func_L01_005BD710(&record);
    check(memcmp(&record, &expected, sizeof(record)) == 0,
          "history reset preserves history contents, axes, mode, repeat, callback and sample ring");
    memset(D_001EE600, 0x5A, sizeof(D_001EE600));
    memcpy(pool_expected, D_001EE600, sizeof(pool_expected));
    pool_expected[0].f18E = 0;
    pool_expected[0].f190 = 0;
    func_L01_005BD710(NULL);
    check(memcmp(D_001EE600, pool_expected, sizeof(pool_expected)) == 0,
          "null history-reset pointer selects only the first actual pool record");
}

static void test_delay_setter_and_decoder_warmup(void) {
    InputRecord5C0 record, expected;
    u8 packet[20];
    unsigned old_delay, new_delay, frame;
    for (old_delay = 0; old_delay <= 6; old_delay++) {
        for (new_delay = 0; new_delay <= 6; new_delay++) {
            memset(&record, 0x5A, sizeof(record));
            record.f568 = (s32)old_delay;
            record.f34C = 5;
            record.f350 = 3;
            expected = record;
            expected.f568 = (s32)new_delay;
            if (new_delay > old_delay) {
                expected.f34C = 0;
                expected.f350 = 0;
            }
            func_L01_005BD790(&record, (s32)new_delay);
            check(memcmp(&record, &expected, sizeof(record)) == 0,
                  "delay setter changes only delay and, on increase, the two warmup fields");
        }
    }
    memset(&record, 0, sizeof(record));
    packet_neutral(packet);
    func_L01_005BD950(&record);
    func_L01_005BD710(&record);
    func_L01_005BD790(&record, 2);
    packet[2] = 0xF7; /* Logical bit 0800 after big-endian inversion. */
    packet[6] = 255;
    for (frame = 0; frame < 3; frame++) {
        decode(&record, packet);
        if (frame < 2) {
            check(record.f1A0 == 0 && record.f100[2] == 0,
                  "actual delay producer makes the real decoder retain neutral state through warmup");
        } else {
            check(record.f1A0 == 0x2800 && (record.f1A4 & 0x2800) == 0x2800,
                  "post-warmup packet produces real button and stick edges");
            check(record.f100[2] == 1.0f && record.f2D0[2] == 1.0f,
                  "post-warmup selected axis passes through the actual magnitude helper");
            check(fabs(record.f258[2]) < 0.000003,
                  "post-warmup positive x executes the actual polynomial helper");
        }
    }
    func_L01_005BD790(&record, 0);
    packet_neutral(packet);
    decode(&record, packet);
    check(record.f1A0 == 0 && record.f1A8 == 0x2800,
          "delay decrease immediately selects neutral input with actual release edges");
}

int main(void) {
    test_initial_pool_and_real_decoder();
    test_selective_reset_ranges();
    test_history_reset_preservation();
    test_delay_setter_and_decoder_warmup();
    printf("Dreadzone actual input reset/history/delay: %u checks passed.\n", checks);
    return 0;
}
