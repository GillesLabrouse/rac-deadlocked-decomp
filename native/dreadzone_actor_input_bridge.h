#ifndef DEADLOCKED_DREADZONE_ACTOR_INPUT_BRIDGE_H
#define DEADLOCKED_DREADZONE_ACTOR_INPUT_BRIDGE_H
#include "dreadzone_actor_snapshot.h"
#include "dreadzone_input.h"

typedef struct DreadzoneActorInputMemory {
    u8 *storage;
    u32 guest_base,bytes;
    int writable;
} DreadzoneActorInputMemory;
typedef struct DreadzoneActorInputContext {
    u8 *record;
    u32 record_guest,record_bytes;
    const DreadzoneActorInputMemory *views;
    u32 view_count;
} DreadzoneActorInputContext;
enum DreadzoneActorInputResult {
    DREADZONE_ACTOR_INPUT_COMPLETED=0,
    DREADZONE_ACTOR_INPUT_APPROXIMATE_VU=1,
    DREADZONE_ACTOR_INPUT_INVALID_STORAGE=-1,
    DREADZONE_ACTOR_INPUT_NEEDS_MEMORY=-2,
    DREADZONE_ACTOR_INPUT_UNSUPPORTED_INDEX=-3,
    DREADZONE_ACTOR_INPUT_UNSUPPORTED_FLOAT=-4,
    DREADZONE_ACTOR_INPUT_NO_TRANSACTION_MEMORY=-5,
    DREADZONE_ACTOR_INPUT_NEEDS_PREDICATE_MAPPING=-6
};
/* Complete controller-to-actor bridge 006A7D48 and button preparation0067BD58,
 * using the real 0068DE60 predicate, timer0055D180, remapper005BE808,
 * magnitude0067C570 and recovered0055D708 angle polynomial.
 *
 * Record is a writable31F0 RAM image with guest16 alignment. At most64 views
 * supply actual disjoint nonwrapping guest/host RAM and permissions. Context
 * and descriptors are disjoint from all backing. Other nonwrapping pointers
 * must designate real allocations; inputs remain stable with exclusive access.
 * Transactions preserve guest aliases and ordered reads, publishing writable
 * images only after the complete selected path succeeds (total<=16MiB).
 * Publication does not synchronize concurrent observers.
 *
 * Actor2F00 supplies the decoder record. Input108 is copied to actor120, then
 * input10C is read and its sign bit inverted into actor124. Actor128 remains
 * an actual third magnitude component. The helper's xyz square/add/sqrt/min
 * and angle polynomial are native approximations, never full VU equivalence.
 * Their supported components are signed zero or magnitude[2^-30,1]. Invalid
 * values/results refuse atomically. The mode/predicate index2F10 is0..1.
 *
 * Bindings:00171DE0 mode/config bytes,0021DDB4 predicate state,0021E824 mode,
 * and one-shot0021E8F4. A nonzero one-shot is genuinely cleared after button
 * preparation, skipping all axis/magnitude/angle work with exact CPU status.
 * 0067BD58 sets timer05F4 from input1C0, or executes its actual decrement;
 * mode1 can force it to1 from word05F0. Its standalone entry is exact CPU.
 * This bridge supplies direction state only: it does not integrate position,
 * choose animation, run collision, renderer, transport or a playable frame.
 */
int dreadzone_actor_input_006A7D48(const DreadzoneActorInputContext *);
int dreadzone_actor_input_0067BD58(const DreadzoneActorInputContext *);
s32 func_L01_0055D180(s16 *timer);

/* Internal implementations require the bounded transaction context. */
int func_L01_006A7D48(const DreadzoneActorInputContext *,u8 *record);
int func_L01_0067BD58(const DreadzoneActorInputContext *,u8 *record);
int dreadzone_actor_input_005BE808(const DreadzoneActorInputContext *,s32 mask,s32 index,s32 *result);
f32 func_L01_0067C570(const u8 *unused,const f32 *vector);
const u8 *dreadzone_actor_input_read(const DreadzoneActorInputContext *,u32,u32);
u8 *dreadzone_actor_input_write(const DreadzoneActorInputContext *,u32,u32);
u32 dreadzone_actor_input_word(const u8 *);
void dreadzone_actor_input_store32(u8 *,u32);
int dreadzone_actor_input_predicate(const DreadzoneActorInputContext *,u8 *,s32 *);
#endif
