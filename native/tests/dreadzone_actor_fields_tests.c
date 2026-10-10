#include "dreadzone_actor_fields.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
static unsigned checks;
static double maximum_error;
static void check(int value, const char *message) {
    checks++;
    if (!value) { fprintf(stderr, "Actor field check failed: %s\n", message); exit(1); }
}
static void write32(u8 *p, u32 value) {
    unsigned i;
    for (i = 0; i < 4; i++) p[i] = (u8)(value >> (8 * i));
}
static u32 read32(const u8 *p) {
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}
static double read_float(const u8 *p) {
    union { u32 bits; f32 value; } copy;
    copy.bits = read32(p);
    return copy.value;
}
static void write_float(u8 *p, f32 value) {
    union { u32 bits; f32 value; } copy;
    copy.value = value;
    write32(p, copy.bits);
}
static DreadzoneActorFieldView view_for(u8 *record, u8 *client, u8 *modes) {
    DreadzoneActorFieldView view;
    memset(&view, 0, sizeof(view));
    view.record = record; view.record_size = 0x31F0; view.record_guest = 0x368F60;
    view.client = client; view.client_size = 0x470; view.client_guest = 0x230280;
    view.mode_bytes_00171DE0 = modes; view.mode_count = 4;
    return view;
}
static void prepare(u8 *record, u8 *client, u8 *modes, u8 pattern, u32 index, u16 flags) {
    memset(record, pattern, 0x31F0); memset(client, pattern, 0x470); memset(modes, pattern, 4);
    write32(record + 0x2F10, index); write32(record + 0x2EFC, 0x230280);
    record[0x1A50] = (u8)flags; record[0x1A51] = (u8)(flags >> 8);
}
/* Independent memory oracle, derived from retail store destinations. Float
 * constants are compared by their recovered bit patterns. Normalized XYZ is
 * checked separately against double mathematics, never counted as VU identity.
 */
static void oracle_stores(u8 *bytes, s32 mode, s32 flag, u16 old_flags) {
    memset(bytes + 0x1A30, 0, 32);
    old_flags = mode == 0 ? (u16)(old_flags | 1) : (u16)(old_flags & 0xFFFE);
    bytes[0x1A50] = (u8)old_flags; bytes[0x1A51] = (u8)(old_flags >> 8);
    if (mode == 0) {
        write32(bytes + 0x1AFC, 0x3F933333); write32(bytes + 0x1A24, 0x3FC6D3F2);
        write32(bytes + 0x1A28, 0xBFC6D3F2); bytes[0x5F4] = bytes[0x5F5] = 0;
        write32(bytes + 0x1AE0, 0);
    } else if (mode == 1) {
        write32(bytes + 0x1AE0, 1); write32(bytes + 0x1A30, 0xC0C00000);
        write32(bytes + 0x1A94, 0x40C00000); write32(bytes + 0x1AFC, 0x40000000);
        write32(bytes + 0x1A24, 0x3F9C61AA); write32(bytes + 0x1A28, 0xBF9C61AA);
    } else if (mode == 2) {
        write32(bytes + 0x1AE0, 2); write32(bytes + 0x1AFC, 0x3F933333);
        write32(bytes + 0x1A24, flag ? 0x3F9C61AA : 0x3FC6D3F2);
        write32(bytes + 0x1A28, flag ? 0xBF9C61AA : 0xBFC6D3F2);
    }
}
static void compare_record(const u8 *actual, const u8 *expected, s32 mode) {
    unsigned i;
    for (i = 0; i < 0x31F0; i++) {
        if (mode && i >= 0x1960 && i < 0x196C) continue;
        check(actual[i] == expected[i], "complete record memory oracle outside approximate XYZ");
    }
}
static void test_mode_zero(void) {
    static const u16 flags[] = {0, 1, 0xAA00, 0xFFFF};
    static const u8 patterns[] = {0, 0x39, 0xA5, 0xFF};
    u8 record[0x31F0], expected[0x31F0], client[0x470], modes[4];
    DreadzoneActorFieldView view;
    unsigned i, j, index, k;
    for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) for (index = 0; index < 4; index++) {
        prepare(record, client, modes, patterns[i], index, flags[j]);
        view = view_for(record, client, modes);
        view.client = NULL; view.client_size = 0; view.client_guest = 0xFFFFFFFFU;
        memcpy(expected, record, sizeof(expected)); oracle_stores(expected, 0, 0, flags[j]);
        check(dreadzone_actor_0068E3B0(&view, 0) == DREADZONE_ACTOR_FIELDS_EXACT_MODE_ZERO,
              "mode0 exact, no client prerequisite");
        compare_record(record, expected, 0);
        for (k = 0; k < 4; k++) check(modes[k] == (k == index ? 0 : patterns[i]), "single global mode byte");
        for (k = 0; k < sizeof(client); k++) check(client[k] == patterns[i], "mode0 does not read/write client");
    }
}
static void test_finite_modes(void) {
    static const s32 modes_to_test[] = {1, 2, 3, -1, INT_MIN, INT_MAX};
    static const f32 vectors[][3] = {
        {3,4,0}, {0,0,0}, {-0.0f,0,-0.0f}, {1,0,0}, {0,-1,0}, {0,0,-1},
        {1,-1,1}, {-3,4,-5}, {0.000000000931322574615478515625f,0,0},
        {1073741824.0f,-1073741824.0f,1073741824.0f},
        {0.000000000931322574615478515625f,1073741824.0f,-1}, {0.125f,-0.25f,0.5f}
    };
    static const u32 w_bits[] = {0, 0x80000000, 0x3F800000, 0x7FC12345};
    struct { u8 before[16], record[0x31F0], after[16]; } guarded;
    u8 client[0x470], original_client[0x470], expected[0x31F0], modes[4];
    DreadzoneActorFieldView view;
    unsigned m, v, flag, axis, i;
    double x, y, z, norm, ideal, error;
    for (m = 0; m < 6; m++) for (v = 0; v < 12; v++) for (flag = 0; flag < 2; flag++) {
        memset(&guarded, 0xA5, sizeof(guarded));
        prepare(guarded.record, client, modes, 0xA5, 2, 0xAAFF);
        view = view_for(guarded.record, client, modes); view.flag_0021E878 = flag ? -1 : 0;
        for (axis = 0; axis < 3; axis++) write_float(client + 0x390 + 4 * axis, vectors[v][axis]);
        write32(client + 0x39C, w_bits[v % 4]);
        memcpy(original_client, client, sizeof(client));
        memcpy(expected, guarded.record, sizeof(expected));
        oracle_stores(expected, modes_to_test[m], view.flag_0021E878, 0xAAFF);
        write32(expected + 0x196C, w_bits[v % 4]);
        check(dreadzone_actor_0068E3B0(&view, modes_to_test[m]) == DREADZONE_ACTOR_FIELDS_APPROXIMATE_VU,
              "nonzero mode advertises approximate VU");
        compare_record(guarded.record, expected, modes_to_test[m]);
        x = vectors[v][0]; y = vectors[v][1]; z = vectors[v][2]; norm = sqrt(x*x + y*y + z*z);
        for (axis = 0; axis < 3; axis++) {
            ideal = norm ? (double)vectors[v][axis] / norm : vectors[v][axis];
            error = fabs(read_float(guarded.record + 0x1960 + 4 * axis) - ideal);
            if (error > maximum_error) maximum_error = error;
            check(error <= 2e-7, "finite normalization agrees with double oracle within tolerance");
            if (vectors[v][axis] == 0.0f)
                check(read32(guarded.record + 0x1960 + 4 * axis) == read32(client + 0x390 + 4 * axis),
                      "signed zero is retained, including all-zero vector");
        }
        for (i = 0; i < 4; i++) check(modes[i] == (i == 2 && modes_to_test[m] <= 2 && modes_to_test[m] >= 1 ?
                                                    (u8)modes_to_test[m] : 0xA5), "mode1/2 global byte only");
        check(memcmp(client, original_client, sizeof(client)) == 0, "client is read only");
        for (i = 0; i < 16; i++) check(guarded.before[i] == 0xA5 && guarded.after[i] == 0xA5, "record guards preserved");
    }
}
static void test_boundaries(void) {
    static const u32 bad_bits[] = {0x7FC12345,0x7F800000,0xFF800000,1,0x80000001,0x307FFFFF,0x4E800001};
    u8 record[0x31F0], original[0x31F0], client[0x470], modes[4];
    DreadzoneActorFieldView view;
    unsigned i;
    int result;
    for (i = 0; i < 14; i++) {
        prepare(record, client, modes, 0xA5, 0, 0xFFFF);
        write_float(client + 0x390, 1); write_float(client + 0x394, 0); write_float(client + 0x398, 0);
        view = view_for(record, client, modes); result = DREADZONE_ACTOR_FIELDS_INVALID_STORAGE;
        if (i == 0) view.record_size--;
        if (i == 1) view.record_guest++;
        if (i == 2) view.record_guest = 0xFFFFFF00;
        if (i == 3) view.mode_bytes_00171DE0 = NULL;
        if (i == 4) view.mode_count = 0xFFFFFFFFU;
        if (i == 5) write32(record + 0x2F10, 0xFFFFFFFFU);
        if (i == 6) write32(record + 0x2F10, 4);
        if (i >= 7) result = DREADZONE_ACTOR_FIELDS_NEEDS_CLIENT;
        if (i == 7) view.client = NULL;
        if (i == 8) view.client_size = 0x39F;
        if (i == 9) view.client_guest += 16;
        if (i == 10) { view.client_guest++; write32(record + 0x2EFC, view.client_guest); }
        if (i == 11) { view.client_guest = 0xFFFFFF00; write32(record + 0x2EFC, view.client_guest); }
        if (i == 12) view.record = NULL;
        if (i == 13) view.mode_count = 0;
        if (i >= 12) result = DREADZONE_ACTOR_FIELDS_INVALID_STORAGE;
        memcpy(original, record, sizeof(record));
        check(dreadzone_actor_0068E3B0(&view, 1) == result, "guest binding/span/alignment boundary");
        check(memcmp(record, original, sizeof(record)) == 0 && modes[0] == 0xA5,
              "invalid storage rejected before mutation");
    }
    for (i = 0; i < sizeof(bad_bits) / sizeof(bad_bits[0]); i++) {
        prepare(record, client, modes, 0x39, 0, 0);
        write32(client + 0x390, bad_bits[i]); write_float(client + 0x394, 1); write_float(client + 0x398, 0);
        memcpy(original, record, sizeof(record)); view = view_for(record, client, modes);
        check(dreadzone_actor_0068E3B0(&view, 2) == DREADZONE_ACTOR_FIELDS_UNSUPPORTED_FLOAT_DOMAIN,
              "nonfinite/outside bounded normalization domain");
        check(memcmp(record, original, sizeof(record)) == 0 && modes[0] == 0x39, "float rejection has no mutation");
    }
    prepare(record, client, modes, 0x39, 0xFFFFFFFFU, 0);
    write_float(client + 0x390, 1); write_float(client + 0x394, 0); write_float(client + 0x398, 0);
    view = view_for(record, client, modes); view.mode_bytes_00171DE0 = NULL; view.mode_count = 0;
    check(dreadzone_actor_0068E3B0(&view, 3) == DREADZONE_ACTOR_FIELDS_APPROXIMATE_VU,
          "other nonzero modes do not access global index/table");
    check(dreadzone_actor_0068E3B0(NULL, 0) == DREADZONE_ACTOR_FIELDS_INVALID_STORAGE, "null view");
}
int main(void) {
    test_mode_zero(); test_finite_modes(); test_boundaries();
    printf("Dreadzone actor fields: %u checks passed; maximum finite normalization error %.9g; no full VU equivalence\n",
           checks, maximum_error);
    return 0;
}
