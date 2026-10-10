/* cflags: -mno-split-addresses */
/* Actual player vtable getter006A7D28..006A7D44,28 bytes. NONMATCHING. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_snapshot.h"
static u32 getter_word(const u8 *p) {
    return (u32)p[0] | ((u32)p[1]<<8) | ((u32)p[2]<<16) | ((u32)p[3]<<24);
}
int dreadzone_actor_006A7D28(const DreadzoneSnapshotContext *host, s32 *result) {
    u32 object, i;
    const u8 *field=0;
    if (!host || !result || !host->record || host->record_bytes<0x31F0 ||
        (host->record_guest&15) || host->record_guest>0xFFFFFFFFU-(host->record_bytes-1))
        return DREADZONE_SNAPSHOT_INVALID_STORAGE;
    object=getter_word(host->record+0x2EF4);
    if (!object) { *result=0; return DREADZONE_SNAPSHOT_COMPLETED; }
    if ((object&3) || object>0xFFFFFFFFU-0x2B3 || (host->view_count && !host->views))
        return DREADZONE_SNAPSHOT_NEEDS_GETTER_OBJECT;
    for (i=0;i<host->view_count;i++) {
        const DreadzoneSnapshotMemory *v=&host->views[i];
        u32 address=object+0x2B0;
        if (!v->storage || !v->bytes || v->guest_base>0xFFFFFFFFU-(v->bytes-1)) continue;
        if (address>=v->guest_base && v->bytes>=4 && address-v->guest_base<=v->bytes-4) {
            if (field) return DREADZONE_SNAPSHOT_NEEDS_GETTER_OBJECT;
            field=v->storage+(address-v->guest_base);
        }
    }
    if (!field) return DREADZONE_SNAPSHOT_NEEDS_GETTER_OBJECT;
    *result=(s32)getter_word(field);
    return DREADZONE_SNAPSHOT_COMPLETED;
}
#else
s32 func_L01_006A7D28(const u8 *record) {
    const u8 *object=*(const u8 **)(record+0x2EF4);
    if (!object) return 0;
    return *(const s32 *)(object+0x2B0);
}
#endif
