#include "dreadzone_world_projection.h"

#include <fenv.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

static int ram_span(const DreadzoneWorldProjectionMemory *ram, uint32_t guest, size_t size,
                    size_t *offset) {
    uint64_t map_begin;
    uint64_t map_end;
    uint64_t request_end;
    uintptr_t host_begin;

    if (ram == NULL || ram->bytes == NULL || offset == NULL) {
        return 0;
    }
    map_begin = (uint64_t)ram->guest_base;
    if ((uint64_t)ram->length > UINT64_C(0x100000000) - map_begin) {
        return 0;
    }
    map_end = map_begin + (uint64_t)ram->length;
    request_end = (uint64_t)guest + (uint64_t)size;
    if ((uint64_t)guest < map_begin || request_end > map_end ||
        request_end > UINT64_C(0x100000000)) {
        return 0;
    }
    if ((uint64_t)guest - map_begin > (uint64_t)SIZE_MAX ||
        size > SIZE_MAX - (size_t)((uint64_t)guest - map_begin)) {
        return 0;
    }
    host_begin = (uintptr_t)ram->bytes;
    if (ram->length > UINTPTR_MAX - host_begin) {
        return 0;
    }
    *offset = (size_t)((uint64_t)guest - map_begin);
    return 1;
}

static uint32_t read_le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void write_le32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static int host_ranges_overlap(uintptr_t a, size_t an, uintptr_t b, size_t bn) {
    return a < b + bn && b < a + an;
}

static float bits_to_f32(uint32_t bits) {
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

static uint32_t f32_to_bits(float value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

/* Volatile temporaries preserve a binary32 rounding point at each MIPS op. */
static float f32_mul(float a, float b) {
    volatile float result = a * b;
    return result;
}

static float f32_sub(float a, float b) {
    volatile float result = a - b;
    return result;
}

static float f32_div(float a, float b) {
    volatile float result = a / b;
    return result;
}

static int rounded_word(float value, uint32_t *word) {
    float rounded;
    int32_t signed_value;
    if (!isfinite(value)) {
        return 0;
    }
    rounded = nearbyintf(value);
    if (!isfinite(rounded) || rounded != value ||
        rounded < -2147483648.0f || rounded >= 2147483648.0f) {
        return 0;
    }
    signed_value = (int32_t)rounded;
    *word = (uint32_t)signed_value;
    return 1;
}

int dreadzone_world_projection_00628210(const DreadzoneWorldProjectionMemory *ram) {
    DreadzoneWorldProjectionMemory ram_copy;
    uint8_t *bytes;
    uintptr_t descriptor_begin, backing_begin;
    size_t input_x_offset, input_y_offset, input_z_offset, scale_offset;
    size_t out_x_offset, out_y_offset, out_z_offset, matrix_offset;
    uint32_t x_bits, y_bits, z_bits, scale_bits;
    uint32_t out_x, out_y, out_z;
    uint32_t matrix[16] = {0};
    float matrix_values[16] = {0.0f};
    float x, y, z, scale;
    float sx, sy, sz, reciprocal_x, reciprocal_y;
    float matrix_x, matrix_y, matrix_z, temp;
    size_t i;

    if (ram == NULL) {
        return -1;
    }
    /* Validate the descriptor's numeric address range before reading it. */
    descriptor_begin = (uintptr_t)(const void *)ram;
    if (sizeof(*ram) > UINTPTR_MAX - descriptor_begin) {
        return -1;
    }
    /* Caller must provide readable descriptor storage; C cannot probe it. */
    memcpy(&ram_copy, ram, sizeof(ram_copy));
    bytes = ram_copy.bytes;
    if (bytes == NULL || ram_copy.length == 0 ||
        ram_copy.length > 0x02000000u ||
        ram_copy.guest_base >= 0x80000000u ||
        (uint64_t)ram_copy.length >
            UINT64_C(0x80000000) - (uint64_t)ram_copy.guest_base) {
        return -1;
    }
    backing_begin = (uintptr_t)(void *)bytes;
    if (ram_copy.length > UINTPTR_MAX - backing_begin ||
        host_ranges_overlap(descriptor_begin, sizeof(*ram), backing_begin,
                            ram_copy.length) ||
        fegetround() != FE_TONEAREST ||
        !ram_span(&ram_copy, 0x00223650u, 4, &input_x_offset) ||
        !ram_span(&ram_copy, 0x00223654u, 4, &input_y_offset) ||
        !ram_span(&ram_copy, 0x00223658u, 4, &input_z_offset) ||
        !ram_span(&ram_copy, 0x00240070u, 4, &scale_offset) ||
        !ram_span(&ram_copy, 0x00223660u, 4, &out_x_offset) ||
        !ram_span(&ram_copy, 0x00223664u, 4, &out_y_offset) ||
        !ram_span(&ram_copy, 0x00223668u, 4, &out_z_offset) ||
        !ram_span(&ram_copy, 0x003B9930u, 64, &matrix_offset)) {
        return -1;
    }

    x_bits = read_le32(bytes + input_x_offset);
    y_bits = read_le32(bytes + input_y_offset);
    z_bits = read_le32(bytes + input_z_offset);
    scale_bits = read_le32(bytes + scale_offset);
    x = bits_to_f32(x_bits);
    y = bits_to_f32(y_bits);
    z = bits_to_f32(z_bits);
    scale = bits_to_f32(scale_bits);
    if (!isfinite(x) || !isfinite(y) || !isfinite(z) || !isfinite(scale)) {
        return -1;
    }

    sx = f32_mul(x, scale);
    sy = f32_mul(y, scale);
    sz = f32_mul(z, scale);
    matrix_x = f32_mul(x, 1024.0f);
    matrix_y = f32_mul(y, 1024.0f);
    matrix_z = f32_mul(z, 1024.0f);
    if (!isfinite(sx) || !isfinite(sy) || !isfinite(sz) ||
        !isfinite(matrix_x) || !isfinite(matrix_y) || !isfinite(matrix_z) ||
        !rounded_word(matrix_x, &out_x) || !rounded_word(matrix_y, &out_y) ||
        !rounded_word(matrix_z, &out_z)) {
        return -1;
    }

    temp = f32_sub(sx, sy);
    if (!isfinite(temp) || temp == 0.0f) {
        return -1;
    }
    reciprocal_x = f32_div(1.0f, temp);
    temp = f32_sub(sy, sz);
    if (!isfinite(temp) || temp == 0.0f) {
        return -1;
    }
    reciprocal_y = f32_div(1.0f, temp);
    if (!isfinite(reciprocal_x) || !isfinite(reciprocal_y)) {
        return -1;
    }

    /* Match the source's complete 64-byte zero helper before its matrix stores. */
    matrix_values[0] = f32_mul(reciprocal_x, 0.5f);
    matrix_values[1] = -reciprocal_x;
    matrix_values[3] = sx;
    matrix_values[4] = f32_mul(reciprocal_y, 0.5f);
    matrix_values[5] = -reciprocal_y;
    matrix_values[7] = sy;
    matrix_values[8] = f32_mul(f32_mul(sy, reciprocal_x), -0.5f);
    matrix_values[9] = f32_mul(sx, reciprocal_x);
    matrix_values[12] = f32_mul(f32_mul(sz, reciprocal_y), -0.5f);
    matrix_values[13] = f32_mul(sy, reciprocal_y);
    for (i = 0; i < 16; ++i) {
        if (!isfinite(matrix_values[i])) {
            return -1;
        }
        matrix[i] = f32_to_bits(matrix_values[i]);
    }

    /* No failure path remains: commit the staged integer and matrix writes. */
    write_le32(bytes + out_x_offset, out_x);
    write_le32(bytes + out_y_offset, out_y);
    write_le32(bytes + out_z_offset, out_z);
    for (i = 0; i < 16; ++i) {
        write_le32(bytes + matrix_offset + 4 * i, matrix[i]);
    }
    return 0;
}

