#include "dreadzone_model_transform.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#if FLT_RADIX != 2 || FLT_MANT_DIG != 24 || FLT_MAX_EXP != 128
#error Model transform requires IEEE binary32 host floats
#endif
typedef char transform_float_size_check[sizeof(float) == 4 ? 1 : -1];

static u32 read32(const u8 *p) {
    return (u32)p[0] | (u32)p[1] << 8 | (u32)p[2] << 16 | (u32)p[3] << 24;
}
static void write_float(u8 *p, float value) {
    u32 bits;
    memcpy(&bits, &value, 4);
    p[0] = (u8)bits; p[1] = (u8)(bits >> 8);
    p[2] = (u8)(bits >> 16); p[3] = (u8)(bits >> 24);
}
static float read_float(const u8 *p) {
    u32 bits = read32(p);
    float value;
    memcpy(&value, &bits, 4);
    return value;
}
static int valid_float(float value) {
    return isfinite(value) && (value == 0.0f || fabsf(value) >= FLT_MIN);
}
static int overlap(uint64_t a, uint64_t n, uint64_t b, uint64_t m) {
    return a < b + m && b < a + n;
}
static u8 *resolve(const DreadzoneModelView *views, u32 count, u32 address, u32 bytes) {
    u32 i;
    for (i = 0; i < count; i++) {
        if (address >= views[i].guest_base &&
            (uint64_t)address + bytes <= (uint64_t)views[i].guest_base + views[i].bytes)
            return views[i].storage + (address - views[i].guest_base);
    }
    return NULL;
}
static void identity(u8 *record) {
    u32 column, row;
    for (column = 0; column < 4; column++)
        for (row = 0; row < 4; row++)
            write_float(record + column * 16 + row * 4, column == row ? 1.0f : 0.0f);
}

int dreadzone_model_transform_static(const DreadzoneModelTransform *request) {
    DreadzoneModelTransform input;
    DreadzoneModelView views[16];
    u8 work[DREADZONE_TRANSFORM_SCRATCH_BYTES];
    u8 *actor, *definition, *animation, *stream, *skeleton, *matrices;
    u32 i, j, count, stream_bytes, skeleton_address, output_address;
    if (request == NULL || (uintptr_t)request > UINTPTR_MAX - (sizeof(*request) - 1))
        return DREADZONE_TRANSFORM_INVALID;
    input = *request;
    if (input.actor == 0) return DREADZONE_TRANSFORM_NO_MODEL;
    if (input.views == NULL || input.view_count == 0 || input.view_count > 16 ||
        (uintptr_t)input.views > UINTPTR_MAX - input.view_count * sizeof(*views) ||
        input.scratch == NULL || input.scratch_bytes < sizeof(work) ||
        (uintptr_t)input.scratch > UINTPTR_MAX - sizeof(work))
        return DREADZONE_TRANSFORM_INVALID;
    memcpy(views, input.views, input.view_count * sizeof(*views));
    for (i = 0; i < input.view_count; i++) {
        if (views[i].storage == NULL || views[i].bytes == 0 ||
            (uint64_t)views[i].guest_base + views[i].bytes > UINT64_C(0x100000000) ||
            overlap(views[i].guest_base, views[i].bytes, UINT64_C(0x70000000), sizeof(work)) ||
            (uintptr_t)views[i].storage > UINTPTR_MAX - views[i].bytes ||
            overlap((uintptr_t)views[i].storage, views[i].bytes,
                (uintptr_t)input.scratch, sizeof(work))) return DREADZONE_TRANSFORM_INVALID;
        for (j = 0; j < i; j++)
            if (overlap(views[i].guest_base, views[i].bytes, views[j].guest_base, views[j].bytes) ||
                overlap((uintptr_t)views[i].storage, views[i].bytes,
                    (uintptr_t)views[j].storage, views[j].bytes)) return DREADZONE_TRANSFORM_INVALID;
    }
    actor = resolve(views, input.view_count, input.actor, 0x60);
    if (actor == NULL) return DREADZONE_TRANSFORM_UNMAPPED;
    output_address = read32(actor + 0x58);
    if (output_address == 0) return DREADZONE_TRANSFORM_NO_MODEL;
    if ((actor[0x55] & 4) != 0 || read32(actor + 0x50) != 0 || read32(actor + 0x5C) != 0)
        return DREADZONE_TRANSFORM_UNSUPPORTED;
    definition = resolve(views, input.view_count, read32(actor + 0x24), 0x1C);
    animation = resolve(views, input.view_count, read32(actor + 0x40), 0x20);
    if (definition == NULL || animation == NULL) return DREADZONE_TRANSFORM_UNMAPPED;
    count = definition[8];
    if (count == 0 || count > 127 || actor[0x57] != count || animation[0x10] == 0 ||
        !valid_float(read_float(actor + 0x44))) return DREADZONE_TRANSFORM_UNSUPPORTED;
    stream = resolve(views, input.view_count, read32(animation + 0x1C), 16);
    if (stream == NULL) return DREADZONE_TRANSFORM_UNMAPPED;
    if (stream[2] != 0) return DREADZONE_TRANSFORM_UNSUPPORTED;
    stream_bytes = (u32)stream[1] * 16;
    if (stream_bytes > 0x2000) return DREADZONE_TRANSFORM_UNSUPPORTED;
    if (stream_bytes != 0) {
        stream = resolve(views, input.view_count, read32(animation + 0x1C), stream_bytes);
        if (stream == NULL) return DREADZONE_TRANSFORM_UNMAPPED;
    }
    skeleton_address = read32(definition + 0x18);
    skeleton = resolve(views, input.view_count, skeleton_address, count * 16);
    matrices = resolve(views, input.view_count, output_address, count * 64);
    if (skeleton == NULL || matrices == NULL) return DREADZONE_TRANSFORM_UNMAPPED;
    if ((skeleton_address & 15) != 0 || (read32(animation + 0x1C) & 15) != 0 ||
        (output_address & 15) != 0 ||
        overlap(output_address, count * 64, input.actor, 0x60) ||
        overlap(output_address, count * 64, read32(actor + 0x24), 0x1C) ||
        overlap(output_address, count * 64, read32(actor + 0x40), 0x20) ||
        overlap(output_address, count * 64, skeleton_address, count * 16) ||
        overlap(output_address, count * 64, read32(animation + 0x1C), stream_bytes ? stream_bytes : 16))
        return DREADZONE_TRANSFORM_INVALID;
    memcpy(work, input.scratch, sizeof(work));
    /* 00583580/00589368: source16 => quaternion, scale, translation32 per joint. */
    memcpy(work + 0x2000, skeleton, count * 16);
    for (i = 0; i < count; i++) {
        u8 *record = work + i * 64;
        u8 *source = skeleton + i * 16;
        u32 parent = source[12] & 127;
        if (parent != 127 && (i == 0 || parent >= i)) return DREADZONE_TRANSFORM_UNSUPPORTED;
        for (j = 0; j < 4; j++) {
            write_float(record + j * 4, j == 3 ? 1.0f : 0.0f);
            write_float(record + 16 + j * 4, j == 3 ? 0.0f : 268435456.0f);
        }
        memcpy(record + 32, source, 16);
        for (j = 0; j < 3; j++) {
            float value = read_float(source + j * 4);
            volatile float scaled = value * 65536.0f;
            if (!valid_float(value) || !valid_float(scaled)) return DREADZONE_TRANSFORM_UNSUPPORTED;
            write_float(record + 32 + j * 4, scaled);
        }
        memcpy(record + 48, record + 32, 16);
    }
    /* 0058A258 takes this complete branch without 005894F0 when byte2 == 0. */
    if (stream_bytes != 0) memcpy(work + 0x2000, stream, stream_bytes);
    identity(work + 0x3FC0);
    /* VU0 0738/0858: initialized q=(0,0,0,1), scale=2^28. Its normalized
     * quaternion basis is identity and its scale becomes1. Both parent banks
     * therefore retain identity bases; the actual parent translation is added
     * in the VU multiply/add order, including the initial *2^16 then *2^-16.
     * This is a proven specialization of the supported static-only path,
     * not an identity-pose replacement for a compressed animated stream. */
    for (i = 0; i < count; i++) {
        u8 *record = work + i * 64;
        u32 parent = skeleton[i * 16 + 12] & 127;
        const u8 *parent_record = work + 0x2000 + parent * 64;
        float translation[3];
        for (j = 0; j < 3; j++) {
            volatile float local = read_float(record + 32 + j * 4) * (1.0f / 65536.0f);
            volatile float value = local + read_float(parent_record + 48 + j * 4);
            if (!valid_float(value)) return DREADZONE_TRANSFORM_UNSUPPORTED;
            translation[j] = value;
        }
        identity(record);
        for (j = 0; j < 3; j++) write_float(record + 48 + j * 4, translation[j]);
        memcpy(work + 0x2000 + i * 64, record, 64);
    }
    memcpy(matrices, work, count * 64);
    memcpy(input.scratch, work, sizeof(work));
    actor[0x55] &= 0xDF;
    return DREADZONE_TRANSFORM_APPROXIMATE;
}
