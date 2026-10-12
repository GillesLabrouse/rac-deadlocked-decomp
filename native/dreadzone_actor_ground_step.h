#ifndef DEADLOCKED_DREADZONE_ACTOR_GROUND_STEP_H
#define DEADLOCKED_DREADZONE_ACTOR_GROUND_STEP_H
#include "common.h"
#include "dreadzone_probe_registers.h"

enum DreadzoneGroundOperation {
    DREADZONE_GROUND_UPDATE=0, /* 0067DDD0; integer0 is genuine force. */
    DREADZONE_GROUND_ENDPOINTS=1, /* 0067DBE8; value/target, integer0, scalar0/1. */
    DREADZONE_GROUND_CLIP=2, /* 00596390; value/target are first/second. */
    DREADZONE_GROUND_HEIGHT=3, /* 00593478; captured value quad, target normal. */
    DREADZONE_GROUND_PACKET_FLAGS=4, /* 00543028, genuine shared packet. */
    DREADZONE_GROUND_HISTORY=5, /* 00686F98; integer0/1 are command/time bound. */
    DREADZONE_GROUND_SURFACE_EULER=6, /* 006824E8; value quad, target two words. */
    DREADZONE_GROUND_SIDE_OFFSET=7 /* 0068A340; target, integer0, scalar0/1/2. */
};
typedef struct DreadzoneGroundRequest {
    u8 *storage;
    u32 guest_base,storage_bytes,record_guest,operation,value_guest,target_guest;
    s32 integer0,integer1;
    f32 scalar[3];
    u32 actor_pool_count,grid_block_count;
    u32 incoming_vf25_29_w[5];
    u32 incoming_side_output_w[2];
} DreadzoneGroundRequest;
typedef struct DreadzoneGroundInfo {
    s32 original_integer;
    f32 original_float;
    u32 has_integer_return,has_float_return;
    u32 probe_calls,side_probe_calls,ground_contacts;
    u32 outgoing_vf25_29_w[5];
} DreadzoneGroundInfo;
enum DreadzoneGroundResult {
    DREADZONE_GROUND_APPROXIMATE=1,
    DREADZONE_GROUND_INVALID=-1,
    DREADZONE_GROUND_UNSUPPORTED_PATH=-2,
    DREADZONE_GROUND_UNSUPPORTED_FLOAT=-3,
    DREADZONE_GROUND_TRANSACTION_MEMORY=-4
};

/* Complete selected original CPU effects in exclusive, authentic caller RAM.
 * A single naturally aligned nonwrapping range below2GiB, at most64MiB,
 * excludes scratchpad70000000..70003FFF. Actual selected records, body,
 * history, globals and collision backing must be present. No contact, state,
 * radius, clock or persistent region is manufactured. Record stride31F0 and
 * quad alignment16 are retained. The request is captured before writes and
 * may alias RAM; info is disjoint from request and backing. Refusals preserve
 * all backing and info. Ordered guest aliases execute on full shadow images.
 *
 * UPDATE is the complete ordinary mode-zero selected DDD0 path, including
 * genuine scheduling, probes, counters, contact response and selected side
 * probes. Unrecovered reached spherical/dynamic/override dependencies refuse
 * atomically. Both actual side-local prior W words must be supplied; their XYZ
 * lanes are overwritten by A340. Wall calls clear VF28/29.W while preserving
 * VF25..27.W; this outgoing state is explicit, never reset per iteration.
 * Probe endpoints are captured actual caller locals, not invented guest RAM.
 *
 * Numeric operations use individually rounded normal/zero host binary32.
 * Selected positions<=8192, vectors/matrix coefficients<=16, time0..16;
 * nonzero selected values have magnitude>=2^-40, except time>=2^-20.
 * Genuine C60 and55D708 helpers execute. Selected raw cross normals are
 * bounded by2^30, without invented normalization before
 * atan inputs. Nonzero atan ratios require abs>=2^-6; zero remains supported.
 * Selected original numeric tables3B9540/3B9580 must be present and unchanged.
 * The bounded history search accepts signed count<=8; larger counts refuse.
 * Exceptional/subnormal selected intermediates refuse. Physical VU ACC
 * precision/flags/timing and
 * dynamic geometry, whole position/frame/actor construction remain separate.
 * Integer/float returns are flagged only for real returning leaf entries;
 * void DDD0 never invents a success return from the original game.
 */
int dreadzone_actor_ground_step(const DreadzoneGroundRequest *,DreadzoneGroundInfo *);

/* UPDATE-only composition of actual A0.word2/word3 with the above RAM/VF effects.
 * Capture the caller's incoming words before writes; incoming may alias output
 * or backing. Output is disjoint from request, info and backing, and publishes
 * only after the full transaction succeeds. Refusals preserve all three outputs.
 * Diagnostic last_writer_pc is zero for preserved state, otherwise the actual
 * reached producer PC. This transport does not model the other GPR/VF lanes.
 * The special593548 height producer squares actual delta-Z and input-W before
 * testing radius; output/global aliases retain that instruction order. Reached
 *2237D8 object lookup remains unsupported. The established RAM-only operations
 * retain their original bounded behavior. */
int dreadzone_actor_ground_registers(const DreadzoneGroundRequest *,const u32 incoming_high64[2],
    DreadzoneProbeRegisters *,DreadzoneGroundInfo *);
/* Selected mode-zero68A340 effect on a captured caller-local quad. Operation
 * must be SIDE_OFFSET; scalar0/1/2 and integer0 are the actual distance, angle,
 * local Z and add-position inputs. target_guest/value_guest are unused.
 * No temporary guest mapping is created. RAM is read-only and byte-unaligned
 * host backing is allowed; the existing guest/domain checks still apply.
 * Request and input words are captured before computation. Input may alias
 * output, request, info or backing. Output is disjoint from request/info/backing;
 * info is disjoint from request/backing/output. Both publish only on success.
 * Original mapped operation7 and its intermediate alias writes are unchanged.
 * Five incoming W words are preserved; other machine banks, physical precision,
 * status/timing and live caller binding remain outside this numeric contract.
 * Return1 complete adapter effects, negative on atomic domain/mapping refusal.
 */
int dreadzone_side_offset_captured_quad(const DreadzoneGroundRequest *,
    const u32 input_quad[4],u32 output_quad[4],DreadzoneGroundInfo *);
#endif
