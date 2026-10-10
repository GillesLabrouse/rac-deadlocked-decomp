#include "common.h"

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_model_pool.h"
#include <stdlib.h>
#else
typedef struct ModelAnimationRecord {
    u16 f0, f2;
    u8 f4, f5, f6, f7;
} ModelAnimationRecord;
extern ModelAnimationRecord D_00293000[32] __attribute__((section(".data")));
extern s32 D_00222D98 __attribute__((section(".data")));
extern s32 D_00222D9C __attribute__((section(".data")));
#endif

void func_L01_0058B4D0(void) {
    s32 index;
#ifdef DEADLOCKED_NATIVE
    u8 *records = dreadzone_model_resolve(DREADZONE_MODEL_ANIMATION, 32 * 8);
    if (records == NULL) abort();
    dreadzone_model_write16(records + 2, 0x200);
    records[5] = 0xFF;
#else
    D_00293000[0].f2 = 0x200;
    D_00293000[0].f5 = 0xFF;
#endif
    D_00222D9C = 1;
    D_00222D98 = 0;
#ifdef DEADLOCKED_NATIVE
    records[4] = 0;
    dreadzone_model_write16(records, 0);
    for (index = 1; index < 31; index++) records[index * 8 + 5] = (u8)(index + 1);
    records[index * 8 + 5] = 0xFF;
#else
    D_00293000[0].f4 = 0;
    D_00293000[0].f0 = 0;
    for (index = 1; index < 31; index++) D_00293000[index].f5 = index + 1;
    D_00293000[index].f5 = 0xFF;
#endif
}
