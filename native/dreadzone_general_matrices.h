#ifndef DEADLOCKED_GENERAL_MATRICES_H
#define DEADLOCKED_GENERAL_MATRICES_H
#include "common.h"
typedef struct DreadzoneGeneralMatrices {
    u8 *scratch;
    u32 bytes;
    u32 joints;
} DreadzoneGeneralMatrices;

typedef struct DreadzoneLocalPoseInit {
    const u8 *skeleton;
    u32 skeleton_bytes;
    u8 *scratch;
    u32 scratch_bytes;
    u32 joints;
} DreadzoneLocalPoseInit;
enum { DREADZONE_GENERAL_MATRICES_APPROXIMATE=1,
       DREADZONE_GENERAL_MATRICES_INVALID=-1,
       DREADZONE_GENERAL_MATRICES_UNSUPPORTED=-2 };
/* CPU-visible scratch effects of genuine589950 + normal VU738/858. Local
 * records64 carry quaternion+0, scale+10, translation+20 and skeleton parent
 * metadata+3C. Parent127 selects the real identity bank3FC0; other parents
 * must precede the child. Scale inheritance follows actual parent bit80.
 * The full16KB scratch publishes atomically, including both matrix banks.
 * Scale/translation lanesw are not consumed by this normal transform path.
 * Finite normal host binary32 operations have explicitly approximate VU
 * rounding. No DMA transfer, synchronization or VU state effect is performed. */
int dreadzone_general_matrices_prepare(const DreadzoneGeneralMatrices *request);
/* Genuine589368 memory effects: skeleton16 -> local64, quaternionidentity,
 * scaleXYZ2^28/w0, translationXYZ*2^16, metadata.w preserved twice. count1..127.
 * Input skeleton and descriptors are captured before atomic output; aliasing
 * scratch is supported. Remaining scratch bytes are untouched. */
int dreadzone_local_pose_initialize(const DreadzoneLocalPoseInit *request);
#endif
