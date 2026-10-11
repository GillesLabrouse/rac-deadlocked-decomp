#ifndef DEADLOCKED_DREADZONE_ACTOR_POSITION_STEP_H
#define DEADLOCKED_DREADZONE_ACTOR_POSITION_STEP_H
#include "common.h"

typedef struct DreadzonePositionStepRequest {
    u8 *storage;
    u32 guest_base,storage_bytes,record_guest,actor_pool_count,grid_block_count;
    u32 incoming_vf25_29_w[5];
    u32 incoming_ground_side_w[2];
} DreadzonePositionStepRequest;
typedef struct DreadzonePositionStepInfo {
    u32 object_exits,capsule_resolve_calls,ground_calls,settle_calls;
    u32 capsule_calls,static_calls,probe_calls,ground_contacts;
    u32 position_changed,outgoing_vf25_29_w[5];
} DreadzonePositionStepInfo;
enum DreadzonePositionStepResult {
    DREADZONE_POSITION_STEP_APPROXIMATE=1,
    DREADZONE_POSITION_STEP_INVALID=-1,
    DREADZONE_POSITION_STEP_UNSUPPORTED_PATH=-2,
    DREADZONE_POSITION_STEP_UNSUPPORTED_FLOAT=-3,
    DREADZONE_POSITION_STEP_TRANSACTION_MEMORY=-4
};

/* Complete selected mode-zero 0067F5E8 CPU effects, not a playable frame.
 * One exclusive writable naturally aligned RAM range below2GiB, <=16MiB,
 * excludes scratch70000000..70003FFF. Genuine record31F0, globals, actor,
 * history, packet242340, grid26FF20 and actual collision backing are inputs.
 * No clock/radius/history/geometry/object pointer is manufactured.
 * Request is captured before writes and may alias RAM. Info is disjoint from
 * request/backing. All ordered aliases operate on a full transaction image;
 * every nested refusal leaves caller RAM and info unchanged.
 *
 * Real 68B258 prefix reads and writes execute when world21E824!=1. Only its
 * complete s4==0 exit is supported. The first reached active lookup598830
 * refuses, including nonnull metadata-negative objects: no return is captured.
 * Selected mode1/2 record transforms and reached unrecovered capsule/ground
 * object/sphere/recovery branches refuse through their genuine APIs.
 *
 * Caller VF25..29 W and both prior ground side-local W words are explicit.
 * Capsule/static preserves the five W values; ground publishes its actual
 * outgoing state. After the first settling call, original EA88 67EBEC
 * publishes A0.word3 into a0.word3 when half2FE==0. Otherwise the definite
 * reached ground publisher supplies that word; a scheduled ground preserves
 * unknown capsule residue and refuses. A3F8 transports this real high GPR lane
 * into C60, which writes VF28.W through its actual W polynomial tail.
 * Its input is finite normal/zero, |W|<=pi and nonzero |W|>=2^-40; W=1 is
 * supported. The actual active secondary-position branch and the scheduled
 * ground/nonzero-half settling path refuse atomically: their inherited capsule
 * high GPR provenance is not recovered. Unknown internal bookkeeping is never
 * consumed by C60 or published as recovered caller state.
 * This adapter follows original W effects; the current SN draft differs in
 * register allocation and is independently nonmatching for these W effects.
 * Other VF/ACC/Q state, hardware exception flags and timing are not claimed.
 * Raw ignored lanes are copied. Consumed scalar/vector fields are finite
 * normal/zero with abs>=2^-40, vectors<=16, positions<=8192. Selected clock
 *0..16 has nonzero floor2^-20. Selected caps require nonnegative length;
 * exceptional/subnormal intermediates refuse. Existing genuine helper
 * numerical/geometry domains also apply. Host binary32 and saturated Q are
 * explicit approximations; no original game success return is invented.
 */
int dreadzone_actor_position_step_0067F5E8(const DreadzonePositionStepRequest *,
                                        DreadzonePositionStepInfo *);

/* Additive complete selected mode-zero position entry with explicit raw caller
 * a0.word2/3. This admits actual active140 and scheduled/nonzero-half routes.
 * Reached mode-zero prefix/velocity leaves preserve these lanes; actual ground
 * and half-zero settling publishers replace them before the heading C60 W.
 * Every reached capsule uses its strict provenance entry: an actual positive
 * final supplementary count or genuine early exit is required. Unknown capsule
 * residue refuses even if a later ground call could overwrite it. The second
 * phase uses actual67FD74 PCPYUD's duplicated140 words2/3, not cleared high64.
 * All existing numerical/dependency/transaction restrictions apply. Incoming
 * lanes may alias request, info or backing and are captured before execution.
 * Original request/info layouts and the older entry's domain are unchanged.
 * Only RAM, reported counters and five outgoing W lanes are published: final
 * full GPR/VF/ACC/Q, live caller provenance and a full player frame are separate.
 */
int dreadzone_actor_position_state_0067F5E8(const DreadzonePositionStepRequest *,
                                         const u32 incoming_high64[2],
                                         DreadzonePositionStepInfo *);
#endif
