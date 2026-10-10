#ifndef DREADZONE_ANIMATION_LAYERS_H
#define DREADZONE_ANIMATION_LAYERS_H

#include "dreadzone_actor_animation.h"

typedef struct DreadzoneAnimationLayersRequest {
    const DreadzoneAnimationContext *memory;
    u32 actor_guest;
    /* These must equal the actual actor+40/+48 inputs, not replace them. */
    u32 selected_definition_guest;
    u32 effective_rate_bits;
} DreadzoneAnimationLayersRequest;

enum DreadzoneAnimationLayersResult {
    DREADZONE_ANIMATION_LAYERS_INVALID = -1,
    DREADZONE_ANIMATION_LAYERS_UNSUPPORTED = -2,
    DREADZONE_ANIMATION_LAYERS_NO_MEMORY = -3,
    DREADZONE_ANIMATION_LAYERS_APPROXIMATE = 1
};

/* Complete CPU memory effects of589CE0, with genuine58ACB8/58A8D8 callees.
 * Existing disjoint views/pool constraints apply. Up to32 MiB of backing is
 * snapshotted. No caller RAM is mutated unless all operations succeed; caller
 * storage must remain exclusively borrowed for the call. This is refusal
 * atomicity, not simultaneous stores observable by another running thread.
 *
 * The primary clock's proved time block is evaluated in an independent local
 * descriptor. Only frame/duration/flags are copied back; real guest head/next
 * links are read and retained by this entry. No guest layer is discarded to
 * satisfy the component's primary-only contract. Genuine pre-cleanup branches,
 * weight subtraction, unlink/release and post-release reloads execute on the
 * shadow views in retail order, including metadata aliases into later nodes.
 *
 * Numeric domain follows dreadzone_animation_clock.h. Secondary nodes are
 * bounded to the actual32-slot pool and the temporal chain must terminate;
 * cyclic temporal chains do not return in retail and are refused atomically.
 * Actor must be disjoint from the node pool footprint. Aliases rejected by
 * the clock/release/cleanup components remain refused. Finite weight/decay
 * magnitudes are limited to1048576. EE arithmetic remains approximate.
 * No event wrapper, clip selection, display cadence or DMA is performed. */
int dreadzone_animation_layers_advance(const DreadzoneAnimationLayersRequest *);

#endif
