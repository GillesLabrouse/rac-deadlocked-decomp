/* cflags: -mno-split-addresses */
#include "common.h"
/* Actual L01 selector: 006658C0..006658D8, 24 bytes. */
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_input_repeat.h"
DreadzoneActorInit31F0 *dreadzone_repeat_actor_at(const DreadzoneRepeatContext *host, u32 index) {
    if (!host || !host->actors || index >= host->actor_count) return NULL;
    return &host->actors[index];
}
#else
typedef struct RepeatActor31F0 { u8 bytes[0x31F0]; } RepeatActor31F0;
extern RepeatActor31F0 D_00368F60[];
RepeatActor31F0 *func_L01_006658C0(s32 index) {
    index *= 0x31F0;
    return (RepeatActor31F0 *)((u8 *)D_00368F60 + index);
}
#endif
