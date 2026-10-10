#ifndef DEADLOCKED_DREADZONE_BIND_POSE_H
#define DEADLOCKED_DREADZONE_BIND_POSE_H
#include "dreadzone_model_pool.h"

enum {
    DREADZONE_BIND_APPROXIMATE = 1,
    DREADZONE_BIND_INVALID = -1,
    DREADZONE_BIND_UNMAPPED = -2,
    DREADZONE_BIND_UNSUPPORTED = -3
};
typedef struct DreadzoneBindPose {
    const DreadzoneModelView *views;
    u32 view_count;
    u32 pose_guest;
    u32 bind_guest;
    s32 count;
    u8 *scratch;
    u32 scratch_bytes;
} DreadzoneBindPose;

/* Complete CPU-visible scratch effects of 00589B60 for count0..127.
 * Pose records are64 bytes (four columns); bind records48 bytes (three rows).
 * The actual A40/AA8 order is pose * bind, including translation in each
 * bind row's fourth component. No inferred inverse-bind matrix is constructed.
 * count0 writes one identity64 record only. For a nonzero count, copies both
 * inputs into scratch, then replaces the first count*64 bytes with products.
 * pose_guest70000000 reads the existing scratch snapshot, as the retail bypass
 * does. Other views may not map scratch guest addresses or alias host scratch.
 * All descriptors are captured before writes. Failure leaves outputs intact.
 * APPROXIMATE denotes host binary32 rounding; no DMA/VIF or VU state executes.
 */
int dreadzone_bind_pose_prepare(const DreadzoneBindPose *request);
#endif
