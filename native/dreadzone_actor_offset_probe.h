#ifndef DEADLOCKED_DREADZONE_ACTOR_OFFSET_PROBE_H
#define DEADLOCKED_DREADZONE_ACTOR_OFFSET_PROBE_H
#include "common.h"
#include "dreadzone_wall_contact.h"

typedef struct DreadzoneOffsetProbeRequest {
    u8 *storage;
    u32 guest_base,storage_bytes,record_guest,output_guest;
    f32 local_z,limit;
    u32 actor_pool_count,grid_block_count;
    u32 incoming_vf25_29_w[5],incoming_local_w[2];
} DreadzoneOffsetProbeRequest;
typedef struct DreadzoneOffsetProbeInfo {
    s32 original_integer;
    u32 cache_updates,wall_calls;
    u32 outgoing_vf25_29_w[5];
    DreadzoneWallInfo wall;
} DreadzoneOffsetProbeInfo;
enum { DREADZONE_OFFSET_APPROXIMATE=1,DREADZONE_OFFSET_INVALID=-1,
    DREADZONE_OFFSET_UNSUPPORTED_PATH=-2,DREADZONE_OFFSET_UNSUPPORTED_FLOAT=-3,
    DREADZONE_OFFSET_TRANSACTION_MEMORY=-4 };

/* Complete selected 006708B0 RAM/return/W effects. Exclusive linear RAM,
 * <=64MiB below2GiB, disjoint scratchpad; byte-unaligned host backing is valid.
 * Genuine record31F0 is quad aligned. Inputs are actual F12/F13 and two prior
 * caller-local W words. No guest scratch mapping or local XYZ is fabricated.
 * Cache hits preserve five W words and do not read mode/radius/local W, output
 * or collision backing unless reached. Recomputed mode-zero probes use actual
 * mask2/cache0/ignored0 and mapped packet/grid/pool globals. Their established
 * numeric/geometry/index-bound contracts apply; unrecovered geometry refuses.
 * Reached wall calls preserve VF25..27.W and clear VF28/29.W. A hit computes
 * ordered binary32 distance including the original executed W square.
 * Scalars/intermediates are finite normal/zero; inputs |local_z|/|limit|<=16.
 * Hardware precision/status/timing and full machine state are not claimed.
 * output_guest zero means NULL; otherwise the genuine final hit stores there,
 * after cache/collision writes, preserving guest aliases. It is not read or
 * mapped on a miss. Request is captured and may alias RAM; info is disjoint
 * from both. All effects publish atomically; negative refusals preserve both.
 * Return1 is adapter completion, original_integer is the game's actual result.
 */
int dreadzone_actor_offset_probe_006708B0(const DreadzoneOffsetProbeRequest *,
    DreadzoneOffsetProbeInfo *);
#endif
