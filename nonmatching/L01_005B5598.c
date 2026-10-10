/* cflags: -mno-split-addresses */
#include "common.h"

/* Actual L01 005B5598..005B5668, 208 bytes. ABI takes class ID and alternate
 * selector. A 12-byte record has class ID, callable guest address and metadata
 * pointer. Sentinel class -1 supplies the genuine fallback record. */
#ifndef DEADLOCKED_NATIVE
typedef struct ClassDispatchRecord { s32 object_class; u32 callback,metadata; } ClassDispatchRecord;
extern ClassDispatchRecord D_003C5E00[];
extern u8 D_003C6CAC[],D_0026B1C0[],D_0026DF90[];
extern u32 D_00222D18,D_00222D1C,D_0026AE00[],D_0026DD90[];
void func_L01_005B5598(s32 objectClass,s32 alternate) {
    s32 i=0;
    u32 slot;
    ClassDispatchRecord *record;
    while (D_003C5E00[i].object_class!=-1 && D_003C5E00[i].object_class!=objectClass) i++;
    record=&D_003C5E00[i];
    if (alternate!=0) {
        slot=D_00222D1C;
        D_0026DD90[slot]=record->callback;
        D_0026DF90[slot]=(u8)(((s32)(record->metadata-(u32)D_003C6CAC))>>5);
    } else {
        slot=D_00222D18;
        D_0026AE00[slot]=record->callback;
        D_0026B1C0[slot]=(u8)(((s32)(record->metadata-(u32)D_003C6CAC))>>5);
    }
}
#endif
