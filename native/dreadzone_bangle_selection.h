#ifndef DEADLOCKED_DREADZONE_BANGLE_SELECTION_H
#define DEADLOCKED_DREADZONE_BANGLE_SELECTION_H
#include "common.h"

typedef struct DreadzoneBangleRange {
    u32 first_packet, packet_count;
    s32 bangle; /* -1 primary; otherwise the real actor74 bit index. */
} DreadzoneBangleRange;
typedef struct DreadzoneBanglePlan {
    DreadzoneBangleRange ranges[16];
    u32 range_count, packet_count;
} DreadzoneBanglePlan;
typedef struct DreadzoneBangleRequest {
    const u8 *raw_class;
    u32 class_bytes;
    u16 actor_mask;
    u8 low_detail, reserved;
} DreadzoneBangleRequest;
enum DreadzoneBangleResult {
    DREADZONE_BANGLE_CANDIDATES = 1,
    DREADZONE_BANGLE_INVALID = -1,
    DREADZONE_BANGLE_UNSUPPORTED = -2
};
/* CPU packet-range candidates from real 0058EADC..0058EB48 and
 * 0058F4C8..0058F52C. Input is the original unrelocated class container,
 * actual actor.half74 and an explicit LOD choice. No initial mask is invented.
 * High/low material passes with class bytes6/A both zero are supported.
 * The sign-bit mask domain is refused: retail LH/PLZCW then addresses outside
 * the ordinary15-bit descriptor table. Low begin0 falls back to high.
 * Missing primary low packets selects high for every subsequent bangle too.
 * Candidates follow primary, then descending set bits. Empty ranges vanish.
 * Output is atomic, disjoint from request/container, bounded to16 ranges.
 * This does not execute LOD-distance, render-queue budget rejection, texture
 * submission, actor construction, animation, clipping, DMA or a game frame.
 * Caller supplies readable stable allocations and exclusive output access.
 */
int dreadzone_bangle_select(const DreadzoneBangleRequest *,DreadzoneBanglePlan *);
#endif
