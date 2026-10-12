#ifndef DEADLOCKED_DREADZONE_WORLD_PROJECTION_H
#define DEADLOCKED_DREADZONE_WORLD_PROJECTION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DreadzoneWorldProjectionMemory {
    uint8_t *bytes;
    uint32_t guest_base;
    size_t length;
} DreadzoneWorldProjectionMemory;

/*
 * Complete bounded RAM effects of actual L01 00628210 and its 0055D2D0
 * zero helper. This component writes the three converted vector words and
 * the 64-byte matrix; it does not initialize the vector/scale producers.
 *
 * The descriptor pointer must designate sizeof(DreadzoneWorldProjectionMemory) readable host
 * bytes for the duration of the call. The function rejects numeric descriptor
 * wrap before copying it, but C cannot validate that an arbitrary host address
 * is mapped/readable. The descriptor bytes must be disjoint from the backing
 * RAM span. The backing must be one stable, exclusive allocation of at most
 * 32 MiB, wholly below guest address 0x80000000. Host backing may be unaligned.
 *
 * Supports only finite IEEE binary32 inputs, FE_TONEAREST, nonzero finite
 * reciprocal denominators, finite intermediates/results, and vector*1024
 * results that are already exact integers in signed 32-bit range. This does
 * not model EE FCSR exception/flag behavior, NaN/subnormal edge behavior, or
 * physical EE rounding details. Returns 0 on success; -1 on preflight/domain
 * failure without modifying RAM.
 */
int dreadzone_world_projection_00628210(const DreadzoneWorldProjectionMemory *ram);

#ifdef __cplusplus
}
#endif

#endif
