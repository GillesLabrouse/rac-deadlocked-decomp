#ifndef DEADLOCKED_DREADZONE_COLLISION_PUBLISH_H
#define DEADLOCKED_DREADZONE_COLLISION_PUBLISH_H
#include "common.h"

typedef struct DreadzoneCollisionPublishRequest {
    u8 *storage;
    u32 guest_base, storage_bytes, container_guest;
} DreadzoneCollisionPublishRequest;
typedef struct DreadzoneCollisionPublishInfo {
    u32 published_words, main_relocations, alternative_relocations;
} DreadzoneCollisionPublishInfo;
enum { DREADZONE_COLLISION_PUBLISH_OK=1, DREADZONE_COLLISION_PUBLISH_INVALID=-1, DREADZONE_COLLISION_PUBLISH_UNSUPPORTED=-2,
       DREADZONE_COLLISION_PUBLISH_MEMORY=-3 };

/* Complete selected integer RAM effects of L01 00577C70..00577D6C.
 * Relative container offsets and descriptor payloads remain guest32 words.
 * Each descriptor reloads the live global table and signed count after writing;
 * aliases follow actual reached ordering. Zero first offset preserves242340;
 * zero main/alternative offsets clear their real globals222BE0/222BE4.
 * One mapped range below2GiB, <=16MiB; naturally aligned word accesses.
 * At most65536 reached relocation iterations per table. Refusal is atomic.
 * Request is captured and may alias backing. Info is disjoint from both.
 * Success is an adapter status, not an invented original game return.
 * No asset placement, full level loader execution or player frame is claimed.
 */
int dreadzone_collision_publish_00577C70(const DreadzoneCollisionPublishRequest *,DreadzoneCollisionPublishInfo *);
#endif
