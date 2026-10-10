#ifndef DEADLOCKED_DREADZONE_MEMORY_H
#define DEADLOCKED_DREADZONE_MEMORY_H

#include "common.h"

enum {
    DREADZONE_SCRATCH_COUNT = 64,
    DREADZONE_POOL_COUNT = 256,
    DREADZONE_POOL_GUEST_BASE = 0x00375720,
    DREADZONE_POOL_RECORD_SIZE = 0x5C
};

typedef struct DreadzoneWorkBlock {
    u32 f0;
    u8 gap4[0x2C];
    u32 f30;
    u32 f34;
} DreadzoneWorkBlock;

typedef struct DreadzoneScratchRecord {
    u32 f0;
    u32 f4;
    u32 f8;
    u32 fC;
} DreadzoneScratchRecord;

/* f58 remains a guest address, including on a 64-bit host. */
typedef struct DreadzonePoolRecord {
    u32 f0;
    u32 f4;
    u32 f8;
    u32 fC[16];
    u32 f4C;
    s32 f50;
    u8 f54;
    u8 f55;
    u8 gap56[2];
    u32 f58;
} DreadzonePoolRecord;

extern DreadzoneWorkBlock D_001DFA70;
extern DreadzoneScratchRecord D_0034D320[DREADZONE_SCRATCH_COUNT];
extern DreadzonePoolRecord D_00375720[DREADZONE_POOL_COUNT];
extern u32 D_00220828;

void func_L01_00628020(void);
void func_L01_006A9640(void);
void func_L01_006A9AE0(DreadzonePoolRecord *record);
void func_L01_0055D2A8(u32 *destination, u32 value, s32 bytes);

#endif
