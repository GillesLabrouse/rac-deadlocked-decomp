#ifndef DEADLOCKED_DREADZONE_ACTOR_BASIS_H
#define DEADLOCKED_DREADZONE_ACTOR_BASIS_H
#include "dreadzone_angle_pair.h"

enum {
    DREADZONE_BASIS_APPROXIMATE=1,
    DREADZONE_BASIS_INVALID=-1,
    DREADZONE_BASIS_UNSUPPORTED=-2
};

/* Complete CPU-visible basis result of resident VU0 B58, including all
 * active X/Y/Z rotations, actual C40 polynomial and B10 ACC-order products.
 * Input is actor.F0.xyz, not quaternion/model geometry. Output is four
 * consecutive quads corresponding to vf20,21,22,23; the last is VF0.
 * Only exact zero angles skip their genuine rotation, as B58's FMAND does.
 * Finite normal/zero angles in[-pi,pi] with nonzero magnitude>=2^-40,
 * the angle-pair component's explicit signed-subnormal-result flushing,
 * and finite normal/zero basis composition intermediates
 * are supported. Refusal leaves the entire output unchanged. Input/output
 * may overlap, since input is snapshotted before computation/publication.
 * APPROXIMATE means ordered host IEEE binary32, not physical VU flags,
 * rounding, pipeline timing or register state. This neither performs actor
 * animation/spatial/model update nor claims complete player/frame execution.
 */
int dreadzone_actor_basis(const float angles_xyz[3],float output_quads[16]);
#endif
