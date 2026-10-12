#ifndef DEADLOCKED_DREADZONE_CLIENT_HISTORY_COPY_H
#define DEADLOCKED_DREADZONE_CLIENT_HISTORY_COPY_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct DreadzoneClientHistoryCopyMemory {
    uint32_t guest_base;
    uint8_t *bytes;
    size_t length;
} DreadzoneClientHistoryCopyMemory;

/* Complete RAM effects of actual L01 00535200 and both real 0055D470 copies.
 * For index0..3, client row230280+index470h supplies its pointer at+50h.
 * That pointer must designate one of its48 history-table entries at
 * 231AD0+index2100h, strideB0h. CopyB0h bytes into231810+indexB0h;
 * then copy280h bytes from23C010+index780h into23C510+index780h.
 * Finally store the second destination at history-copy destination+70h.
 * The first copy readsC0h bytes, including its actual16-byte lookahead;
 * the second reads/stores exactly280h bytes, including both final quads.
 *
 * The caller supplies one truthful, exclusive readable/writable RAM span
 * <=16MiB below2GiB. Host backing may be unaligned; the descriptor must be
 * a valid readable object disjoint from that backing. All nonwrapping
 * pointers must designate actual allocations with stable lifetimes.
 * The two copy domains must be disjoint, including the first read lookahead.
 * These bounds describe native support, rather than new retail predicates.
 * Success0 publishes only those RAM effects. Invalid/unsupported-1 refuses
 * before mutation; the descriptor remains unchanged. No lookup invocation,
 * table initialization, client+50 producer, machine banks or frame promised.
 */
int dreadzone_client_history_copy_00535200(const DreadzoneClientHistoryCopyMemory *,uint32_t index);

#ifdef __cplusplus
}
#endif
#endif
