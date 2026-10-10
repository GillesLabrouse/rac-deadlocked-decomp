/* cflags: -mno-split-addresses */
/* asflags: --float-immediates */
#include "common.h"

/* Experimental L01 record initializer. Depends on the real virtual actor
 * constructor and child-link backend; it does not provide substitute effects.
 * Pointer layouts below are PS2 layouts, not 64-bit host structures. */
typedef struct Record31F0 Record31F0;
typedef struct RecordVector { f32 x, y, z, w; } RecordVector;
typedef struct RecordVtable {
    u8 gap0[0x2C];
    void (*f2C)(Record31F0 *, void *, RecordVector *, RecordVector *, s32);
} RecordVtable;
typedef struct RecordOwner { u8 gap0[0xE0]; Record31F0 *fE0; } RecordOwner;
typedef struct RecordActor {
    u8 gap0[0x36]; u16 f36;
    u8 gap38[0x3C]; s16 f74;
    u8 gap76[0x36]; RecordOwner *fAC;
} RecordActor;
typedef struct RecordPayload { u8 gap0[0x38]; s32 f38; } RecordPayload;
typedef struct RecordChild { s32 f0; RecordPayload *f4; } RecordChild;
typedef struct RecordDefinition {
    u8 gap0[0x551]; u8 f551;
    u8 gap552[0x1A]; void (*f56C)(void);
    u8 *f570;
} RecordDefinition;
struct Record31F0 {
    s32 f0;
    u8 gap4[0x10];
    RecordVtable * f14;
    u8 gap18[0x88];
    RecordVector fA0;
    RecordVector fB0;
    u8 gapC0[0x1A20];
    s32 f1AE0;
    u8 gap1AE4[0x834];
    s32 f2318;
    u8 gap231C[0xB04];
    f32 f2E20;
    u8 gap2E24[0xC0];
    u8 f2EE4;
    u8 gap2EE5[0x7];
    u8 * f2EEC;
    RecordActor * f2EF0;
    u8 gap2EF4[0x8];
    u8 * f2EFC;
    RecordDefinition * f2F00;
    f32 f2F04;
    f32 f2F08;
    f32 f2F0C;
    s32 f2F10;
    s32 f2F14;
    u8 gap2F18[0xC];
    f32 f2F24;
    RecordChild * f2F28;
    u8 gap2F2C[0xBE];
    u8 f2FEA;
    u8 gap2FEB[0x1];
    u8 f2FEC;
    u8 gap2FED[0x203];
};

extern Record31F0 D_00368F60[];
extern RecordVtable D_003B10C0;
extern u8 D_001EE600[], D_00230280[], D_003BD930[];
extern u8 D_00171D50[], D_00171D80[], D_00171DB0[];
extern u8 D_00171E00[], D_00171E78[], D_00171EF0[];
extern u8 D_00171DE0[];
extern s32 D_001711D0[], D_00220810[];
extern Record31F0 *D_003660F8[];
extern s16 D_00171B50 __attribute__((section(".data")));
extern s32 D_0021E824 __attribute__((section(".data")));
extern s32 D_0021E828 __attribute__((section(".data")));
s32 func_0015C6E8(s32);
/* Slot six is unused by the callee; retain it to place the output in slot seven.
 * The final float is passed independently in f12 under the original ABI. */
s32 func_0015FA68(s32, RecordVector *, RecordVector *, s32, s32, s32,
                  s32, RecordChild **, f32);
void func_L01_006A93A0(Record31F0 *);
void func_L01_00669078(Record31F0 *);
void func_L01_0068E3B0(Record31F0 *, u8);
void func_L01_00688578(Record31F0 *, s32);
/* The stored callback is not invoked here; its input signature is unresolved. */
void func_L01_006B5CB8(void);
typedef char record_f0_offset_check[(u32)&((Record31F0 *)0)->f0 == 0x0 ? 1 : -1];
typedef char record_f14_offset_check[(u32)&((Record31F0 *)0)->f14 == 0x14 ? 1 : -1];
typedef char record_fA0_offset_check[(u32)&((Record31F0 *)0)->fA0 == 0xA0 ? 1 : -1];
typedef char record_fB0_offset_check[(u32)&((Record31F0 *)0)->fB0 == 0xB0 ? 1 : -1];
typedef char record_f1AE0_offset_check[(u32)&((Record31F0 *)0)->f1AE0 == 0x1AE0 ? 1 : -1];
typedef char record_f2318_offset_check[(u32)&((Record31F0 *)0)->f2318 == 0x2318 ? 1 : -1];
typedef char record_f2E20_offset_check[(u32)&((Record31F0 *)0)->f2E20 == 0x2E20 ? 1 : -1];
typedef char record_f2EE4_offset_check[(u32)&((Record31F0 *)0)->f2EE4 == 0x2EE4 ? 1 : -1];
typedef char record_f2EEC_offset_check[(u32)&((Record31F0 *)0)->f2EEC == 0x2EEC ? 1 : -1];
typedef char record_f2EF0_offset_check[(u32)&((Record31F0 *)0)->f2EF0 == 0x2EF0 ? 1 : -1];
typedef char record_f2EFC_offset_check[(u32)&((Record31F0 *)0)->f2EFC == 0x2EFC ? 1 : -1];
typedef char record_f2F00_offset_check[(u32)&((Record31F0 *)0)->f2F00 == 0x2F00 ? 1 : -1];
typedef char record_f2F04_offset_check[(u32)&((Record31F0 *)0)->f2F04 == 0x2F04 ? 1 : -1];
typedef char record_f2F08_offset_check[(u32)&((Record31F0 *)0)->f2F08 == 0x2F08 ? 1 : -1];
typedef char record_f2F0C_offset_check[(u32)&((Record31F0 *)0)->f2F0C == 0x2F0C ? 1 : -1];
typedef char record_f2F10_offset_check[(u32)&((Record31F0 *)0)->f2F10 == 0x2F10 ? 1 : -1];
typedef char record_f2F14_offset_check[(u32)&((Record31F0 *)0)->f2F14 == 0x2F14 ? 1 : -1];
typedef char record_f2F24_offset_check[(u32)&((Record31F0 *)0)->f2F24 == 0x2F24 ? 1 : -1];
typedef char record_f2F28_offset_check[(u32)&((Record31F0 *)0)->f2F28 == 0x2F28 ? 1 : -1];
typedef char record_f2FEA_offset_check[(u32)&((Record31F0 *)0)->f2FEA == 0x2FEA ? 1 : -1];
typedef char record_f2FEC_offset_check[(u32)&((Record31F0 *)0)->f2FEC == 0x2FEC ? 1 : -1];

typedef char record_size_check[sizeof(Record31F0) == 0x31F0 ? 1 : -1];
typedef char record_actor_owner_check[(u32)&((RecordActor *)0)->fAC == 0xAC ? 1 : -1];
typedef char record_definition_callback_check[(u32)&((RecordDefinition *)0)->f56C == 0x56C ? 1 : -1];
typedef char record_vtable_constructor_check[(u32)&((RecordVtable *)0)->f2C == 0x2C ? 1 : -1];

void func_L01_006A8420(s32 index) {
    Record31F0 *record = &D_00368F60[index];
    RecordDefinition *definition;
    RecordActor *actor;
    s32 i, sourceOffset, destinationOffset, column, selection;
    f32 health;
    func_L01_006A93A0(record);
    record->f2FEA = (u8)index;
    record->f14 = &D_003B10C0;
    D_003B10C0.f2C(record, NULL, &record->fA0, &record->fB0, index);
    definition = (RecordDefinition *)(D_001EE600 + index * 4 * 0x5C0);
    record->f2EFC = D_00230280 + index * 0x470;
    record->f2FEA = (u8)index;
    record->f2F10 = index;
    record->f2F00 = definition;
    if (D_0021E824 == 0 && D_0021E828 == 1 && index == D_0021E828 && definition->f551 != 0)
        record->f2F00 = (RecordDefinition *)(D_001EE600 + 0x5C0);
    record->f2EEC = D_003BD930;
    record->f2F00->f56C = func_L01_006B5CB8;
    record->f2F00->f570 = &record->f2FEC;
    /* Retail uses lq/pextuw/prot3w then mtc1: these are bit copies, not
     * conversions of integer components to floating-point values. */
    record->f2F04 = record->fA0.x;
    record->f2F08 = record->fA0.y;
    record->f2F0C = record->fA0.z;
    func_0015FA68(record->f2F10, &record->fA0, &record->fB0, record->f2EE4,
                  record->f2F14, record->f2318, index * 4, &record->f2F28, record->f2E20);
    record->f2F28->f4->f38 = index;
    record->f0 = index;
    destinationOffset = 0;
    for (i = 0; i < 3; i++) {
        sourceOffset = i * 0x10;
        column = record->f2FEA * 4;
        *(s32 *)(D_00171E00 + destinationOffset + column) = *(s32 *)(D_00171D50 + sourceOffset + column);
        column = record->f2FEA * 4;
        *(s32 *)(D_00171E78 + destinationOffset + column) = *(s32 *)(D_00171D80 + sourceOffset + column);
        column = record->f2FEA * 4;
        *(s32 *)(D_00171EF0 + destinationOffset + column) = *(s32 *)(D_00171DB0 + sourceOffset + column);
        destinationOffset += 0x28;
    }
    if (D_00171DE0[record->f2FEA] != D_001711D0[record->f2FEA]) {
        D_00171DE0[record->f2FEA] = (u8)D_001711D0[record->f2FEA];
        func_L01_0068E3B0(record, D_00171DE0[record->f2FEA]);
        record->f1AE0 = 0;
    }
    record->f0 = func_0015C6E8(0);
    actor = record->f2EF0;
    health = (f32)(D_00171B50 == 0 ? 50 : D_00171B50);
    record->f2F24 = health;
    record->f2E20 = health;
    actor->fAC->fE0 = record;
    func_L01_00669078(record);
    if (D_0021E824 == 0 && D_0021E828 == 1 && index == D_0021E828) {
        record->f2EF0->f74 = (s16)index;
        record->f2F14 = 0;
        actor = record->f2EF0;
        actor->f36 |= 0x8000;
        actor = record->f2EF0;
        actor->f36 &= 0x87FF;
        func_L01_00688578(record, 3);
    }
    selection = D_00220810[index];
    if (selection != 0) func_L01_00688578(record, selection);
    D_003660F8[index] = record;
}
