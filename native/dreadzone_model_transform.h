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

/* Complete CPU-visible model-storage effects of the normal static-animation
 * path through 0058A648, 0058A440, 0058A258, 00589368 and 00589950.
 * Supported: 1..127 joints, animation stream byte2 == 0, no actor55 bit4,
 * no actor50/5C lists, root parent127 and subsequent earlier parents or127.
 * Unsupported branches and invalid mappings leave every output unchanged.
 * The scratchpad's complete 16 KiB backing, matrices and dirty bit publish
 * together. Guest pointers remain numeric32; every host descriptor is copied
 * before writes. Views and scratch must have disjoint backing.
 * APPROXIMATE means finite normal host float arithmetic, not VU bit-equivalent
 * rounding. This neither executes DMA/VIF nor changes VU register/data state;
 * it completes the explicitly supported CPU-visible memory effects only.
 */
int dreadzone_model_transform_static(const DreadzoneModelTransform *request);
#endif
