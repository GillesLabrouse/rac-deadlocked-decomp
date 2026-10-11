#ifndef DEADLOCKED_DREADZONE_GUEST_ACTOR_POOL_H
#define DEADLOCKED_DREADZONE_GUEST_ACTOR_POOL_H
#include <stdint.h>
typedef struct DreadzoneGuestActorAllocation {
    uint8_t *storage;
    uint32_t guest_base, bytes, gp, object_class, payload_bytes;
    uint8_t *scratch;
    uint32_t scratch_bytes;
    const uint8_t *vu_data;
    uint32_t vu_data_bytes;
} DreadzoneGuestActorAllocation;
/* Supported complete 005869E0 RAM/SPR effects, using real payload
 * allocation and fresh-class adapters. Result is a guest actor, or zero on
 * ordinary exhaustion; success1/exhaustion0, invalid-1/unmapped-2/unsupported-3/
 * host allocation-4. Refusals leave all caller backing and result unchanged.
 * Single disjoint <=16MiB RAM range below2GiB, base16 aligned, GP22597F,
 * explicit16KiB SPR/4KiB VU. Pool bounds must be16 aligned and span <=1024
 * complete100B actors. FF success requires mapped next-slot byte+120.
 * Positive payload <=255*64 bytes uses valid <=2048-word descriptors and
 * <=4096*64-byte payload arena, with mapped trailing lookahead word. Descriptor,
 * payload and actor-pool ranges cannot overlap each other or control words
 * 222D10..222D58.
 * Actor pool and reached FF following-byte writes cannot overlap control words
 * 222D10..222D58, clock21DDB8 or debug flag21EC28.
 * Reached fresh-model paths obey the fresh-class adapter's full domain.
 * Debug exhaustion callbacks are unsupported when21EC28 is nonzero.
 * This does not supply the occupied-pool boundary producer or construct a player.
 * Binary32/VU and immediate DMA remain approximate, EE stack/ABI excluded. */
int dreadzone_guest_actor_allocate(const DreadzoneGuestActorAllocation *, uint32_t *);
#endif
