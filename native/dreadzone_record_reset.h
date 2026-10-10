#ifndef DEADLOCKED_DREADZONE_RECORD_RESET_H
#define DEADLOCKED_DREADZONE_RECORD_RESET_H

#include "dreadzone_memory.h"

typedef struct DreadzoneQuad {
    u32 words[4];
} DreadzoneQuad;

typedef struct DreadzoneResetRecord {
    u32 f0;
    u8 gap4[0x24C];
    DreadzoneQuad f250;
    DreadzoneQuad f260;
    u8 gap270[0x40];
    u32 f2B0, f2B4, f2B8, f2BC;
    u8 gap2C0[0x18];
    u32 f2D8;
    s16 f2DC;
    u8 gap2DE[6];
    s16 f2E4;
    u16 f2E6;
    f32 f2E8, f2EC;
    u32 f2F0;
    u8 gap2F4[4];
    u32 f2F8;
    u8 gap2FC[4];
    u32 f300, f304;
    u8 gap308[8];
    u32 f310, f314;
    u8 gap318[0x10];
    u32 f328, f32C;
    u8 gap330[0x10];
} DreadzoneResetRecord;

/* The fill routine writes actual u32 storage. The alternate record member
 * expresses the retail layout without widening any guest fields on Windows.
 */
typedef union DreadzoneResetStorage {
    u32 words[0x4100 / 4];
    DreadzoneResetRecord records[20];
} DreadzoneResetStorage;

extern DreadzoneResetStorage D_0037B320;
extern u32 D_00220844;
extern u32 D_0037F428[7];

#define RESET_RECORDS D_0037B320.records
#define RESET_WORDS D_0037B320.words
#define ZERO_QUAD ((DreadzoneQuad){ { 0, 0, 0, 0 } })

void func_L01_006AF588(void);

#endif
