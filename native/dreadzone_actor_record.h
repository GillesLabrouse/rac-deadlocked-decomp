#ifndef DREADZONE_ACTOR_RECORD_H
#define DREADZONE_ACTOR_RECORD_H

#include <stddef.h>
#include <stdint.h>

typedef enum DreadzoneActorRecordStatus {
    DREADZONE_ACTOR_RECORD_OK = 1,
    DREADZONE_ACTOR_RECORD_INVALID = -1,
    DREADZONE_ACTOR_RECORD_UNSUPPORTED = -2
} DreadzoneActorRecordStatus;

typedef struct DreadzoneActorRecordRequest {
    const uint8_t *ram;
    uint32_t ram_base;
    uint32_t ram_size;
    uint32_t actor;
} DreadzoneActorRecordRequest;

typedef struct DreadzoneActorRecordInfo {
    DreadzoneActorRecordStatus status;
    uint32_t metadata_address;
    uint32_t method_address;
    int64_t value;
} DreadzoneActorRecordInfo;

/* Complete selected L01 667008 -> 5B5668 -> 666140 read-only effects and
 * low64 signed return. The caller supplies live guest RAM, including registry
 * and selector tables; this function neither creates nor initializes them.
 * Only an actual metadata+4 entry equal to 666140 is supported. A null actor
 * genuinely returns zero without reading those globals. Other methods refuse.
 * RAM is one caller-owned contiguous span, <=16 MiB, below 80000000, excluding
 * 70000000..70003FFF. All reached loads must be aligned and mapped.
 * Output must be disjoint from request/RAM. Request may alias RAM. Every
 * refusal preserves the output and RAM. No full GPR/frame or exact-code claim.
 */
DreadzoneActorRecordStatus dreadzone_actor_record_00667008(
    const DreadzoneActorRecordRequest *request,
    DreadzoneActorRecordInfo *result);

#endif
