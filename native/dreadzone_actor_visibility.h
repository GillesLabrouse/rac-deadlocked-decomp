#ifndef DREADZONE_ACTOR_VISIBILITY_H
#define DREADZONE_ACTOR_VISIBILITY_H
#include "common.h"
typedef struct DreadzoneActorVisibilityRequest {
    u8 *storage;
    u32 guest_base,storage_bytes,actor_guest,mode_word;
} DreadzoneActorVisibilityRequest;
typedef struct DreadzoneActorVisibilityInfo {
    u32 skipped,halfword_stores,flags;
} DreadzoneActorVisibilityInfo;
/* Original L01 RAM effects only. No actor initialization or return-register ABI. */
int dreadzone_actor_visibility_005A8630(const DreadzoneActorVisibilityRequest *,DreadzoneActorVisibilityInfo *);
#endif
