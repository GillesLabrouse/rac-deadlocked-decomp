#ifndef DEADLOCKED_DREADZONE_ACTOR_TRANSFORM_H
#define DEADLOCKED_DREADZONE_ACTOR_TRANSFORM_H
#include <stdint.h>
typedef struct DreadzoneActorTransform {
    uint8_t *storage;
    uint32_t guest_base,bytes,actor,actor_pool_count;
    uint8_t *scratch;
    uint32_t scratch_bytes;
    const uint8_t *vu_data;
    uint32_t vu_data_bytes;
} DreadzoneActorTransform;
/* Complete supported 0058DF58 RAM/SPR effects with actual basis, spatial and
 * initial-model components. Explicit RAM <=16MiB below2GiB, aligned actor,
 * disjoint RAM/SPR/VU. No actor50 blend list. Computed basis requires actor.FC
 * positive zero and supported XYZ angles; existing basis preserves W lanes.
 * Numeric operations use separate bounded IEEE binary32, not hardware VU
 * state/timing. Consumed offset/scale/position and every arithmetic intermediate
 * must be finite normal or zero; packed conversion requires signed32 range.
 * Spatial storage must obey its complete adapter contract, with
 * explicit pool count and original222D14 base. Reached model updates obey the
 * initial frame-zero/no-blend model contract. Return1/negative actor skip0,
 * invalid-1/unmapped-2/unsupported-3/host allocation-4. Every failure is atomic.
 * Direct missing outer mappings return-2; nested model/spatial refusals return-3.
 * This does not construct an actor/player, update a frame or render anything. */
int dreadzone_actor_transform(const DreadzoneActorTransform *);
#endif
