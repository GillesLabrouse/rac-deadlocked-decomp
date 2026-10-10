#ifndef DEADLOCKED_DREADZONE_INPUT_REPEAT_H
#define DEADLOCKED_DREADZONE_INPUT_REPEAT_H
#include "dreadzone_input_context_init.h"

typedef struct DreadzoneRepeatInputView {
    u32 guest_address;
    InputRecord5C0 *storage;
} DreadzoneRepeatInputView;

/* 00614EE8 updates only button repetition. It does not update actors.
 * The four-element globals are separated by 16 bytes in retail storage.
 * Actor input links at 2F00 remain guest32 values; views are explicit bindings.
 */
typedef struct DreadzoneRepeatContext {
    s32 player_count;
    DreadzoneActorInit31F0 *actors;
    u32 actor_count;
    const DreadzoneRepeatInputView *inputs;
    u32 input_count;
    u32 mask_0021F580;
    s32 countdown_0021F588[4];
    u32 previous_0021F598[4];
} DreadzoneRepeatContext;

enum DreadzoneRepeatResult {
    DREADZONE_REPEAT_COMPLETE = 0,
    DREADZONE_REPEAT_INVALID = 1,
    DREADZONE_REPEAT_NEEDS_INPUT = 2
};
DreadzoneActorInit31F0 *dreadzone_repeat_actor_at(const DreadzoneRepeatContext *, u32);
int dreadzone_input_repeat(DreadzoneRepeatContext *);
#endif
