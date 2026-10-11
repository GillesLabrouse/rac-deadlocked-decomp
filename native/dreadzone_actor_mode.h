#ifndef DEADLOCKED_DREADZONE_ACTOR_MODE_H
#define DEADLOCKED_DREADZONE_ACTOR_MODE_H
#include "common.h"

typedef struct DreadzoneActorModeRequest {
    const u8 *storage;
    u32 guest_base,storage_bytes,record_guest;
} DreadzoneActorModeRequest;
typedef struct DreadzoneActorModeInfo { s32 mode; u32 metadata_queries; } DreadzoneActorModeInfo;

/* Complete selected reads and integer result of L01 0068A090. Its reached
 * 006BF278 query reads actual metadata byte +EF8 for index18; no answer is
 * supplied by the caller. The original record+2F8 signed halfword is retained,
 * including values outside0..2. This API does not publish record+2659.
 * One stable exclusive RAM range <=16MiB below2GiB, including the complete
 * aligned31F0 record, actual globals and any reached metadata, is required.
 * Request is captured before result writes; info must be disjoint from both
 * request and backing. Missing or misaligned selected reads return-1 with
 * unchanged RAM and info. Success1 is an adapter status; mode is the actual
 * original return. No constructor, movement or frame completion is supplied.
 */
int dreadzone_actor_mode_0068A090(const DreadzoneActorModeRequest *,DreadzoneActorModeInfo *);
#endif
