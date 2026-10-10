#ifndef DEADLOCKED_DREADZONE_GEOMETRY_H
#define DEADLOCKED_DREADZONE_GEOMETRY_H
#include "common.h"

typedef struct DreadzoneGeometryView {
    u32 guest_base, bytes;
    u8 *storage;
} DreadzoneGeometryView;
typedef struct DreadzoneGeometryContext {
    const DreadzoneGeometryView *views;
    u32 view_count;
} DreadzoneGeometryContext;
/* The bounded host interface accepts at most 32 disjoint guest/host views. */
typedef struct DreadzoneGeometryPlan DreadzoneGeometryPlan;
typedef int (*DreadzoneGeometryRead)(void *, u32 address, u32 bytes, u64 *value);

enum {
    DREADZONE_GEOMETRY_OK = 0,
    DREADZONE_GEOMETRY_UNMAPPED = -1,
    DREADZONE_GEOMETRY_INVALID = -2,
    DREADZONE_GEOMETRY_ALLOCATION_FAILED = -3,
    DREADZONE_GEOMETRY_CONFIG = 0x001DFCF0,
    DREADZONE_GEOMETRY_TABLE = 0x002699A0,
    DREADZONE_GEOMETRY_FALLBACK = 0x00269B20,
    DREADZONE_GEOMETRY_FALLBACK_MINUS_THREE = 0x00269B38
};

/* Actual 00576AA8 and both packing callees. Headers are 16 bytes; output groups
 * are 64 bytes. Only the lower eight bytes of each output quad are overwritten.
 * Serialized pointers remain u32. Remap and texture inputs may have guest
 * prefixes for negative indices, as the retail callees read them before testing
 * the signed index. This constructs real packets; it does not submit to a GPU.
 * Guest load/store alignment is enforced; host buffers may be unaligned.
 * A nonpositive header count is a no-op, without reading any guest pointers.
 */
int dreadzone_geometry_patch(const DreadzoneGeometryContext *, u32 headers,
                            u32 textures, u32 remap, s32 header_count);

/* Prepare observes prior planned writes, including aliases within an asset.
 * No guest write occurs until apply has validated every target. A plan contains
 * guest addresses, not borrowed host pointers, and can be applied to equivalent
 * backing after an enclosing relocation transaction is prepared. Keep the
 * underlying inputs stable between preparation and application. Apply succeeds
 * only once; destroy also releases an unapplied plan. Empty no-op plans are NULL.
 */
int dreadzone_geometry_prepare(const DreadzoneGeometryContext *, u32 headers,
                              u32 textures, u32 remap, s32 header_count,
                              DreadzoneGeometryPlan **);
/* Optional transaction reader supplies preceding class relocation writes.
 * Geometry's own planned writes overlay its results. Every read and write is
 * still validated against context views. The reader returns one on success.
 */
int dreadzone_geometry_prepare_with_reader(const DreadzoneGeometryContext *,
                              DreadzoneGeometryRead, void *reader_context,
                              u32 headers, u32 textures, u32 remap, s32 header_count,
                              DreadzoneGeometryPlan **);
u64 dreadzone_geometry_write_count(const DreadzoneGeometryPlan *);
int dreadzone_geometry_write_at(const DreadzoneGeometryPlan *, u64 index,
                               u32 *address, u32 *bytes, u64 *value);
int dreadzone_geometry_apply(DreadzoneGeometryPlan *, const DreadzoneGeometryContext *);
void dreadzone_geometry_destroy(DreadzoneGeometryPlan *);

/* Shared implementation interfaces for the two genuine packing algorithms. */
int dreadzone_geometry_read(DreadzoneGeometryPlan *, u32 address, u32 bytes, u64 *value);
int dreadzone_geometry_write(DreadzoneGeometryPlan *, u32 address, u32 bytes, u64 value);
int dreadzone_geometry_emit_textured(DreadzoneGeometryPlan *, u32 output, u32 texture,
                                    s32 word0, s32 word4, s32 word10, s32 word14, s32 index);
int dreadzone_geometry_emit_table(DreadzoneGeometryPlan *, u32 output,
                                 s32 word0, s32 word4, s32 word10, s32 word14, s32 index);
#endif
