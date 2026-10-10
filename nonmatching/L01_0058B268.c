#include "common.h"

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_model_pool.h"

int dreadzone_model_pool_release(u32 actor_address) {
    u8 *actor = dreadzone_model_resolve(actor_address, 0x5C);
    u8 *definition, *record;
    u32 matrices, definition_address, first;
    s32 low, high, middle;
    int status;
    if (actor == NULL) return DREADZONE_MODEL_UNMAPPED;
    matrices = dreadzone_model_read32(actor + 0x58);
    if (matrices == 0) return DREADZONE_MODEL_OK;
    definition_address = dreadzone_model_read32(actor + 0x24);
    if (definition_address == 0) return DREADZONE_MODEL_OK;
    definition = dreadzone_model_resolve(definition_address, 9);
    if (definition == NULL) return DREADZONE_MODEL_UNMAPPED;
    if (definition[8] == 0) return DREADZONE_MODEL_OK;
    status = dreadzone_model_pool_validate();
    if (status != 0) return status;
    if (matrices < D_00222D7C || ((matrices - D_00222D7C) & 63) != 0) return DREADZONE_MODEL_CORRUPT;
    first = (matrices - D_00222D7C) >> 6;
    low = 0;
    high = D_00222D8C;
    middle = high >> 1;
    for (;;) {
        u32 record_first;
        if (low > high || middle < 0 || middle >= D_00222D8C) return DREADZONE_MODEL_CORRUPT;
        record = dreadzone_model_resolve(D_00222D78 + (u32)middle * 8, 8);
        record_first = dreadzone_model_read16(record + 4);
        if (first < record_first) high = middle - 1;
        else if (record_first < first) low = middle + 1;
        else break;
        middle = (high + low) >> 1;
    }
    if (record[7] != 0 || dreadzone_model_read32(record) != actor_address || record[6] != definition[8]) {
        return DREADZONE_MODEL_CORRUPT;
    }
    record[7] = 1;
    D_00222D90++;
    D_00222D88 += definition[8];
    if (middle < D_00222D94) D_00222D94 = middle;
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
extern ModelDescriptor *D_00222D78;
extern u8 *D_00222D7C;
extern s32 D_00222D8C __attribute__((section(".data")));
extern s32 D_00222D90 __attribute__((section(".data")));
extern s32 D_00222D88;
extern s32 D_00222D94 __attribute__((section(".data")));

void func_L01_0058B268(ModelActor *actor) {
    s32 low, high, middle, first;
    ModelDescriptor *record;
    if (actor->f58 == NULL || actor->f24 == NULL || actor->f24[8] == 0) return;
    low = 0;
    high = D_00222D8C;
    first = (s32)(actor->f58 - D_00222D7C) >> 6;
    middle = high >> 1;
    for (;;) {
        record = &D_00222D78[middle];
        if (first < record->f4) high = middle - 1;
        else if (record->f4 < first) low = middle + 1;
        else break;
        middle = (high + low) >> 1;
    }
    record->f7 = 1;
    D_00222D90++;
    D_00222D88 += actor->f24[8];
    if (middle < D_00222D94) D_00222D94 = middle;
}
#endif
