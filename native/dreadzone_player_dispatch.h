#ifndef DEADLOCKED_DREADZONE_PLAYER_DISPATCH_H
#define DEADLOCKED_DREADZONE_PLAYER_DISPATCH_H
#include "dreadzone_actor_idle.h"
typedef DreadzoneActorIdleRequest DreadzonePlayerDispatchRequest;
typedef struct DreadzonePlayerDispatchInfo {
    DreadzoneActorIdleInfo idle;
    u32 idle_calls,cleared_378,cleared_37c;
} DreadzonePlayerDispatchInfo;
/* Selected complete L01 690F18 substate0 route through real695570 and common
 * tail. Requires actual table3B08C0 word690FE0; other entries/routes refuse.
 * Captures object2EF0, substate25CC, object54 byte and25D0 in original order.
 * After idle, reloads25D0; a change clears378 before reloading the retained
 * object's54 byte. A byte change clears37C. Guest aliases retain that order.
 * Record/object/storage ownership and original API are hypotheses; this is
 * a RAM adapter using the idle API's bounded numeric/mode/state/world domain.
 * The inherited wall W inputs are transported without an invented producer.
 * One exclusive writable main range<=16MiB below10000000 excludes original
 * guest stack/locals. Record16 alignment/31F0 bytes required. Request may alias
 * RAM; info is disjoint from request/RAM. Whole outer shadow publishes only on
 * success; any nested or tail refusal preserves RAM/info. Original return,
 * stack, complete banks, hardware flags/precision/timing, matching credit and
 * an initialized/live/full player frame remain outside this contract. */
int dreadzone_player_dispatch_00690F18(const DreadzonePlayerDispatchRequest *,DreadzonePlayerDispatchInfo *);
#endif
