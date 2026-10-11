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
 * This legacy request lacks the explicit inherited VU input required by the
 * decoder. Use the fixed entry below for the verified initial-frame domain.
 * The earlier identity-only
 * implementation and its completion guarantee have been withdrawn.
 * Null actor/model returns NO_MODEL. Invalid host descriptors or unmapped
 * actor backing retain distinct statuses. Every status leaves all caller
 * memory unchanged; views and scratch must have disjoint backing.
 * APPROXIMATE is retained for API compatibility but is never returned.
 */
int dreadzone_model_transform_static(const DreadzoneModelTransform *request);
typedef struct DreadzoneFixedModelTransform {
    DreadzoneModelTransform model;
    const u8 *vu_data;
    u32 vu_data_bytes;
} DreadzoneFixedModelTransform;
/* Normal initial-frame 0058A648 memory effects through actual 0058A440,
 * skeleton transfer/00589368, fixed transfer/005894F0, 00589950 and final
 * matrix transfer. Requires positive-zero actor44, no actor55 bit4, no actor50
 * or5C nodes, matching1..127 joint counts and a fixed-only animation stream.
 * Inherited VU data4096 bytes are explicit input; no zero-state assumption.
 * Only APPROXIMATE host binary32 memory effects are supplied. Hardware DMA
 * registers/timing, VU state and original stack/register ABI are excluded.
 * Actor, definition and animation addresses must be4-byte aligned; skeleton,
 * stream and matrix addresses must be16-byte aligned. Complete actor100 and
 * referenced assets must be mapped; matrices may not
 * overlap actor or referenced assets. Guest and host views are disjoint, and
 * neither scratch nor VU backing may overlap any view or each other.
 * Request/view descriptors are captured before writes, including borrowed
 * descriptors in scratch or output backing. All preflight/helper refusals
 * leave every caller byte unchanged. Success publishes matrices, full scratch
 * and actor55 dirty-bit clear together. No class/actor construction occurs.
 */
int dreadzone_model_transform_fixed(const DreadzoneFixedModelTransform *request);

typedef DreadzoneFixedModelTransform DreadzoneInitialModelTransform;
/* Same bounded frame-zero contract, with actual current/next variable transfers
 * before the normal decoder. Requires1..128 fixed and0..128 variable quads and
 * nonzero animation frame count. Current frame is0; next is1, wrapping to0 when
 * frame count is1. The complete reached stream span must fit one mapped view
 * and be disjoint from actor and output. Skeleton initialization precedes all
 * animation transfers. Decoder/parent/numeric guards still apply; variable
 * length alone is not a success guarantee. Atomicity and explicit VU inputs
 * match the fixed entry. General matrices writes parent127 identity atSPR3FC0,
 * even when a128-quad next transfer filled that location. This is initial
 * pose construction, without repeated animation updates or player creation. */
int dreadzone_model_transform_initial(const DreadzoneInitialModelTransform *request);
#endif
