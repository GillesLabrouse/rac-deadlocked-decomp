#include "common.h"

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_model_pool.h"
#include <string.h>

int dreadzone_model_pool_compact(void) {
    s32 destination_index, source_index;
    u32 next_record;
    u8 *descriptors;
    int status = dreadzone_model_pool_validate();
    if (status != 0) return status;
    if (D_00222D90 < 0x400 - D_00222D8C && D_00222D88 < D_00222D84) return DREADZONE_MODEL_OK;
    if (D_00222D90 == 0) return DREADZONE_MODEL_CORRUPT;
    descriptors = dreadzone_model_resolve(D_00222D78, DREADZONE_MODEL_DESCRIPTOR_BYTES);
    destination_index = D_00222D94;
    next_record = dreadzone_model_read16(descriptors + destination_index * 8 + 4);
    for (source_index = destination_index + 1; source_index < D_00222D8C; source_index++) {
        u8 *record = descriptors + source_index * 8;
        if (record[7] == 0) {
            u8 *actor = dreadzone_model_resolve(dreadzone_model_read32(record), 0x5C);
            u32 source_address = dreadzone_model_read32(actor + 0x58);
            u32 count = record[6], index;
            dreadzone_model_write32(actor + 0x58, D_00222D7C + next_record * 64);
            dreadzone_model_write16(record + 4, (u16)next_record);
            for (index = 0; index < count; index++, next_record++) {
                u8 *source = dreadzone_model_resolve(source_address + index * 64, 64);
                u8 *destination = dreadzone_model_resolve(D_00222D7C + next_record * 64, 64);
                u8 quad[16];
                u32 offset;
                /* Retail loads each quad before storing it, in forward order. */
                for (offset = 0; offset < 64; offset += 16) {
                    memcpy(quad, source + offset, 16);
                    memcpy(destination + offset, quad, 16);
                }
            }
            memmove(descriptors + destination_index * 8, record, 8);
            destination_index++;
        }
    }
    D_00222D80 = D_00222D7C + next_record * 64;
    D_00222D84 = 0x3094 - (s32)next_record;
    D_00222D8C = destination_index;
    D_00222D94 = 0x7FFFFFFF;
    D_00222D90 = 0;
    D_00222D88 = 0;
    return DREADZONE_MODEL_OK;
}
#else
typedef unsigned int ModelQuad __attribute__((mode(TI)));
typedef struct ModelMatrix { ModelQuad f0, f10, f20, f30; } ModelMatrix;
typedef struct ModelActor {
    u8 f0[0x24];
    u8 *f24;
    u8 f28[0x30];
    ModelMatrix *f58;
} ModelActor;
typedef struct ModelDescriptor {
    ModelActor *f0;
    u16 f4;
    u8 f6, f7;
} ModelDescriptor;
extern s32 D_00222D8C __attribute__((section(".data")));
extern s32 D_00222D90 __attribute__((section(".data")));
extern s32 D_00222D88;
extern s32 D_00222D84 __attribute__((section(".data")));
extern s32 D_00222D94 __attribute__((section(".data")));
extern ModelDescriptor *D_00222D78;
extern ModelMatrix *D_00222D7C __attribute__((section(".data")));
extern ModelMatrix *D_00222D80 __attribute__((section(".data")));

void func_L01_0058B338(void) {
    s32 destination_index, source_index, next_record;
    if (D_00222D90 < 0x400 - D_00222D8C && D_00222D88 < D_00222D84) return;
    destination_index = D_00222D94;
    next_record = D_00222D78[destination_index].f4;
    for (source_index = destination_index + 1; source_index < D_00222D8C; source_index++) {
        if (D_00222D78[source_index].f7 == 0) {
            ModelActor *actor = D_00222D78[source_index].f0;
            ModelMatrix *source = actor->f58;
            s32 count = D_00222D78[source_index].f6;
            actor->f58 = D_00222D7C + next_record;
            D_00222D78[source_index].f4 = next_record;
            while (count != 0) {
                ModelMatrix *destination = D_00222D7C + next_record;
                destination->f0 = source->f0;
                next_record++;
                count--;
                destination->f10 = source->f10;
                destination->f20 = source->f20;
                destination->f30 = source->f30;
                source++;
            }
            D_00222D78[destination_index] = D_00222D78[source_index];
            destination_index++;
        }
    }
    D_00222D80 = D_00222D7C + next_record;
    D_00222D84 = 0x3094 - next_record;
    D_00222D8C = destination_index;
    D_00222D94 = 0x7FFFFFFF;
    D_00222D90 = 0;
    D_00222D88 = 0;
}
#endif
