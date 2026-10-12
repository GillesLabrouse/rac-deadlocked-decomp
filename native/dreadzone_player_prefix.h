#ifndef DEADLOCKED_DREADZONE_PLAYER_PREFIX_H
#define DEADLOCKED_DREADZONE_PLAYER_PREFIX_H
#include "dreadzone_player_dispatch.h"
#include "dreadzone_actor_mode.h"
#include "dreadzone_slot_word.h"
typedef DreadzonePlayerDispatchRequest DreadzonePlayerPrefixRequest;
typedef struct DreadzonePlayerPrefixInfo {
    DreadzoneSlotWordInfo slot;
    DreadzoneActorModeInfo mode;
    DreadzonePlayerDispatchInfo dispatch;
    u32 stub_calls,getter_calls,slot_calls,mode_calls,dispatch_calls,getter_index;
    u32 retained_f0_words[4];
} DreadzonePlayerPrefixInfo;
/* Selected RAM prefix67C010 up to PRE-instruction67C1B0, before histories and
 * movement basis. Runs actual no-op5A8628 RAM leaf, validates/reloads actual
 * vtable targets6A9630 and690F18, invokes recovered getter, slot-word, mode
 * and dispatcher algorithms in original order. Stores mode's low byte exactly
 * as SB; its full integer result is separate. Copies A0->110 as raw quad and
 * retains the final F0 quad corresponding to original stack+10 for later tail.
 * This is that one caller-local value, not an original stack/bank publication.
 * Requires GP22597F, initial/late nonzero266D, mode word21E824!=1 at both reached
 * tests and substate0. Nested supported numeric/world domains still apply;
 * original alternate callbacks/routes refuse, without invented answers.
 * One stable exclusive writable main range<=16MiB below10000000, record16
 * alignment/31F0 bytes, disjoint info and captured request/backing aliases.
 * Whole outer shadow publishes only after selected prefix success. APIs/types/
 * ownership remain hypotheses. Full67C010, initialization, original return,
 * bank/status/timing equivalence, matching credit and live gameplay excluded. */
int dreadzone_player_prefix_0067C010(const DreadzonePlayerPrefixRequest *,DreadzonePlayerPrefixInfo *);
#endif
