#ifndef DEADLOCKED_DREADZONE_COLLISION_TRIANGLE_H
#define DEADLOCKED_DREADZONE_COLLISION_TRIANGLE_H
#include "common.h"

typedef struct DreadzoneCollisionTriangleRequest {
    float vertex[3][3];
    float cell_origin[3];
    float start_relative[3],end_relative[3],direction[3];
    float best_fraction;
    u32 clip_flags[3],effective_flags,surface,source_guest;
} DreadzoneCollisionTriangleRequest;
typedef struct DreadzoneCollisionTriangleContact {
    float fraction,position_scaled[3],normal[3],vertices_scaled[3][3];
    u32 source_guest,hit_metadata;
} DreadzoneCollisionTriangleContact;
typedef struct DreadzoneCollisionTriangleQStatus {
    u32 division_executed,q_bits,exception_bits;
} DreadzoneCollisionTriangleQStatus;
enum {
    DREADZONE_TRIANGLE_MISS=0,
    DREADZONE_TRIANGLE_APPROXIMATE_CONTACT=1,
    DREADZONE_TRIANGLE_INVALID=-1,
    DREADZONE_TRIANGLE_UNSUPPORTED=-2
};

/* Complete single-triangle iteration0053E1E8..0053E384 within53DC70:
 * common clip/surface filters, plane crossing, Q division, three edge tests,
 * nearest-fraction rejection and actual XYZ contact publication.
 * vertices are the decoded cell-relative VF1..3; clip_flags are their raw
 * +C words. cell_origin=VF16, relative endpoints=VF14/15, direction=VF13.
 * They are separately supplied because original f32 subtraction order matters.
 * effective_flags is the genuine caller flags XOR0x10; surface is face.byte3.
 * best_fraction is live VF24.x, normally initialized to1 by the full probe.
 * source_guest is the original t8 terrain backing address. Success preserves
 * it in the contact (s5) and supplies surface+0x1000 as hit_metadata (s6).
 * Position and vertices remain in the original1024-scaled coordinate units;
 * normal is the unnormalized edge cross product. The probe's final scaling,
 * packet/cache publication, actor tests, scratch/DMA and VU save/restore are
 * outside this iteration. No whole-probe/frame result is supplied here.
 * Finite normal/zero values and intermediates, best in[0,1], valid packed clip
 * bits and nonzero finite-normal division denominator are supported. Explicit
 * UNSUPPORTED preserves output, including a zero-denominator/grazing case.
 * MISS also preserves output. Input/output overlap is allowed by snapshot.
 * CONTACT is an ordered host-IEEE approximation of actual macro-VU/Q math,
 * not physical VU rounding/flush/flags/register/pipeline validation.
 */
int dreadzone_collision_triangle(const DreadzoneCollisionTriangleRequest *request,
                                DreadzoneCollisionTriangleContact *contact);
/* Same complete calculation, additionally representing the actual VDIV
 * zero-denominator saturation instead of refusing it. Q receives signed
 * 0x7F7FFFFF; exception_bits is0x10 for0/0 and0x20 for nonzero/0, otherwise0.
 * This is a numerical diagnostic, not the physical VU status register, timing
 * or exception pipeline. All original plane and edge branches still execute;
 * a saturated Q is never treated as an automatic miss or successful contact.
 * If a later operation leaves the supported finite-normal/zero domain, both
 * outputs remain unchanged. On MISS, contact stays unchanged and q_status
 * reports whether division executed (early filters can skip it entirely).
 * On CONTACT both outputs are supplied. The two outputs must be disjoint;
 * either may overlap the request, which is snapshotted before computation.
 */
int dreadzone_collision_triangle_with_q(const DreadzoneCollisionTriangleRequest *request,
                                       DreadzoneCollisionTriangleContact *contact,
                                       DreadzoneCollisionTriangleQStatus *q_status);
#endif
