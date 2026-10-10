/* cflags: -mno-split-addresses */
/* Complete L01 read-only predicate0068DE60..0068DF1C (188 bytes).
 * The return preserves the global byte, including values other than zero/one.
 * NONMATCHING: compiled code is compared independently with the retail span. */
#include "common.h"
typedef char SnapshotWordWidth[sizeof(u32) == 4 ? 1 : -1];
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_snapshot.h"
static u32 snapshot_word(const u8 *p) {
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}
static const u8 *snapshot_map(const DreadzoneSnapshotContext *host, u32 address, u32 bytes) {
    const u8 *found = 0;
    u32 i;
    if (!address || address > 0xFFFFFFFFU - (bytes - 1) ||
        (host->view_count && !host->views)) return 0;
    for (i = 0; i < host->view_count; i++) {
        const DreadzoneSnapshotMemory *v = &host->views[i];
        if (!v->storage || !v->bytes || v->guest_base > 0xFFFFFFFFU - (v->bytes - 1)) continue;
        if (address >= v->guest_base && bytes <= v->bytes && address - v->guest_base <= v->bytes - bytes) {
            if (found) return 0;
            found = v->storage + (address - v->guest_base);
        }
    }
    return found;
}
#define WORD(o) snapshot_word(record + (o))
#define BYTE(o) record[o]
#define MODE(i) host->mode_bytes_00171DE0[i]
#define STATE host->state_0021DDB4
int dreadzone_actor_0068DE60(const DreadzoneSnapshotContext *host, s32 *result) {
    const u8 *record, *mapped;
    u32 client, related;
    if (!host || !result || !host->record || host->record_bytes < 0x31F0 ||
        (host->record_guest & 15) || host->record_guest > 0xFFFFFFFFU - (host->record_bytes - 1))
        return DREADZONE_SNAPSHOT_INVALID_STORAGE;
    record = host->record;
    if (!host->mode_bytes_00171DE0 || !host->mode_count ||
        host->mode_count - 1 > 0xFFFFFFFFU - 0x00171DE0 || WORD(0x2F10) >= host->mode_count)
        return DREADZONE_SNAPSHOT_INVALID_STORAGE;
#else
extern u8 D_00171DE0[];
extern s32 D_0021DDB4;
#define WORD(o) (*(u32 *)(record + (o)))
#define BYTE(o) record[o]
#define MODE(i) D_00171DE0[i]
#define STATE D_0021DDB4
s32 func_L01_0068DE60(u8 *record) {
    u8 *client, *related;
#endif
    u32 action;
    s32 value = 0;
    if (!MODE(WORD(0x2F10))) goto completed;
    if (WORD(0x3F4) || STATE == 2 || STATE == 6) goto completed;
    if (BYTE(0x266D)) {
#ifdef DEADLOCKED_NATIVE
        client = WORD(0x2EFC);
        if ((client & 3) || client > 0xFFFFFFFFU - 0x53)
            return DREADZONE_SNAPSHOT_NEEDS_CLIENT;
        mapped = snapshot_map(host, client + 0x50, 4);
        if (!mapped) return DREADZONE_SNAPSHOT_NEEDS_CLIENT;
        related = snapshot_word(mapped);
        if (related) {
            if ((related & 1) || related > 0xFFFFFFFFU - 0x87)
                return DREADZONE_SNAPSHOT_NEEDS_RELATED_OBJECT;
            mapped = snapshot_map(host, related + 0x86, 2);
            if (!mapped) return DREADZONE_SNAPSHOT_NEEDS_RELATED_OBJECT;
            if (((u16)mapped[0] | ((u16)mapped[1] << 8)) == 5) goto completed;
        }
#else
        client = (u8 *)WORD(0x2EFC);
        related = *(u8 **)(client + 0x50);
        if (related && *(s16 *)(related + 0x86) == 5) goto completed;
#endif
    }
    action = WORD(0x25CC);
    if (action == 0x71 || action == 0x31 || action == 0x1C || action == 0x8E || action == 0x8F)
        goto completed;
    value = MODE(WORD(0x2F10));
completed:
#ifdef DEADLOCKED_NATIVE
    *result = value;
    return DREADZONE_SNAPSHOT_COMPLETED;
#else
    return value;
#endif
}
