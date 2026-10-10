#include "dreadzone_input.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(sizeof(InputRecord5C0) == 0x5C0, "Input record stride changed");
_Static_assert(offsetof(InputRecord5C0, f100) == 0x100, "Selected axes moved");
_Static_assert(offsetof(InputRecord5C0, f140) == 0x140, "Unmasked axes moved");
_Static_assert(offsetof(InputRecord5C0, f18E) == 0x18E, "History index moved");
_Static_assert(offsetof(InputRecord5C0, f1A4) == 0x1A4, "Pressed mask moved");
_Static_assert(offsetof(InputRecord5C0, f348) == 0x348, "Previous snapshot moved");
_Static_assert(offsetof(InputRecord5C0, f38C) == 0x38C, "Channel rings moved");
_Static_assert(offsetof(InputRecord5C0, f568) == 0x568, "Delay moved");

u8 D_0021DE31[1];
s32 D_0021DDB4_[1];
s32 D_0021E8A0[1];
static unsigned checks;
static double greatest_angle_error;

static void check(int condition, const char *message) {
    checks++;
    if (!condition) {
        fprintf(stderr, "Dreadzone input check failed: %s\n", message);
        exit(1);
    }
}

static void near_value(double actual, double expected, double tolerance,
                       const char *message) {
    check(isfinite(actual) && fabs(actual - expected) <= tolerance, message);
}

static void reset(InputRecord5C0 *record, u8 packet[20]) {
    memset(record, 0, sizeof(*record));
    memset(packet, 0, 20);
    packet[1] = 0x79;
    packet[2] = packet[3] = 255;
    packet[4] = packet[5] = packet[6] = packet[7] = 127;
    D_0021DE31[0] = 0;
    D_0021DDB4_[0] = 0;
    D_0021E8A0[0] = 0;
}

static void buttons(u8 packet[20], unsigned held) {
    unsigned inverted = held ^ 0xFFFF;
    packet[2] = (u8)(inverted >> 8);
    packet[3] = (u8)inverted;
}

/* Perform the four observed previous-mask operations of the real poll caller.
 * Packet transport/configuration and its optional callback are outside this
 * unit fixture. All decoder and angular arithmetic calls are real recovered C.
 */
static void decode(InputRecord5C0 *record, const u8 packet[20], s32 length) {
    record->f1A0 = 0;
    record->f1AC = record->f348;
    record->f1BC = record->f1B0;
    record->f1B0 = 0;
    func_L01_005BDA40(record, packet, length);
}

/* Independent mathematical oracle: continuous atan2, not the recovered
 * polynomial, and a double-precision circular distance. This measures the
 * scalar approximation on a normal finite grid; it does not emulate VU flags.
 */
static void test_actual_angle_helpers(void) {
    static const double pi = 3.14159265358979323846;
    int x, y;
    double actual, expected, error;
    check(func_L01_0055D708(0.0f, 0.0f) == 0.0f, "actual zero branch returns zero");
    check(!signbit(func_L01_0055D708(-0.0f, -0.0f)), "zero branch clears signed zero");
    for (x = -16; x <= 16; x++) {
        for (y = -16; y <= 16; y++) {
            if (x == 0 && y == 0) continue;
            actual = func_L01_0055D708((f32)x / 16.0f, (f32)y / 16.0f);
            expected = atan2((double)y, (double)x);
            error = fabs(actual - expected);
            if (error > pi) error = fabs(2.0 * pi - error);
            if (error > greatest_angle_error) greatest_angle_error = error;
            check(isfinite(actual) && error < 0.000003,
                  "actual angle polynomial agrees with independent atan2 in every quadrant");
        }
    }
    near_value(func_L01_0055D708(1.0f, -0.0f), 0.0, 0.000003,
               "negative-zero y uses the correct quadrant");
    near_value(func_L01_0055D708(-1.0f, -0.0f), -pi, 0.000003,
               "negative-zero y selects the negative-pi boundary");
    near_value(func_L01_0055D910(0.0f, 0.0f), 0.0, 0.0, "equal angles have zero distance");
    check(!signbit(func_L01_0055D910(-0.0f, 0.0f)), "angular abs clears signed zero");
    near_value(func_L01_0055D910(3.0f, -3.0f), 2.0 * pi - 6.0, 0.000001,
               "crossing the pi seam uses the short circular distance");
    for (x = -16; x <= 16; x++) {
        for (y = -16; y <= 16; y++) {
            f32 a = (f32)((double)x * pi / 16.0);
            f32 b = (f32)((double)y * pi / 16.0);
            expected = fabs((double)a - b);
            if (expected >= pi) expected = 2.0 * pi - expected;
            near_value(func_L01_0055D910(a, b), expected, 0.000001,
                       "actual delta leaf agrees with independent circular distance");
        }
    }
}

static void test_neutral_and_button_edges(void) {
    struct { u32 before; InputRecord5C0 record; u32 after; } guarded;
    u8 packet[20];
    unsigned i;
    reset(&guarded.record, packet);
    guarded.before = 0xCAFEBABEU;
    guarded.after = 0x12345678U;
    decode(&guarded.record, packet, 20);
    check(guarded.record.f1A0 == 0 && guarded.record.f1A4 == 0 && guarded.record.f1A8 == 0,
          "neutral packet has no held/pressed/released buttons");
    for (i = 0; i < 16; i++) check(guarded.record.f100[i] == 0.0f, "neutral channels are zero");
    check(guarded.record.f18E == 1 && guarded.record.f190 == 1, "neutral sample advances history");
    check(guarded.record.f2D0[0] == 0.0f && guarded.record.f258[0] == 0.0f,
          "neutral packet uses the real zero-angle helper and magnitude");
    buttons(packet, 0x0801);
    decode(&guarded.record, packet, 4);
    check(guarded.record.f1A0 == 0x0801 && guarded.record.f1A4 == 0x0801 && guarded.record.f1A8 == 0,
          "button packet creates the expected press edges");
    decode(&guarded.record, packet, 4);
    check(guarded.record.f1A4 == 0 && guarded.record.f1A8 == 0, "held packet has no repeated edges");
    buttons(packet, 0x0001);
    decode(&guarded.record, packet, 4);
    check(guarded.record.f1A0 == 1 && guarded.record.f1A4 == 0 && guarded.record.f1A8 == 0x0800,
          "partial release preserves the remaining held bit");
    buttons(packet, 0);
    decode(&guarded.record, packet, 4);
    check(guarded.record.f1A8 == 1 && guarded.record.f1A0 == 0, "final release clears held");
    check(guarded.before == 0xCAFEBABEU && guarded.after == 0x12345678U,
          "decoder preserves both record boundaries");
}

static double axis_oracle(unsigned sample) {
    /* Piecewise packet-domain oracle, independently expressed in byte ranges. */
    if (sample >= 79 && sample <= 175) return 0.0;
    if (sample < 79) return sample <= 3 ? -1.0 : ((double)sample - 79.0) / 76.0;
    return sample >= 251 ? 1.0 : ((double)sample - 175.0) / 76.0;
}

static void test_axis_boundaries_and_pressure(void) {
    InputRecord5C0 record;
    u8 packet[20];
    unsigned channel, sample;
    for (channel = 0; channel < 4; channel++) {
        for (sample = 0; sample <= 255; sample++) {
            reset(&record, packet);
            packet[4 + channel] = (u8)sample;
            decode(&record, packet, 8);
            near_value(record.f100[channel], axis_oracle(sample), 0.0000001,
                       "all 256 byte values obey the recovered axis thresholds and saturation");
            if (channel >= 2)
                near_value(record.f2D0[0], fabs(axis_oracle(sample)), 0.0000001,
                           "single-axis magnitude agrees with an independent packet-domain oracle");
        }
    }
    reset(&record, packet);
    packet[4] = packet[6] = 255;
    decode(&record, packet, 7);
    check(record.f100[0] == 0 && record.f100[2] == 0, "short packet does not decode sticks");
    reset(&record, packet);
    for (channel = 4; channel < 16; channel++) packet[4 + channel] = (u8)((channel - 4) * 23);
    decode(&record, packet, 19);
    for (channel = 4; channel < 16; channel++) check(record.f100[channel] == 0, "length 19 omits pressure");
    decode(&record, packet, 20);
    for (channel = 4; channel < 16; channel++)
        near_value(record.f100[channel], (double)packet[4 + channel] / 255.0, 0.0000001,
                   "all pressure channels use their own packet byte and normalization");
    packet[8] = 255;
    decode(&record, packet, 20);
    check(record.f100[4] == 1.0f, "maximum pressure reaches one");
}

static void test_sample_delays_and_history(void) {
    InputRecord5C0 record;
    u8 packet[20];
    unsigned delay, frame, expected, previous;
    for (delay = 0; delay <= 6; delay++) {
        reset(&record, packet);
        record.f568 = (s32)delay;
        previous = 0;
        for (frame = 0; frame < 18; frame++) {
            buttons(packet, frame + 1);
            packet[4] = (u8)(176 + frame);
            decode(&record, packet, 8);
            expected = frame < delay ? 0 : frame - delay + 1;
            check((unsigned)record.f1A0 == expected && (unsigned)record.f1B0 == expected,
                  "each delay selects its oldest expected logical/raw mask across ring wraps");
            check((unsigned)record.f1A4 == (~previous & expected) &&
                  (unsigned)record.f1A8 == (~expected & previous),
                  "delay preserves logical press/release transitions");
            near_value(record.f100[0], frame < delay ? 0.0 : axis_oracle(176 + frame - delay),
                       0.0000001, "button and axis samples use the same delayed ring position");
            check(record.f34C == (s32)((frame + 1) % 7), "seven-sample write ring wraps exactly");
            previous = expected;
        }
        check(record.f350 == (s32)delay, "warmup count saturates at the configured delay");
    }
    reset(&record, packet);
    for (frame = 0; frame < 45; frame++) decode(&record, packet, 20);
    check(record.f190 == 30 && record.f18E == 15, "30-sample history wraps and saturates");
    D_0021DDB4_[0] = -1;
    decode(&record, packet, 20);
    check(record.f190 == 30 && record.f18E == 15, "state minus one pauses history advancement");
}

static void test_direction_modes_and_flip(void) {
    InputRecord5C0 record;
    u8 packet[20];
    reset(&record, packet);
    packet[6] = 255;
    packet[7] = 0;
    decode(&record, packet, 8);
    check(record.f1A0 == 0x3000 && record.f1B0 == 0, "selected stick augments logical but not raw held");
    check(record.f2D0[0] > 1.4f && record.f2D0[0] < 1.42f, "real scalar magnitude uses both axes");
    near_value(record.f258[0], -0.7853981633974483, 0.000003, "decoder calls the actual quadrant helper");
    check((record.f1A4 & 0x10000) != 0 && (record.f1C4 & 0x10000) == 0,
          "strong-stick history edge follows the earlier pressed snapshot");
    reset(&record, packet);
    D_0021DE31[0] = 1;
    D_0021DDB4_[0] = 7;
    packet[4] = packet[6] = 255;
    decode(&record, packet, 8);
    check(record.f100[0] == -1 && record.f100[2] == -1 && record.f140[0] == 1 && record.f140[2] == 1,
          "flip applies to selected axes and reverses the separate copied pair");
    check(record.f354[0] == 0x8000 && record.f370[0] == 0 && record.f1B0 == 0,
          "state seven augments only the logical sampled mask");
    reset(&record, packet);
    buttons(packet, 0xFFFF);
    packet[4] = packet[5] = 255;
    record.f1CC = 1;
    D_0021E8A0[0] = 1;
    decode(&record, packet, 8);
    check(record.f1A0 == (0xFFFF & ~0x5030) && record.f1C0 == 0xFFFF && record.f348 == 0xFFFF,
          "mode one masks current buttons after preserving snapshots");
    check(record.f100[0] == 0 && record.f100[1] == 0 && record.f140[0] == 1 && record.f1CC == 0,
          "mode one clears selected axes and honors the actual L01 reset flag");
    reset(&record, packet);
    buttons(packet, 0xFFFF);
    packet[6] = packet[7] = 255;
    record.f1CC = 2;
    decode(&record, packet, 8);
    check(record.f1A0 == 0x900 && record.f1B0 == 0x900 && record.f1A4 == 0x900,
          "mode two retains only its recovered button mask");
    check(record.f100[2] == 0 && record.f100[3] == 0 && record.f140[2] == 1,
          "mode two clears magnitude inputs after making the unmasked copy");
}

static void test_repeat_order(void) {
    InputRecord5C0 record;
    u8 packet[20];
    unsigned frame;
    reset(&record, packet);
    record.f54D = 2;
    record.f54E = 3;
    buttons(packet, 1);
    decode(&record, packet, 4);
    check(record.f1A4 == 1 && record.f54F == 2, "new press starts first-repeat delay");
    for (frame = 1; frame <= 5; frame++) {
        decode(&record, packet, 4);
        check(record.f1A4 == (frame == 2 || frame == 5 ? 1 : 0),
              "held repeat honors first delay and subsequent period");
        check(record.f1C4 == 0 && record.f1E0[(record.f18E + 29) % 30] == 0,
              "repeat changes final pressed after snapshot and history writes");
    }
    record.f54F = 0;
    decode(&record, packet, 4);
    check(record.f1A4 == 1 && record.f54F == 3, "byte-countdown underflow triggers repeat");
    buttons(packet, 0);
    decode(&record, packet, 4);
    check(record.f1A8 == 1 && record.f54F == 2, "release restores first-repeat delay");
}

static void test_angular_history_transition(void) {
    InputRecord5C0 record;
    u8 packet[20];
    unsigned i;
    reset(&record, packet);
    /* Real helper values establish an already strong, steady history. This
     * avoids the separate weak-to-strong magnitude trigger in this fixture.
     */
    for (i = 0; i < 30; i++) {
        record.f2D0[i] = 1.0f;
        record.f258[i] = func_L01_0055D708(1.0f, 0.0f);
    }
    packet[6] = 255;
    decode(&record, packet, 8);
    check((record.f1A4 & 0x10000) == 0, "steady strong direction does not create an angular flick");
    packet[6] = 127;
    packet[7] = 255;
    decode(&record, packet, 8);
    check((record.f1A4 & 0x10000) != 0,
          "actual circular-distance helper detects a strong quarter-turn history change");
    check((record.f1E0[1] & 0x10000) != 0 && (record.f1C4 & 0x10000) == 0,
          "angular flick reaches pressed history after the earlier snapshot");
}

int main(void) {
    test_actual_angle_helpers();
    test_neutral_and_button_edges();
    test_axis_boundaries_and_pressure();
    test_sample_delays_and_history();
    test_direction_modes_and_flip();
    test_repeat_order();
    test_angular_history_transition();
    printf("Dreadzone actual packet decoder/math: %u checks passed; greatest angle error %.9g rad.\n",
           checks, greatest_angle_error);
    return 0;
}
