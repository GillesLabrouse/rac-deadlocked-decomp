/* cflags: -mno-split-addresses */
#include "common.h"

/* L01 asynchronous resource loader. Not matching; no host backend is supplied.
 * Sector/offset meanings are provisional. See DREADZONE_OBJECT_SETUP.md. */
typedef struct LoaderBlock {
    u8 gap0[4];
    s32 f4;
    s32 f8;
    u8 gapC[4];
    u8 f10;
    u8 gap11;
    u8 f12;
    u8 gap13[6];
    u8 f19;
    u8 f1A;
    u8 f1B;
    u8 gap1C[4];
    u8 *f20;
    s32 f24;
} LoaderBlock;
typedef struct LoaderEntry { s32 f0; s32 f4; } LoaderEntry;
typedef void (*LoaderCallback)(LoaderBlock *, s32);
extern LoaderBlock D_001711A8;
extern LoaderEntry D_001CF050[];
extern s32 D_001CEC04 __attribute__((section(".data")));
extern s32 D_001CEBF4 __attribute__((section(".data")));
extern u8 *D_0024375C __attribute__((section(".data")));
void func_00157F98(void);
void func_001634A8(s32);
s32 func_00163928(void);
s32 func_00163840(s32, s32, s32, s32, s32, LoaderCallback, LoaderBlock *);
void func_L01_00583980(const u8 *, u8 *);
void func_L01_00653940(LoaderBlock *, s32);
void func_L01_006539B8(LoaderBlock *, s32);
typedef char loader_size_check[sizeof(LoaderBlock) == 0x28 ? 1 : -1];
typedef char loader_stage_check[(u32)&((LoaderBlock *)0)->f24 == 0x24 ? 1 : -1];
typedef char loader_flags_check[(u32)&((LoaderBlock *)0)->f19 == 0x19 ? 1 : -1];
typedef char loader_entry_check[sizeof(LoaderEntry) == 8 ? 1 : -1];

void func_L01_00653028(void) {
    LoaderEntry *entry;
    u8 *buffer;
    s32 offset;
    func_00157F98();
    func_001634A8(0);
    if (func_00163928() != 0) return;
    switch (D_001711A8.f24) {
    case 1:
        entry = &D_001CF050[D_001711A8.f4];
        if (entry->f0 != 0 && entry->f4 != 0) {
            buffer = D_001711A8.f20 + D_001CEC04 - (entry->f4 << 11);
            func_001634A8(1);
            entry = &D_001CF050[D_001711A8.f4];
            func_00163840(0, (s32)buffer, entry->f0 + D_001CEBF4,
                         entry->f4, 0, func_L01_00653940, &D_001711A8);
        } else {
            func_L01_00653940(&D_001711A8, 0);
        }
        break;
    case 2:
        func_00163840(1, D_001711A8.f4, 0, 0, 0,
                     func_L01_006539B8, &D_001711A8);
        D_001711A8.f24 = 4;
        break;
    case 4:
        entry = &D_001CF050[D_001711A8.f4];
        buffer = D_001711A8.f20 + D_001CEC04 - (entry->f4 << 11);
        offset = *(s32 *)buffer;
        if (offset != 0) {
            func_L01_00583980(buffer + offset - (entry->f0 << 11), D_0024375C);
            D_001711A8.f19 = 1;
        } else {
            D_001711A8.f19 = 0;
        }
        D_001711A8.f24 = 5;
        break;
    case 5:
        entry = &D_001CF050[D_001711A8.f4];
        buffer = D_001711A8.f20 + D_001CEC04 - (entry->f4 << 11);
        offset = *(s32 *)(buffer + 8);
        if (offset != 0) {
            func_L01_00583980(buffer + offset - (entry->f0 << 11), D_001711A8.f20);
            D_001711A8.f1A = 1;
        } else {
            D_001711A8.f1A = 0;
        }
        D_001711A8.f24 = 6;
        break;
    case 6:
        if (D_001711A8.f1B != 0) D_001711A8.f24 = 7;
        break;
    case 7:
        D_001711A8.f10 = D_001711A8.f8 == 0 ? 1 : 0;
        D_001711A8.f24 = 0;
        D_001711A8.f12 = 0;
        break;
    }
}
