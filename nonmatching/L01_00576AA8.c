#include "common.h"

#ifndef DEADLOCKED_NATIVE
typedef struct GeometryHeader {
    u8 *f0;
    s32 f4;
    u32 f8, fC;
} GeometryHeader;
extern void func_L01_00576348(u8 *, u8 *, s32, s32, s32, s32, s32);
extern void func_L01_00576650(u8 *, s32, s32, s32, s32, s32);

void func_L01_00576AA8(GeometryHeader *headers, u8 *textures, u8 *remap, s32 count) {
    s32 i, quad;
    for (i = 0; i < count; i++, headers++) {
        s32 patch_quads = headers->f4 >> 16;
        s32 quads = headers->f4 & 0xFFFF;
        u8 *record = headers->f0 + ((quads - patch_quads) * 16);
        headers->f4 = quads;
        for (quad = 0; quad < patch_quads; quad += 4, record += 64) {
            s32 index = *(s32 *)(record + 0x20);
            if (index >= 0) index = remap[index];
            if (textures != NULL) {
                func_L01_00576348(record, textures + index * 16,
                    *(s32 *)record, *(s32 *)(record + 4),
                    *(s32 *)(record + 0x10), *(s32 *)(record + 0x14), index);
            } else {
                func_L01_00576650(record, *(s32 *)record, *(s32 *)(record + 4),
                    *(s32 *)(record + 0x10), *(s32 *)(record + 0x14), index);
            }
        }
    }
}
#else
#include "../native/dreadzone_geometry.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct GeometryWrite {
    u32 address, bytes;
    u64 value;
} GeometryWrite;
struct DreadzoneGeometryPlan {
    const DreadzoneGeometryContext *source;
    GeometryWrite *writes;
    size_t count, capacity;
    int error, applied;
    DreadzoneGeometryRead reader;
    void *reader_context;
};

static int context_valid(const DreadzoneGeometryContext *context) {
    u32 i, j;
    if (context == NULL || context->views == NULL || context->view_count == 0 || context->view_count > 32) return 0;
    for (i = 0; i < context->view_count; i++) {
        const DreadzoneGeometryView *a = context->views + i;
        if (a->storage == NULL || a->bytes == 0 || (u64)a->guest_base + a->bytes > ((u64)1 << 32) ||
            (uintptr_t)a->storage > UINTPTR_MAX - a->bytes) return 0;
        for (j = 0; j < i; j++) {
            const DreadzoneGeometryView *b = context->views + j;
            if (((u64)a->guest_base < (u64)b->guest_base + b->bytes && (u64)b->guest_base < (u64)a->guest_base + a->bytes) ||
                ((uintptr_t)a->storage < (uintptr_t)b->storage + b->bytes && (uintptr_t)b->storage < (uintptr_t)a->storage + a->bytes)) return 0;
        }
    }
    return 1;
}
static u8 *resolve(const DreadzoneGeometryContext *context, u32 address, u32 bytes) {
    u32 i;
    for (i = 0; i < context->view_count; i++) {
        const DreadzoneGeometryView *view = context->views + i;
        if (address >= view->guest_base && (u64)address + bytes <= (u64)view->guest_base + view->bytes) {
            return view->storage + (address - view->guest_base);
        }
    }
    return NULL;
}
int dreadzone_geometry_read(DreadzoneGeometryPlan *plan, u32 address, u32 bytes, u64 *value) {
    u8 *source;
    u32 byte;
    if (plan == NULL || value == NULL || bytes == 0 || bytes > 8 || plan->error != 0) return 0;
    if ((bytes == 2 || bytes == 4 || bytes == 8) && (address & (bytes - 1)) != 0) {
        plan->error = DREADZONE_GEOMETRY_INVALID; return 0;
    }
    source = resolve(plan->source, address, bytes);
    if (source == NULL) { plan->error = DREADZONE_GEOMETRY_UNMAPPED; return 0; }
    if (plan->reader != NULL) {
        if (!plan->reader(plan->reader_context, address, bytes, value)) {
            plan->error = DREADZONE_GEOMETRY_UNMAPPED; return 0;
        }
    } else {
        *value = 0;
        for (byte = 0; byte < bytes; byte++) *value |= (u64)source[byte] << (byte * 8);
    }
    for (byte = 0; byte < bytes; byte++) {
        u8 data = (u8)(*value >> (byte * 8));
        size_t index = plan->count;
        while (index != 0) {
            const GeometryWrite *write = plan->writes + --index;
            if ((u64)address + byte >= write->address && (u64)address + byte < (u64)write->address + write->bytes) {
                data = (u8)(write->value >> ((address + byte - write->address) * 8));
                break;
            }
        }
        *value = (*value & ~((u64)0xFF << (byte * 8))) | ((u64)data << (byte * 8));
    }
    return 1;
}
int dreadzone_geometry_write(DreadzoneGeometryPlan *plan, u32 address, u32 bytes, u64 value) {
    GeometryWrite *write;
    if (plan == NULL || bytes == 0 || bytes > 8 || plan->error != 0) return 0;
    if ((bytes == 2 || bytes == 4 || bytes == 8) && (address & (bytes - 1)) != 0) {
        plan->error = DREADZONE_GEOMETRY_INVALID; return 0;
    }
    if (resolve(plan->source, address, bytes) == NULL) { plan->error = DREADZONE_GEOMETRY_UNMAPPED; return 0; }
    if (plan->count == plan->capacity) {
        size_t capacity = plan->capacity == 0 ? 128 : plan->capacity * 2;
        GeometryWrite *expanded;
        if (capacity < plan->capacity || capacity > SIZE_MAX / sizeof(*write)) {
            plan->error = DREADZONE_GEOMETRY_ALLOCATION_FAILED; return 0;
        }
        expanded = (GeometryWrite *)realloc(plan->writes, capacity * sizeof(*write));
        if (expanded == NULL) { plan->error = DREADZONE_GEOMETRY_ALLOCATION_FAILED; return 0; }
        plan->writes = expanded; plan->capacity = capacity;
    }
    write = plan->writes + plan->count++;
    write->address = address; write->bytes = bytes; write->value = value;
    return 1;
}
void dreadzone_geometry_destroy(DreadzoneGeometryPlan *plan) {
    if (plan != NULL) { free(plan->writes); free(plan); }
}
u64 dreadzone_geometry_write_count(const DreadzoneGeometryPlan *plan) {
    return plan == NULL ? 0 : (u64)plan->count;
}
int dreadzone_geometry_write_at(const DreadzoneGeometryPlan *plan, u64 index,
                               u32 *address, u32 *bytes, u64 *value) {
    const GeometryWrite *write;
    if (plan == NULL || index >= plan->count || address == NULL || bytes == NULL || value == NULL) return 0;
    write = plan->writes + (size_t)index;
    *address = write->address; *bytes = write->bytes; *value = write->value;
    return 1;
}
int dreadzone_geometry_apply(DreadzoneGeometryPlan *plan, const DreadzoneGeometryContext *context) {
    size_t i;
    if (plan == NULL) return DREADZONE_GEOMETRY_OK;
    if (plan->error != 0 || plan->applied || !context_valid(context)) return DREADZONE_GEOMETRY_INVALID;
    for (i = 0; i < plan->count; i++) {
        if (resolve(context, plan->writes[i].address, plan->writes[i].bytes) == NULL) return DREADZONE_GEOMETRY_UNMAPPED;
    }
    for (i = 0; i < plan->count; i++) {
        GeometryWrite *write = plan->writes + i;
        u8 *target = resolve(context, write->address, write->bytes);
        u32 byte;
        for (byte = 0; byte < write->bytes; byte++) target[byte] = (u8)(write->value >> (byte * 8));
    }
    plan->applied = 1;
    return DREADZONE_GEOMETRY_OK;
}
int dreadzone_geometry_prepare_with_reader(const DreadzoneGeometryContext *context,
                              DreadzoneGeometryRead reader, void *reader_context, u32 headers,
                              u32 textures, u32 remap, s32 header_count,
                              DreadzoneGeometryPlan **output) {
    DreadzoneGeometryPlan *plan;
    s32 i;
    if (output == NULL) return DREADZONE_GEOMETRY_INVALID;
    *output = NULL;
    if (header_count <= 0) return DREADZONE_GEOMETRY_OK;
    if (!context_valid(context)) return DREADZONE_GEOMETRY_INVALID;
    if ((u64)headers + (u64)(u32)header_count * 16 > ((u64)1 << 32)) return DREADZONE_GEOMETRY_UNMAPPED;
    plan = (DreadzoneGeometryPlan *)calloc(1, sizeof(*plan));
    if (plan == NULL) return DREADZONE_GEOMETRY_ALLOCATION_FAILED;
    plan->source = context;
    plan->reader = reader; plan->reader_context = reader_context;
    for (i = 0; i < header_count && plan->error == 0; i++) {
        u32 header = headers + (u32)i * 16, record, low, quad;
        s32 high;
        u64 packed, base;
        if (!dreadzone_geometry_read(plan, header + 4, 4, &packed) || !dreadzone_geometry_read(plan, header, 4, &base)) break;
        high = (s16)((u32)packed >> 16); low = (u32)packed & 0xFFFF;
        record = (u32)base + (low - (u32)high) * 16;
        if (!dreadzone_geometry_write(plan, header + 4, 4, low)) break;
        for (quad = 0; high > 0 && quad < (u32)high && plan->error == 0; quad += 4, record += 64) {
            u64 value, words[4];
            s32 index;
            if (!dreadzone_geometry_read(plan, record + 0x20, 4, &value)) break;
            index = (s32)(u32)value;
            if (index >= 0) {
                if (!dreadzone_geometry_read(plan, remap + (u32)index, 1, &value)) break;
                index = (s32)value;
            }
            if (!dreadzone_geometry_read(plan, record, 4, words) ||
                !dreadzone_geometry_read(plan, record + 4, 4, words + 1) ||
                !dreadzone_geometry_read(plan, record + 0x10, 4, words + 2) ||
                !dreadzone_geometry_read(plan, record + 0x14, 4, words + 3)) break;
            if (textures != 0) {
                if (!dreadzone_geometry_emit_textured(plan, record, textures + (u32)index * 16,
                    (s32)(u32)words[0], (s32)(u32)words[1], (s32)(u32)words[2], (s32)(u32)words[3], index)) break;
            } else {
                if (!dreadzone_geometry_emit_table(plan, record, (s32)(u32)words[0],
                    (s32)(u32)words[1], (s32)(u32)words[2], (s32)(u32)words[3], index)) break;
            }
        }
    }
    if (plan->error != 0) { int error = plan->error; dreadzone_geometry_destroy(plan); return error; }
    *output = plan;
    return DREADZONE_GEOMETRY_OK;
}
int dreadzone_geometry_prepare(const DreadzoneGeometryContext *context, u32 headers,
                              u32 textures, u32 remap, s32 header_count,
                              DreadzoneGeometryPlan **output) {
    return dreadzone_geometry_prepare_with_reader(context, NULL, NULL, headers, textures, remap, header_count, output);
}
int dreadzone_geometry_patch(const DreadzoneGeometryContext *context, u32 headers,
                            u32 textures, u32 remap, s32 header_count) {
    DreadzoneGeometryPlan *plan;
    int status = dreadzone_geometry_prepare(context, headers, textures, remap, header_count, &plan);
    if (status != 0) return status;
    status = dreadzone_geometry_apply(plan, context);
    dreadzone_geometry_destroy(plan);
    return status;
}
#endif
