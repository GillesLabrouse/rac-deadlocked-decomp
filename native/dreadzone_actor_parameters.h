#ifndef DEADLOCKED_DREADZONE_ACTOR_PARAMETERS_H
#define DEADLOCKED_DREADZONE_ACTOR_PARAMETERS_H
#include "common.h"
typedef struct DreadzoneActorParameterView {
    u8 *record;
    u32 record_guest, record_bytes;
    s32 mode_0021E824;
} DreadzoneActorParameterView;
enum DreadzoneActorParameterResult {
    DREADZONE_ACTOR_PARAMETERS_COMPLETED = 0,
    DREADZONE_ACTOR_PARAMETERS_INVALID_STORAGE = -1
};
/* Complete00669A20 parameter initialization. Writes the first16 bytes of18
 * blocks separated byB0 bytes, preserving their remaining storage and all
 * other actor fields. The six retail global reads are supplied as one stable
 * mode word; concurrent mode changes are outside this bounded host domain.
 * This is a constructor dependency, not a complete actor constructor. */
int dreadzone_actor_00669A20(const DreadzoneActorParameterView *);
#endif
