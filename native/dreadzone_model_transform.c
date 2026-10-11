#include "dreadzone_model_transform.h"
#include <stdint.h>
#include <string.h>

static u32 read32(const u8 *p) {
    return (u32)p[0] | (u32)p[1] << 8 | (u32)p[2] << 16 | (u32)p[3] << 24;
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

int dreadzone_model_transform_static(const DreadzoneModelTransform *request) {
    DreadzoneModelTransform input;
    DreadzoneModelView views[16];
    u8 *actor;
    u32 i, j;
    if (request == NULL || (uintptr_t)request > UINTPTR_MAX - (sizeof(*request) - 1))
        return DREADZONE_TRANSFORM_INVALID;
    input = *request;
    if (input.actor == 0) return DREADZONE_TRANSFORM_NO_MODEL;
    if (input.views == NULL || input.view_count == 0 || input.view_count > 16 ||
        (uintptr_t)input.views > UINTPTR_MAX - input.view_count * sizeof(*views) ||
        input.scratch == NULL || input.scratch_bytes < DREADZONE_TRANSFORM_SCRATCH_BYTES ||
        (uintptr_t)input.scratch > UINTPTR_MAX - DREADZONE_TRANSFORM_SCRATCH_BYTES)
        return DREADZONE_TRANSFORM_INVALID;
    memcpy(views, input.views, input.view_count * sizeof(*views));
    for (i = 0; i < input.view_count; i++) {
        if (views[i].storage == NULL || views[i].bytes == 0 ||
            (uint64_t)views[i].guest_base + views[i].bytes > UINT64_C(0x100000000) ||
            overlap(views[i].guest_base, views[i].bytes, UINT64_C(0x70000000), DREADZONE_TRANSFORM_SCRATCH_BYTES) ||
            (uintptr_t)views[i].storage > UINTPTR_MAX - views[i].bytes ||
            overlap((uintptr_t)views[i].storage, views[i].bytes,
                (uintptr_t)input.scratch, DREADZONE_TRANSFORM_SCRATCH_BYTES)) return DREADZONE_TRANSFORM_INVALID;
        for (j = 0; j < i; j++)
            if (overlap(views[i].guest_base, views[i].bytes, views[j].guest_base, views[j].bytes) ||
                overlap((uintptr_t)views[i].storage, views[i].bytes,
                    (uintptr_t)views[j].storage, views[j].bytes)) return DREADZONE_TRANSFORM_INVALID;
    }
    actor = resolve(views, input.view_count, input.actor, 0x60);
    if (actor == NULL) return DREADZONE_TRANSFORM_UNMAPPED;
    if (read32(actor + 0x58) == 0) return DREADZONE_TRANSFORM_NO_MODEL;
    /* 0058A2E0's zero-variable branch targets 0058A314, whose JAL
     * still calls 005894F0. Fixed compressed channels must be decoded before
     * generating matrices. The previous identity specialization omitted this
     * call; retain no successful nonempty-model path until it is closed. */
    return DREADZONE_TRANSFORM_UNSUPPORTED;
}
