#ifndef DEADLOCKED_DREADZONE_TERRAIN_CONTACT_H
#define DEADLOCKED_DREADZONE_TERRAIN_CONTACT_H
#include "dreadzone_collision_triangle.h"
#include "dreadzone_probe_registers.h"
typedef struct DreadzoneTerrainContactRequest {
    const u8 *storage;
    u32 guest_base,bytes,root;
    float start_world[3],end_world[3];
    u32 caller_flags;
} DreadzoneTerrainContactRequest;
typedef struct DreadzoneTerrainContactInfo {
    u32 grid_records,cells_processed,faces_processed,quad_faces_processed;
    u32 grid_zero_q_mask,triangle_q_exceptions;
} DreadzoneTerrainContactInfo;
/* CPU terrain-contact stage of0053DC70. Uses the genuine ordered grid, tree
 * lookup, cell-clipped packed vertex/clip decode, primary faces, quad mutation
 * pass and complete saturated-Q triangle kernel. Prefetch lookup/copy precedes
 * current-cell decoding. Physical X/Y/Z are remapped to the tree helper's
 * actual at/v0/v1 register contract (root Z, row Y, final node X).
 * Traversal stops only when the live best fraction is
 * <=the current cell's fraction, or the actual fraction1 marker is reached.
 * Endpoints are world units; contact fields remain in original1024-scaled
 * units. source_guest is the original terrain root. Info is a diagnostic of
 * CPU work, not game state or physical VU flags.
 * Arithmetic is the explicitly approximate ordered host IEEE f32 domain of
 * the grid and triangle adapters, including their saturated zero-Q result.
 * Nonfinite/subnormal intermediates refuse the whole transaction.
 * Supports one readable immutable guest32 range entirely below0x80000000,
 * bounded grid/IEEE domains, nonzero QWC for nonzero leaves, well-formed packed
 * blocks/indices and quad_count<=primary_count. caller_flags.bit0 must be clear
 * (the actual flag bypasses terrain). Refusals preserve both outputs; a genuine
 * terrain miss preserves contact and supplies info. Outputs must be disjoint
 * from each other and source storage; either may overlap the input snapshot.
 * This is not the whole probe return: dynamic actors, original DMA/cache/VU
 * state, packet publication and final unit conversion remain unimplemented.
 * Unconsumed vertex/face preloads and unused odd decode slots are not exposed:
 * the API returns contact/work effects, rather than original scratch contents.
 */
int dreadzone_terrain_contact(const DreadzoneTerrainContactRequest *request,
                             DreadzoneCollisionTriangleContact *contact,
                             DreadzoneTerrainContactInfo *info);
/* Internal probe composition: retain incoming high64 unless the genuine
 * nonempty vertex decode reaches its final packed producer. A nonnull state
 * is disjoint from request, contact, info and storage. All outputs publish
 * only on success. This terrain stage omits the later dynamic producers.
 */
int dreadzone_terrain_contact_tracked(const DreadzoneTerrainContactRequest *,
    DreadzoneCollisionTriangleContact *,DreadzoneTerrainContactInfo *,
    DreadzoneProbeRegisters *);
#endif
