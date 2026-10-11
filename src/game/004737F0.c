/* cflags: -G0 */
/* Complete resident4737F0; actual L01 sibling591940 changes only the callee. */
#include "common.h"
typedef struct ReclassParam {
    u8 pad0[0x10];
    u8 threshold;
} ReclassParam;
typedef struct ReclassArray {
    u8 pad0[0x48];
    ReclassParam *params[0]; /* GNU variable-length trailing table. */
} ReclassArray;
typedef struct ReclassActor {
    u8 pad0[0x24];
    ReclassArray *array;
    u8 pad28[0x2D];
    u8 state55;
} ReclassActor;

typedef char actor_array_offset_check[(u32)&((ReclassActor *)0)->array == 0x24 ? 1 : -1];
typedef char actor_flag_offset_check[(u32)&((ReclassActor *)0)->state55 == 0x55 ? 1 : -1];
typedef char class_table_offset_check[(u32)&((ReclassArray *)0)->params == 0x48 ? 1 : -1];
typedef char parameter_byte_offset_check[(u32)&((ReclassParam *)0)->threshold == 0x10 ? 1 : -1];

void func_0046F2B0(ReclassActor *, ReclassParam *, s32, s32, f32);

void func_004737F0(ReclassActor *actor, s32 index, f32 threshold) {
    ReclassParam *param = actor->array->params[index];
    if ((f32)param->threshold <= threshold)
        threshold = (f32)((s32)param->threshold - 1);
    func_0046F2B0(actor, param, index, 0, threshold);
    actor->state55 &= (u8)~2;
}
