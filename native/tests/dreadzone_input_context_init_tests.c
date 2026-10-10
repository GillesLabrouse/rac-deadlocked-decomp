#include "dreadzone_input_context_init.h"
#include "dreadzone_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(sizeof(DreadzoneActorInit31F0) == 0x31F0, "Actor size changed");
_Static_assert(offsetof(DreadzoneActorInit31F0, fields.f2FEC) == 0x2FEC, "Context moved");
_Static_assert(sizeof(((DreadzoneActorInit31F0 *)0)->fields.f14) == 4, "Guest vtable must be u32");

/* Real word-fill dependency lives in the shared memory recovery. Its pool
 * globals are storage only; these tests invoke no pool initializer or driver.
 */
DreadzonePoolRecord D_00375720[DREADZONE_POOL_COUNT];
u32 D_00220828;
s32 D_002208A8, D_002208AC;
u8 D_0021DE31[1];
s32 D_0021DDB4_[1], D_0021E8A0[1];
static unsigned checks;

static void check(int condition, const char *message) {
    checks++;
    if (!condition) {
        fprintf(stderr, "Dreadzone context initialization check failed: %s\n", message);
        exit(1);
    }
}

static void test_complete_actor_reset(void) {
    struct {
        u8 before[16];
        DreadzoneActorInit31F0 actor;
        u8 after[16];
    } guarded;
    static const u8 patterns[] = {0, 0xA5, 0xFF, 0x39};
    unsigned pattern, byte;
    for (pattern = 0; pattern < sizeof(patterns); pattern++) {
        u8 expected[0x31F0] = {0};
        const u8 *actual = (const u8 *)&guarded.actor;
        memset(&guarded, patterns[pattern], sizeof(guarded));
        /* Independent complete byte-image oracle from observed writes:
         * every byte is cleared except one guest vtable and two byte fields.
         */
        expected[0x14] = 0xC0;
        expected[0x15] = 0x10;
        expected[0x16] = 0x3B;
        expected[0x266D] = 1;
        expected[0x31CF] = 5;
        func_L01_006A6598(&guarded.actor);
        for (byte = 0; byte < sizeof(expected); byte++)
            check(actual[byte] == expected[byte], "real word clear and sparse writes produce the complete actor byte image");
        for (byte = 0; byte < 16; byte++)
            check(guarded.before[byte] == patterns[pattern] && guarded.after[byte] == patterns[pattern],
                  "whole-actor clear preserves both external guards");
        check(guarded.actor.fields.f2FEC.f3C == 0 && guarded.actor.fields.f2FEC.f1D8 == 0,
              "real actor reset initializes callback cursor and frame counter");
    }
}

static void test_base_constructor_preservation(void) {
    DreadzoneActorInit31F0 actor;
    u8 expected[0x31F0];
    unsigned byte;
    memset(&actor, 0xA5, sizeof(actor));
    memcpy(expected, &actor, sizeof(expected));
    expected[0x14] = 0x18;
    expected[0x15] = 0x07;
    expected[0x16] = 0x3B;
    expected[0x17] = 0;
    check(func_L01_006A93A0(&actor) == &actor, "base constructor returns the same record");
    for (byte = 0; byte < sizeof(expected); byte++)
        check(((const u8 *)&actor)[byte] == expected[byte], "base constructor preserves every byte outside its guest vtable");
    check(func_L01_006A9638(&actor) == &actor && memcmp(&actor, expected, sizeof(expected)) == 0,
          "actual identity helper preserves the complete record");
    check(func_L01_006A9638((u8 *)&actor + 0x1970) == (u8 *)&actor + 0x1970 &&
          memcmp(&actor, expected, sizeof(expected)) == 0,
          "actual base-constructor argument is the opaque subobject at offset1970");
}

static void test_initialized_callback_decoder(void) {
    DreadzoneActorInit31F0 actor;
    InputRecord5C0 input;
    u8 packet[20];
    unsigned delay, frame;
    for (delay = 0; delay <= 6; delay++) {
        u32 previous = 0;
        memset(&actor, 0xA5, sizeof(actor));
        func_L01_006A6598(&actor);
        /* Input-record BSS and packet transport remain controlled fixtures.
         * Actor context initialization above is the actual recovered producer.
         */
        memset(&input, 0, sizeof(input));
        input.f568 = (s32)delay;
        D_002208AC = D_002208A8 = 0;
        for (frame = 0; frame < 24; frame++) {
            u32 held = frame >= 4 + delay && frame < 12 + delay ? 0x800U : 0U;
            memset(packet, 0, sizeof(packet));
            packet[1] = 0x79;
            packet[2] = frame >= 4 && frame < 12 ? 0xF7 : 0xFF;
            packet[3] = 0xFF;
            packet[4] = packet[5] = packet[6] = packet[7] = 128;
            input.f1AC = input.f348;
            input.f1BC = input.f1B0;
            input.f1A0 = input.f1B0 = 0;
            func_L01_006B5CB8(&input, packet, 20, &actor.fields.f2FEC);
            func_L01_005BDA40(&input, packet, 20);
            check((u32)input.f1A0 == held && (u32)input.f1A4 == (held & ~previous) && (u32)input.f1A8 == (previous & ~held),
                  "actual initialized context, callback and decoder preserve delayed press/hold/release");
            check(actor.fields.f2FEC.f1D8 == (s32)frame + 1,
                  "callback starts at real zero frame count and advances safely");
            if (frame % 4 == 3)
                check(actor.fields.f2FEC.f3C == 0 && actor.fields.f2FEC.f1DC >= 0 && actor.fields.f2FEC.f1DC <= 76,
                      "real initialized callback publishes a bounded stream each fourth frame");
            else
                check(actor.fields.f2FEC.f1DC == -1, "incomplete callback groups retain the observed sentinel");
            previous = held;
        }
        check(D_002208AC == 24 && input.f190 == 24,
              "callback statistics and actual decoder history advance together");
        check(actor.fields.f14 == 0x003B10C0 && actor.fields.f266D == 1 && actor.fields.f31CF == 5,
              "callback execution preserves reset-established actor fields");
    }
}

int main(void) {
    test_complete_actor_reset();
    test_base_constructor_preservation();
    test_initialized_callback_decoder();
    printf("Dreadzone real actor/context initializer: %u checks passed.\n", checks);
    return 0;
}
