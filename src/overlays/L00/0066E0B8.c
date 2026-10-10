#include "common.h"

/* Canonical L00 copy. L01's equivalent uses scratch storage at 0034D320;
 * this copy's actual scratch base is 0032B0E0, independently checked in L00.
 */
typedef struct WorkBlock {
    u32 f0;
    u8 gap4[0x2C];
    u32 f30;
    u32 f34;
} WorkBlock;

typedef struct ScratchRecord {
    u32 f0, f4, f8, fC;
} ScratchRecord;

extern WorkBlock D_001DFA70;
extern ScratchRecord D_0032B0E0[64];

typedef char work30_check[(u32)&((WorkBlock *)0)->f30 == 0x30 ? 1 : -1];
typedef char work34_check[(u32)&((WorkBlock *)0)->f34 == 0x34 ? 1 : -1];
typedef char scratch_stride_check[sizeof(ScratchRecord) == 0x10 ? 1 : -1];

void func_L00_0066E0B8(void) {
    s32 remaining;
    ScratchRecord *record;
    u32 saved = D_001DFA70.f0;
    D_001DFA70.f34 = 0;
    D_001DFA70.f30 = saved;
    record = D_0032B0E0;
    remaining = 63;
    do {
        record->f0 = 0;
        record->f4 = 0;
        remaining--;
        record++;
    } while (remaining >= 0);
}
