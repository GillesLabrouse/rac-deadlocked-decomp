#ifndef DEADLOCKED_DREADZONE_POSE_MASKS_H
#define DEADLOCKED_DREADZONE_POSE_MASKS_H
#include "dreadzone_model_pool.h"

typedef struct DreadzonePoseMasks {
    u8 *scratch;u32 scratch_bytes;
    u8 *vu_data;u32 vu_data_bytes;
    const DreadzoneModelView *views;u32 view_count;
    s32 first_joint,joint_count;
    u32 first_list_guest,second_list_guest;
} DreadzonePoseMasks;
enum DreadzonePoseMasksStatus {
    DREADZONE_POSE_MASKS_COMPLETED=0,
    DREADZONE_POSE_MASKS_INVALID=-1,
    DREADZONE_POSE_MASKS_NEEDS_MEMORY=-2,
    DREADZONE_POSE_MASKS_UNSUPPORTED=-3
};
/* Complete CPU scratch/VU data effects of5893F0. Each reached byte list uses
 * its genuine halfword offset and signed joint IDs. The first list governs
 * the finite caller range, then the real ending sentinel is set. A second
 * list (only when the first list is nonnull) starts at its actual first ID and clears only selected records until
 * the negative terminator; gaps retain their incoming mask values.
 *
 * Required banks are16KiB scratch and4KiB VU data, host-disjoint. Scratch is
 * implicitly mapped at70000000, allowing actual lists to alias ordered writes.
 * Up to64 disjoint guest/host source views provide ordinary list backing.
 * Views cannot map scratch or alias either bank/descriptors. Source backing
 * and descriptors must remain stable/exclusively borrowed. The request is
 * captured; descriptors and all reads are validated before atomic publication.
 * First range uses0..127 and positive count with end<=128; second list must
 * finish within0..127. Other domains refuse rather than simulate a return.
 * Both full banks retain every byte except real metadata writes. Integer
 * effects are exact within this domain; VU register state, timing and blend
 * arithmetic are excluded. This producer does not itself decode a pose. */
int dreadzone_pose_masks_prepare(const DreadzonePoseMasks *);
#endif
