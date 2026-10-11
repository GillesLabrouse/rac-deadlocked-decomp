#ifndef DEADLOCKED_DREADZONE_ACTOR_CALLBACK_ENQUEUE_H
#define DEADLOCKED_DREADZONE_ACTOR_CALLBACK_ENQUEUE_H
#include "common.h"
typedef struct DreadzoneCallbackEnqueueRequest {
    u8 *storage;
    u32 guest_base,storage_bytes,record_guest;
} DreadzoneCallbackEnqueueRequest;
typedef struct DreadzoneCallbackEnqueueInfo {
    u32 skipped,queue_calls,capacity_exits,queued,entry_guest,argument_guest;
} DreadzoneCallbackEnqueueInfo;
enum {DREADZONE_CALLBACK_ENQUEUE_OK=1,DREADZONE_CALLBACK_ENQUEUE_INVALID=-1,DREADZONE_CALLBACK_ENQUEUE_MEMORY=-4};
/* Complete selected L01 66ABD8 and reached54A4E8 integer RAM effects.
 * Callback66ACA0 is enqueued, never executed or replaced with a return.
 * Real actor2EF0, queue222AF4/241140 and head222A9C are supplied guest inputs.
 * Signed count>=256 still sets the actual actor25C8 byte before capacity exit;
 * negative counts retain guest32 shift/add semantics when their accesses map.
 * Every linked-list load follows actual store order, including aliases.
 * One exclusive borrowed RAM range below2GiB, <=16MiB; aligned guest words and
 * record31F0/16. Request is captured and may alias RAM; info is disjoint from
 * both. Refusal preserves RAM/info. No constructor/queue execution/full frame,
 * outgoing low-GPR return, physical cache/timing or byte-exact credit claimed.
 */
int dreadzone_actor_callback_enqueue_0066ABD8(const DreadzoneCallbackEnqueueRequest *,DreadzoneCallbackEnqueueInfo *);
#endif
