#ifndef DEADLOCKED_DREADZONE_PLACED_ACTORS_H
#define DEADLOCKED_DREADZONE_PLACED_ACTORS_H
#include <stdint.h>
typedef struct DreadzonePlacedActors {
    uint8_t *storage;
    uint32_t guest_base,bytes,gp,header,output_cell,mode,actor_pool_count;
    uint8_t *scratch;
    uint32_t scratch_bytes;
    const uint8_t *vu_data;
    uint32_t vu_data_bytes;
} DreadzonePlacedActors;
/* Complete supported0057B788 RAM/SPR composition. Cached upload
 * descriptor220160 must equal1170D0; no MMIO upload is supplied. Published
 * tables/resources are explicit. <=1024records, mode0/nonzero index offsets,
 * actual mutable record loads, mapping table243800 and per-record definition
 * flags are preserved. Scale multiplication requires finite normal-or-zero
 * inputs and result; nested transform/model numeric limits also apply.
 * Direct outer unmapped spans return-2; nested component refusals return-3.
 * Height requests, environment58FC50 and special7D0
 * callbacks are unsupported. Fresh/model/spatial domains remain bounded.
 * Return1, invalid-1/unmapped-2/unsupported-3/hostallocation-4. Atomic RAM/SPR.
 * No selected mission, complete player, original ABI/timing or gameplay claim. */
int dreadzone_placed_actors_load(const DreadzonePlacedActors *);
#endif
