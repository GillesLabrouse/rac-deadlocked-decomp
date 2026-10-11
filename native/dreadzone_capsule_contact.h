#ifndef DEADLOCKED_DREADZONE_CAPSULE_CONTACT_H
#define DEADLOCKED_DREADZONE_CAPSULE_CONTACT_H
#include "common.h"

typedef struct DreadzoneCapsuleRequest {
    u8 *storage;
    u32 guest_base,storage_bytes;
    u32 center_guest,packet_guest,grid_guest,actor_pool_pointer_guest;
    u32 actor_pool_count,ignored_actor_guest,caller_flags;
    float radius,height;
    u32 incoming_vf25_29_w[5];
} DreadzoneCapsuleRequest;

typedef struct DreadzoneCapsuleInfo {
    u32 selected_cells,cells_processed,faces_processed,quad_faces_processed;
    u32 tree_calls,dma_memory_copies,actor_candidates,actors_ignored;
    u32 actors_filtered,actors_marked,write_words,q_bits,q_exception_bits;
} DreadzoneCapsuleInfo;

enum {DREADZONE_CAPSULE_INVALID=-1,DREADZONE_CAPSULE_UNSUPPORTED=-2};

/* Bounded CPU execution of the genuine vertical capsule probe 0053FAA8.
 * Radius and height are actual independent f12/f13 inputs; center is a0,
 * flags are a1 and ignored_actor_guest is a2. packet.word0 supplies the real
 * collision tree. grid_guest and actor_pool_pointer_guest name the actual
 * dynamic-cell table and actor-base global (L01: 26FF20 and222D14).
 *
 * A complete terrain/quad contact and actual dynamic-list filtering are
 * supported. An eligible dynamic geometry/primitive candidate is UNSUPPORTED;
 * it is never treated as an empty list or a fabricated miss. Actual ignored,
 * visited, broadphase-rejected and definition/flag-filtered actors execute.
 * The returned0/1 is the original probe result in this explicit domain,
 * including generation and visitation writes and final packet publication.
 *
 * Guest RAM is one writable, nonwrapping range below2 GiB, disjoint from
 * scratch70000000..3FFF. Natural guest alignment and all genuine source reads
 * are checked. Positive radius/height, normal-or-zero finite IEEE inputs and
 * intermediates, positive lower bounds and upper bounds<1024 are supported.
 * Spatial cells are bounded to0..63, terrain selections<=127, actor lists<=255,
 * actor pool<=65536, packed geometry with valid indices and nonzero vertex_count, valid indices and quad_count<=nf.
 * Ordered host-f32 and saturated Q/I-D are declared numerical approximations,
 * not physical VU rounding, status timing or DMA synchronization. Incoming
 * unsaved VF25..29 W lanes are explicit caller state, never fabricated fields.
 *
 * Request is captured before execution and may alias RAM. Info must be
 * disjoint from request and RAM. Every refusal preserves RAM and info; a real
 * miss still publishes the actual generation/visitation effects. Caller must
 * provide exclusive access while this snapshot transaction executes.
 * This does not execute a player frame, ground/wall response or rendering.
 */
int dreadzone_capsule_contact(const DreadzoneCapsuleRequest *request,
                             DreadzoneCapsuleInfo *info);
#endif
