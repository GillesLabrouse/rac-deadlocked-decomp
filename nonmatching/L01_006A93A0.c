/* cflags: -mno-split-addresses */
/* Real base constructor, 006A93A0..006A9630 (656 bytes), NONMATCHING.
 * Retail also performs empty fixed-count loops with only nop bodies. This
 * recovery preserves memory/return effects, not those instruction timings.
 */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_input_context_init.h"
#define BASE_VTABLE ((u32)0x003B0718)
#define BASE_OFFSET(field) offsetof(DreadzoneActorInit31F0, fields.field)
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
extern u8 D_003B0718[];
#define BASE_VTABLE ((u32)D_003B0718)
#define BASE_OFFSET(field) ((u32)&((DreadzoneActorInit31F0 *)0)->fields.field)
#endif
typedef char BaseActorSize[sizeof(DreadzoneActorInit31F0) == 0x31F0 ? 1 : -1];
typedef char BaseVtableOffset[BASE_OFFSET(f14) == 0x14 ? 1 : -1];
#undef BASE_OFFSET
void *func_L01_006A9638(void *);

DreadzoneActorInit31F0 *func_L01_006A93A0(DreadzoneActorInit31F0 *record) {
    record->fields.f14 = BASE_VTABLE;
    func_L01_006A9638((u8 *)record + 0x1970);
    return record;
}
