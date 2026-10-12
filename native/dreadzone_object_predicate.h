#ifndef DREADZONE_OBJECT_PREDICATE_00596E48_H
#define DREADZONE_OBJECT_PREDICATE_00596E48_H
#include <stdint.h>

typedef struct DreadzoneObjectPredicateRequest {
    const uint8_t *storage;
    uint32_t guest_base, storage_bytes, packet_guest;
} DreadzoneObjectPredicateRequest;

typedef struct DreadzoneObjectPredicateInfo {
    uint32_t original_return;
} DreadzoneObjectPredicateInfo;

enum {
    DREADZONE_OBJECT_PREDICATE_INVALID = -1,
    DREADZONE_OBJECT_PREDICATE_UNSUPPORTED = -2,
    DREADZONE_OBJECT_PREDICATE_COMPLETE = 1
};

/* Read-only typed guest-RAM model of actual L01 00596E48/00596E38.
 * The packet, globals, and reached indirect object bytes must share one
 * mapped nonwrapping range below 2 GiB. Guest RAM is never modified. Info is
 * published only after all reads selected by the original lazy branches are
 * mapped. Info must be disjoint from request and backing; request may alias
 * backing because it is captured before execution. This predicate does not
 * perform collision geometry or establish packet/object provenance. */
int dreadzone_object_predicate_00596E48(
    const DreadzoneObjectPredicateRequest *request,
    DreadzoneObjectPredicateInfo *info);
#endif
