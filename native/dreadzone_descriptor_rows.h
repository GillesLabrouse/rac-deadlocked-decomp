#ifndef DEADLOCKED_DREADZONE_DESCRIPTOR_ROWS_H
#define DEADLOCKED_DREADZONE_DESCRIPTOR_ROWS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DreadzoneDescriptorRowsMemory {
    uint8_t *bytes;
    uint32_t guest_base;
    size_t length;
} DreadzoneDescriptorRowsMemory;

/*
 * RAM-only component corresponding to actual L01 0x0057B498 and its reached
 * zero helper 0x0055D2A8. The descriptor pointer must designate readable
 * sizeof(DreadzoneDescriptorRowsMemory) host bytes and be disjoint from the backing allocation;
 * backing must be stable/exclusive, at most 32 MiB, and guest-addressed wholly
 * below 2 GiB. The function checks descriptor address wrap before copying but
 * cannot probe arbitrary host-pointer readability.
 *
 * source_guest points to a 16-byte header whose first word is a signed count;
 * rows begin at source_guest+0x10 and are 32 bytes each. cursor_word_guest
 * points to the guest cursor word. The API rejects negative/unsupported counts,
 * overflow, unaligned source/cursor fields, and overlapping participating
 * guest spans. It stages all record bytes before mutation and returns -1
 * without RAM changes on refusal, 0 on success. This bounded component does
 * not model surrounding startup/runtime context or promise a native frame.
 */
int dreadzone_descriptor_rows_0057B498(const DreadzoneDescriptorRowsMemory *ram,
                                uint32_t source_guest,
                                uint32_t cursor_word_guest);

#ifdef __cplusplus
}
#endif

#endif
