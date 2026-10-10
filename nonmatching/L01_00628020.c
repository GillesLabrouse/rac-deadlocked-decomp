/* Dreadzone pre-loop memory initialization: all 64 retail bytes match after
 * linking actual L01 symbols. Kept here because the accepted canonical copy
 * is L00 0066E0B8, which has an independently verified different scratch base.
 */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_memory.h"
#else
typedef struct DreadzoneWorkBlock {
    u32 f0;
    u8 gap4[0x2C];
    u32 f30;
    u32 f34;
} DreadzoneWorkBlock;
typedef struct DreadzoneScratchRecord {
    u32 f0, f4, f8, fC;
} DreadzoneScratchRecord;
extern DreadzoneWorkBlock D_001DFA70;
extern DreadzoneScratchRecord D_0034D320[64];
#endif

#ifdef DEADLOCKED_NATIVE
#define MEMORY_FIELD(type, field) offsetof(type, field)
#else
#define MEMORY_FIELD(type, field) ((u32)&((type *)0)->field)
#endif
typedef char check_work30[MEMORY_FIELD(DreadzoneWorkBlock, f30) == 0x30 ? 1 : -1];
typedef char check_work34[MEMORY_FIELD(DreadzoneWorkBlock, f34) == 0x34 ? 1 : -1];
typedef char check_scratch10[sizeof(DreadzoneScratchRecord) == 0x10 ? 1 : -1];
#undef MEMORY_FIELD

void func_L01_00628020(void) {
    s32 remaining;
    DreadzoneScratchRecord *record;
    u32 saved = D_001DFA70.f0;
    D_001DFA70.f34 = 0;
    D_001DFA70.f30 = saved;
    record = D_0034D320;
    remaining = 63;
    do {
        record->f0 = 0;
        record->f4 = 0;
        remaining--;
        record++;
    } while (remaining >= 0);
}
