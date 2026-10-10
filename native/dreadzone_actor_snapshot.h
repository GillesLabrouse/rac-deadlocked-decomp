#ifndef DEADLOCKED_DREADZONE_ACTOR_SNAPSHOT_H
#define DEADLOCKED_DREADZONE_ACTOR_SNAPSHOT_H
#include "common.h"

typedef struct DreadzoneSnapshotMemory {
    const u8 *storage;
    u32 guest_base, bytes;
} DreadzoneSnapshotMemory;
typedef struct DreadzoneSnapshotContext {
    const u8 *record;
    u32 record_guest, record_bytes;
    const u8 *mode_bytes_00171DE0;
    u32 mode_count;
    s32 state_0021DDB4;
    const DreadzoneSnapshotMemory *views;
    u32 view_count;
} DreadzoneSnapshotContext;
enum DreadzoneSnapshotResult {
    DREADZONE_SNAPSHOT_COMPLETED = 0,
    DREADZONE_SNAPSHOT_INVALID_STORAGE = -1,
    DREADZONE_SNAPSHOT_NEEDS_CLIENT = -2,
    DREADZONE_SNAPSHOT_NEEDS_RELATED_OBJECT = -3,
    DREADZONE_SNAPSHOT_NEEDS_VTABLE = -4,
    DREADZONE_SNAPSHOT_UNKNOWN_GETTER = -5,
    DREADZONE_SNAPSHOT_NEEDS_GETTER_OBJECT = -6
};
/* Complete, read-only 0068DE60 predicate. Result is the original raw mode byte,
 * not a boolean. Guest mappings are required only when retail dereferences them.
 * The output must be separate caller-owned storage. Errors leave it unchanged.
 * This predicate alone does not construct or publish an actor snapshot. */
int dreadzone_actor_0068DE60(const DreadzoneSnapshotContext *, s32 *result);
int dreadzone_actor_006A7D28(const DreadzoneSnapshotContext *, s32 *result);
s32 func_L01_00690E68(const u8 *record);
/* Complete0066A2A0 CPU snapshot, including genuine supported virtual getters.
 * Client storage must be separately owned, guest16-aligned, and match the
 * record's guest pointer2EFC. Other mapped regions must be disjoint from that
 * client, except an identical read-only client view used by the predicate.
 * The fixed-address mode array must also be disjoint from client storage.
 * Vtable/getter/predicate reads into the written client range150..2A4 are
 * rejected before mutation: retail performs those reads after earlier copies.
 * All dependencies are checked before mutation. This does not run an actor
 * constructor, network transport, model transformation, VU code or game frame.
 */
int dreadzone_actor_0066A2A0(const DreadzoneSnapshotContext *,
                          u8 *client, u32 client_guest, u32 client_bytes);
#endif
