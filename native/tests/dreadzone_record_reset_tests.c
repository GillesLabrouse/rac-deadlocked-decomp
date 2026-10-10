#include "../dreadzone_record_reset.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DreadzoneResetStorage D_0037B320;
u32 D_00220844;
u32 D_0037F428[7];

/* Required storage for the real pool source that also supplies word filling. */
DreadzonePoolRecord D_00375720[DREADZONE_POOL_COUNT];
u32 D_00220828;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "Check failed at line %d: %s\n", __LINE__, #condition); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

static void check_reset(unsigned char seed) {
    unsigned char expected[0x340];
    unsigned index;
    const u32 invalid = 0xFFFFFFFFU;
    const s16 invalid_half = -1;
    const f32 default_float = 40.0f;

    memset(&D_0037B320, seed, sizeof(D_0037B320));
    D_00220844 = 0x76543210;
    memset(D_0037F428, seed, sizeof(D_0037F428));
    memset(D_00375720, 0xB7, sizeof(D_00375720));
    D_00220828 = 0x12345678;

    func_L01_006AF588();

    /* Independent byte oracle: retail fills the whole 0x4100-byte range,
     * then overrides exactly these five nonzero fields in all twenty records.
     * Compare gaps as well as named fields to catch short fills or bad strides.
     */
    memset(expected, 0, sizeof(expected));
    memcpy(expected, &invalid, sizeof(invalid));
    memcpy(expected + 0x2DC, &invalid_half, sizeof(invalid_half));
    memcpy(expected + 0x2E4, &invalid_half, sizeof(invalid_half));
    memcpy(expected + 0x2E8, &default_float, sizeof(default_float));
    memcpy(expected + 0x2EC, &default_float, sizeof(default_float));
    for (index = 0; index < 20; index++) {
        CHECK(memcmp((const u8 *)&D_0037B320 + index * 0x340,
                     expected, sizeof(expected)) == 0);
    }
    CHECK(D_00220844 == 0);
    for (index = 0; index < 7; index++) {
        CHECK(D_0037F428[index] == 0);
    }
    CHECK(D_00220828 == 0x12345678);
    for (index = 0; index < sizeof(D_00375720); index++) {
        CHECK(((const u8 *)D_00375720)[index] == 0xB7);
    }
}

int main(void) {
    check_reset(0xA5);
    check_reset(0xFF);
    check_reset(0);
    CHECK(sizeof(DreadzoneResetStorage) == 0x4100);
    puts("Dreadzone record reset checks passed.");
    return EXIT_SUCCESS;
}
