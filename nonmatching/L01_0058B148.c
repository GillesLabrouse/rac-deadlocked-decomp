#include "common.h"

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_model_pool.h"
#include <stdint.h>
#include <string.h>

u32 D_00222D78, D_00222D7C, D_00222D80;
s32 D_00222D84, D_00222D88, D_00222D8C, D_00222D90, D_00222D94;
s32 D_00222D98, D_00222D9C;

static DreadzoneModelView model_views[16];
static u32 model_view_count;

u32 dreadzone_model_read32(const u8 *bytes) {
    return (u32)bytes[0] | ((u32)bytes[1] << 8) | ((u32)bytes[2] << 16) | ((u32)bytes[3] << 24);
}
void dreadzone_model_write32(u8 *bytes, u32 value) {
    u32 i;
    for (i = 0; i < 4; i++) bytes[i] = (u8)(value >> (i * 8));
}
u16 dreadzone_model_read16(const u8 *bytes) {
    return (u16)((u32)bytes[0] | ((u32)bytes[1] << 8));
}
void dreadzone_model_write16(u8 *bytes, u16 value) {
    bytes[0] = (u8)value;
    bytes[1] = (u8)(value >> 8);
}

static int overlap(u32 a, u32 an, u32 b, u32 bn) {
    return (uint64_t)a < (uint64_t)b + bn && (uint64_t)b < (uint64_t)a + an;
}

int dreadzone_model_pool_bind(const DreadzoneModelView *views, u32 count) {
    u32 i, j;
    model_view_count = 0;
    if (views == NULL || count == 0 || count > 16) return DREADZONE_MODEL_UNMAPPED;
    for (i = 0; i < count; i++) {
        if (views[i].storage == NULL || views[i].bytes == 0 ||
            (uint64_t)views[i].guest_base + views[i].bytes > UINT64_C(0x100000000) ||
            (uintptr_t)views[i].storage > UINTPTR_MAX - views[i].bytes) return DREADZONE_MODEL_UNMAPPED;
        for (j = 0; j < i; j++) {
            if (overlap(views[i].guest_base, views[i].bytes, views[j].guest_base, views[j].bytes) ||
                ((uintptr_t)views[i].storage < (uintptr_t)views[j].storage + views[j].bytes &&
                 (uintptr_t)views[j].storage < (uintptr_t)views[i].storage + views[i].bytes)) {
                return DREADZONE_MODEL_CORRUPT;
            }
        }
    }
    memcpy(model_views, views, count * sizeof(*views));
    model_view_count = count;
    return DREADZONE_MODEL_OK;
}

u8 *dreadzone_model_resolve(u32 address, u32 bytes) {
    u32 i;
    for (i = 0; i < model_view_count; i++) {
        if (address >= model_views[i].guest_base &&
            (uint64_t)address + bytes <= (uint64_t)model_views[i].guest_base + model_views[i].bytes) {
            return model_views[i].storage + (address - model_views[i].guest_base);
        }
    }
    return NULL;
}

static int outside_pool(u32 address, u32 bytes) {
    return !overlap(address, bytes, D_00222D7C, DREADZONE_MODEL_MATRIX_BYTES) &&
           !overlap(address, bytes, D_00222D78, DREADZONE_MODEL_DESCRIPTOR_BYTES);
}

int dreadzone_model_pool_validate(void) {
    u8 *descriptors;
    u32 next = 0, freed_records = 0, freed_descriptors = 0;
    s32 first_freed = 0x7FFFFFFF, i;
    if ((D_00222D7C & 15) != 0 || (D_00222D78 & 7) != 0 ||
        D_00222D8C < 0 || D_00222D8C > DREADZONE_MODEL_DESCRIPTORS ||
        D_00222D84 < 0 || D_00222D84 > DREADZONE_MODEL_RECORDS ||
        overlap(D_00222D7C, DREADZONE_MODEL_MATRIX_BYTES, D_00222D78, DREADZONE_MODEL_DESCRIPTOR_BYTES)) {
        return DREADZONE_MODEL_CORRUPT;
    }
    descriptors = dreadzone_model_resolve(D_00222D78, DREADZONE_MODEL_DESCRIPTOR_BYTES);
    if (descriptors == NULL || dreadzone_model_resolve(D_00222D7C, DREADZONE_MODEL_MATRIX_BYTES) == NULL) {
        return DREADZONE_MODEL_UNMAPPED;
    }
    for (i = 0; i < D_00222D8C; i++) {
        u8 *record = descriptors + i * 8;
        u32 actor_address = dreadzone_model_read32(record);
        u32 first = dreadzone_model_read16(record + 4);
        u32 count = record[6];
        if (first != next || count == 0 || next + count > DREADZONE_MODEL_RECORDS || record[7] > 1) {
            return DREADZONE_MODEL_CORRUPT;
        }
        next += count;
        if (record[7] != 0) {
            if (first_freed == 0x7FFFFFFF) first_freed = i;
            freed_records += count;
            freed_descriptors++;
        } else {
            u8 *actor = dreadzone_model_resolve(actor_address, 0x5C);
            u32 definition_address;
            u8 *definition;
            if (actor == NULL) return DREADZONE_MODEL_UNMAPPED;
            definition_address = dreadzone_model_read32(actor + 0x24);
            definition = dreadzone_model_resolve(definition_address, 9);
            if (definition == NULL) return DREADZONE_MODEL_UNMAPPED;
            if (!outside_pool(actor_address, 0x5C) || !outside_pool(definition_address, 9) ||
                definition[8] != count || dreadzone_model_read32(actor + 0x58) != D_00222D7C + first * 64) {
                return DREADZONE_MODEL_CORRUPT;
            }
        }
    }
    if (D_00222D80 != D_00222D7C + next * 64 ||
        D_00222D84 != (s32)(DREADZONE_MODEL_RECORDS - next) ||
        D_00222D88 != (s32)freed_records || D_00222D90 != (s32)freed_descriptors ||
        D_00222D94 != first_freed) return DREADZONE_MODEL_CORRUPT;
    return DREADZONE_MODEL_OK;
}

int dreadzone_model_pool_allocate(u32 actor_address, u32 *matrices) {
    u8 *actor = dreadzone_model_resolve(actor_address, 0x5C);
    u8 *definition, *descriptor;
    u32 definition_address, count;
    s32 i;
    int status;
    if (actor == NULL || matrices == NULL) return DREADZONE_MODEL_UNMAPPED;
    definition_address = dreadzone_model_read32(actor + 0x24);
    definition = dreadzone_model_resolve(definition_address, 9);
    if (definition == NULL) return DREADZONE_MODEL_UNMAPPED;
    count = definition[8];
    if (count == 0 || !outside_pool(actor_address, 0x5C) || !outside_pool(definition_address, 9)) {
        return DREADZONE_MODEL_CORRUPT;
    }
    status = dreadzone_model_pool_validate();
    if (status != 0) return status;
    for (i = 0; i < D_00222D8C; i++) {
        u8 *record = dreadzone_model_resolve(D_00222D78 + (u32)i * 8, 8);
        if (record[7] == 0 && dreadzone_model_read32(record) == actor_address) return DREADZONE_MODEL_CORRUPT;
    }
    if (D_00222D84 < (s32)count) {
        if (D_00222D84 + D_00222D88 < (s32)count) return DREADZONE_MODEL_CAPACITY;
        status = dreadzone_model_pool_compact();
        if (status != 0) return status;
    }
    if (D_00222D8C == DREADZONE_MODEL_DESCRIPTORS) return DREADZONE_MODEL_CAPACITY;
    dreadzone_model_write32(actor + 0x58, D_00222D80);
    D_00222D80 += count * 64;
    D_00222D84 -= (s32)count;
    descriptor = dreadzone_model_resolve(D_00222D78 + (u32)D_00222D8C * 8, 8);
    dreadzone_model_write32(descriptor, actor_address);
    dreadzone_model_write16(descriptor + 4, (u16)((dreadzone_model_read32(actor + 0x58) - D_00222D7C) >> 6));
    descriptor[6] = (u8)count;
    descriptor[7] = 0;
    D_00222D8C++;
    *matrices = dreadzone_model_read32(actor + 0x58);
    return DREADZONE_MODEL_OK;
}
#else
typedef struct ModelActor {
    u8 f0[0x24];
    u8 *f24;
    u8 f28[0x30];
    u8 *f58;
} ModelActor;
typedef struct ModelDescriptor {
    ModelActor *f0;
    u16 f4;
    u8 f6, f7;
} ModelDescriptor;
extern s32 D_00222D84 __attribute__((section(".data")));
extern u8 *D_00222D80;
extern s32 D_00222D8C __attribute__((section(".data")));
extern ModelDescriptor *D_00222D78 __attribute__((section(".data")));
extern u8 *D_00222D7C __attribute__((section(".data")));
extern void func_L01_0058B338(void);

u8 *func_L01_0058B148(ModelActor *actor) {
    s32 count = actor->f24[8];
    if (D_00222D84 < count) func_L01_0058B338();
    actor->f58 = D_00222D80;
    D_00222D84 -= count;
    D_00222D80 += count * 64;
    D_00222D78[D_00222D8C].f0 = actor;
    D_00222D78[D_00222D8C].f4 = (s32)(actor->f58 - D_00222D7C) >> 6;
    D_00222D78[D_00222D8C].f6 = count;
    D_00222D78[D_00222D8C].f7 = 0;
    D_00222D8C++;
    return actor->f58;
}
#endif
