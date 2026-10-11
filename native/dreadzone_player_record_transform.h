#ifndef DEADLOCKED_DREADZONE_PLAYER_RECORD_TRANSFORM_H
#define DEADLOCKED_DREADZONE_PLAYER_RECORD_TRANSFORM_H
#include <stdint.h>
typedef struct DreadzonePlayerRecordTransform {
    uint8_t *storage;
    uint32_t guest_base,bytes,record,actor_pool_count;
    uint8_t *scratch;
    uint32_t scratch_bytes;
    const uint8_t *vu_data;
    uint32_t vu_data_bytes;
} DreadzonePlayerRecordTransform;
/* Complete supported66E580 RAM/SPR path. Real actor transform and
 * both collision-volume updates are composed. Nonoscillator states only;
 * genuine52C/4D0 rotation override remains. Volume height mode0 only, with
 * finite normal/zero consumed inputs and intermediates, full actual coordinate
 * range subject to nested transform/volume bounds. Explicit original mappings,
 * pool, disjoint RAM/SPR/VU. No cleanup, construction, original stack/register ABI, VU register
 * effects or repeated frame claim.
 * Return1 success,-1invalid,-2direct unmapped,-3unsupported,-4allocation.
 * Every failure is atomic; borrowed request is captured first. */
int dreadzone_player_record_transform(const DreadzonePlayerRecordTransform *);
#endif
