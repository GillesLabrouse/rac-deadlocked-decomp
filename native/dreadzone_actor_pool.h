#ifndef DEADLOCKED_DREADZONE_ACTOR_POOL_H
#define DEADLOCKED_DREADZONE_ACTOR_POOL_H

#include "common.h"

/* Host descriptors only. Every serialized address remains a 32-bit guest link.
 * The payload allocator is executable; the asset-dependent actor initializer
 * 00586B10 and full actor allocation are not supplied by this interface. */
typedef struct DreadzonePayloadState {
    u32 head;
    u32 tail;
    u32 cursor;
} DreadzonePayloadState;
typedef struct DreadzonePayloadArena {
    u32 descriptor_base;
    u32 descriptor_count;
    u32 *descriptors;
    u32 payload_base;
    u32 payload_bytes;
    u8 *payload;
} DreadzonePayloadArena;

/* Returns a guest payload address, or zero without mutation on failure.
 * Validated host domain: up to 2048 descriptors, 4096 payload blocks and
 * nonzero requests up to 255 blocks. Allocation leaves payload bytes untouched. */
u32 dreadzone_payload_allocate(const DreadzonePayloadArena *arena,
                              DreadzonePayloadState *state, u32 bytes);
/* Applies the real actor allocator's payload-clear step after allocation.
 * It writes full four-byte words, rounding a positive byte count upward to
 * four. The remaining 64-byte allocation padding stays untouched. */
u32 dreadzone_payload_allocate_zeroed(const DreadzonePayloadArena *arena,
                                     DreadzonePayloadState *state, u32 bytes);
/* Frees an exact, 64-byte-aligned allocation start. Invalid inputs return
 * zero without mutation; retail instead traps for an unknown allocation. */
int dreadzone_payload_release(const DreadzonePayloadArena *arena,
                             DreadzonePayloadState *state, u32 address);
u8 *dreadzone_payload_translate(const DreadzonePayloadArena *arena,
                               u32 address, u32 bytes);

#endif
