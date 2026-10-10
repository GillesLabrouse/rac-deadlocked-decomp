#ifndef DEADLOCKED_DREADZONE_MODEL_POOL_H
#define DEADLOCKED_DREADZONE_MODEL_POOL_H

#include "common.h"

enum {
    DREADZONE_MODEL_RECORDS = 0x3094,
    DREADZONE_MODEL_RECORD_BYTES = 0x40,
    DREADZONE_MODEL_DESCRIPTORS = 0x400,
    DREADZONE_MODEL_MATRIX_BYTES = 0xC2500,
    DREADZONE_MODEL_DESCRIPTOR_BYTES = 0x2000,
    DREADZONE_MODEL_CONFIG = 0x00243700,
    DREADZONE_MODEL_ANIMATION = 0x00293000,
    DREADZONE_MODEL_OK = 0,
    DREADZONE_MODEL_UNMAPPED = -1,
    DREADZONE_MODEL_CORRUPT = -2,
    DREADZONE_MODEL_CAPACITY = -3
};

/* Retail descriptors remain eight bytes; f0 and actor+0x58 are guest addresses. */
typedef struct DreadzoneModelDescriptor {
    u32 actor;
    u16 first_record;
    u8 count;
    u8 released;
} DreadzoneModelDescriptor;

typedef struct DreadzoneModelView {
    u32 guest_base;
    u32 bytes;
    u8 *storage;
} DreadzoneModelView;

/* Bind at most 16 disjoint guest regions. The caller owns the writable backing.
 * Native operations validate the retail domain before writing. Invalid retail
 * states return an error; they are not made into successful game operations.
 * Allocation requires the nonzero definition[8] used by the actual actor caller.
 * Reset changes counters only. Allocation does not initialize matrix contents.
 */
int dreadzone_model_pool_bind(const DreadzoneModelView *views, u32 count);
int dreadzone_model_pool_init(void);
int dreadzone_model_pool_validate(void);
int dreadzone_model_pool_allocate(u32 actor, u32 *matrices);
int dreadzone_model_pool_release(u32 actor);
int dreadzone_model_pool_compact(void);

/* These addresses hold the observed retail state, in address order. */
extern u32 D_00222D78, D_00222D7C, D_00222D80;
extern s32 D_00222D84, D_00222D88, D_00222D8C, D_00222D90, D_00222D94;
extern s32 D_00222D98, D_00222D9C;

void func_L01_0058B050(void);
void func_L01_0058B098(void);
void func_L01_0058B4D0(void);

/* Internal shared translation and little-endian field helpers. No guest pointer
 * is dereferenced on the host. These do not extend any retail record layout.
 */
u8 *dreadzone_model_resolve(u32 address, u32 bytes);
u32 dreadzone_model_read32(const u8 *bytes);
void dreadzone_model_write32(u8 *bytes, u32 value);
u16 dreadzone_model_read16(const u8 *bytes);
void dreadzone_model_write16(u8 *bytes, u16 value);

#endif
