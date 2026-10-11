#ifndef DEADLOCKED_DREADZONE_MODEL_TRANSFORM_H
#define DEADLOCKED_DREADZONE_MODEL_TRANSFORM_H
#include "dreadzone_model_pool.h"

enum {
    DREADZONE_TRANSFORM_APPROXIMATE = 1,
    DREADZONE_TRANSFORM_NO_MODEL = 0,
    DREADZONE_TRANSFORM_INVALID = -1,
    DREADZONE_TRANSFORM_UNSUPPORTED = -2,
    DREADZONE_TRANSFORM_UNMAPPED = -3,
    DREADZONE_TRANSFORM_SCRATCH_BYTES = 0x4000
};

typedef struct DreadzoneModelTransform {
    const DreadzoneModelView *views;
    u32 view_count;
    u32 actor;
    u8 *scratch;
    u32 scratch_bytes;
} DreadzoneModelTransform;

/* Reserved model transform entry; nonempty models are currently UNSUPPORTED.
 * Original 0058A258 calls decoder 005894F0 even with zero variable qwords.
 * Closing its fixed compressed channels is required before this entry may
 * prepare matrices or clear the actor dirty bit. The earlier identity-only
 * implementation and its completion guarantee have been withdrawn.
 * Null actor/model returns NO_MODEL. Invalid host descriptors or unmapped
 * actor backing retain distinct statuses. Every status leaves all caller
 * memory unchanged; views and scratch must have disjoint backing.
 * APPROXIMATE is retained for API compatibility but is never returned.
 */
int dreadzone_model_transform_static(const DreadzoneModelTransform *request);
#endif
