/* cflags: -mno-split-addresses */
/* Real virtual actor reset, 006A6598..006A65E8 (80 bytes), NONMATCHING.
 * 00668070 invokes slot28 of vtable003B10C0 before callback installation.
 * This is an observed complete word clear, not an inferred context memset.
 */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_input_context_init.h"
#define ACTOR_VTABLE ((u32)0x003B10C0)
#define BASE_VTABLE ((u32)0x003B0718)
#define INIT_OFFSET(field) offsetof(DreadzoneActorInit31F0, fields.field)
#else
typedef union DreadzoneActorInit31F0 {
    u32 words[0x31F0 / 4];
    struct {
        u8 pad0[0x14];
        u32 f14;
        u8 pad18[0x2655];
        u8 f266D;
        u8 pad266E[0x97E];
        u8 f2FEC[0x1E0];
        u8 pad31CC[3];
        u8 f31CF;
        u8 pad31D0[0x20];
    } fields;
} DreadzoneActorInit31F0;
extern u8 D_003B10C0[], D_003B0718[];
#define ACTOR_VTABLE ((u32)D_003B10C0)
#define BASE_VTABLE ((u32)D_003B0718)
#define INIT_OFFSET(field) ((u32)&((DreadzoneActorInit31F0 *)0)->fields.field)
#endif

typedef char InitActorSize[sizeof(DreadzoneActorInit31F0) == 0x31F0 ? 1 : -1];
typedef char InitVtableOffset[INIT_OFFSET(f14) == 0x14 ? 1 : -1];
typedef char InitOptionOffset[INIT_OFFSET(f266D) == 0x266D ? 1 : -1];
typedef char InitContextOffset[INIT_OFFSET(f2FEC) == 0x2FEC ? 1 : -1];
typedef char InitStateOffset[INIT_OFFSET(f31CF) == 0x31CF ? 1 : -1];
#undef INIT_OFFSET

/* Shared real word-fill implementation is recovered in L01_006A9640.c for
 * native execution. Retail uses aligned 32-bit stores, including this size.
 */
void func_L01_0055D2A8(u32 *, u32, s32);

DreadzoneActorInit31F0 *func_L01_006A93A0(DreadzoneActorInit31F0 *);

void func_L01_006A6598(DreadzoneActorInit31F0 *record) {
    func_L01_0055D2A8(record->words, 0, 0x31F0);
    func_L01_006A93A0(record);
    record->fields.f31CF = 5;
    record->fields.f14 = ACTOR_VTABLE;
    record->fields.f266D = 1;
}
