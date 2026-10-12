#ifndef DREADZONE_CLIENT_HISTORY_LOOKUP_H
#define DREADZONE_CLIENT_HISTORY_LOOKUP_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Selected-address effect of actual L01 005353C8..00535408.
 * For index 0..3, search 48 records at 00231AD0+index*2100h, stride B0h,
 * comparing the signed halfword at +86h to the full signed-int32 marker.
 * Return the first matching guest record address, or zero for no match.
 * The caller retains exclusive access to truthful readable storage of at most
 * 16 MiB below 2 GiB, containing the complete selected 2100h-byte table.
 * Host storage may be unaligned. selected must be a valid writable uint32
 * object disjoint from the entire host storage interval; both lifetimes and
 * allocation provenance are the caller's responsibility.
 * Success 1 publishes selected. Invalid mapping/output -1 or index -2 leaves
 * both storage and selected unchanged. No RAM writes, machine-bank effects,
 * table initialization, client publication or runtime startup are promised. */
int dreadzone_client_history_lookup_005353C8(const uint8_t *storage,
    uint32_t guest_base, uint32_t storage_bytes, uint32_t index,
    int32_t marker, uint32_t *selected);

#ifdef __cplusplus
}
#endif
#endif
