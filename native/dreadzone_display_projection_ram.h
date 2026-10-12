#ifndef DREADZONE_DISPLAY_PROJECTION_RAM_H
#define DREADZONE_DISPLAY_PROJECTION_RAM_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* RAM effects of actual L01 005469A0..00546B38 and helper 00546918..005469A0.
 * Reads signed descriptor width/height at 00206B90/92; supports 1..32767.
 * Writes the staged 001D3F40..001D3F90 fields and selected 0023FE40..002400AC
 * fields, preserving untouched bytes and raw screen-vector Z/W bits.
 * The caller exclusively owns a truthful writable mapping of <=16 MiB, below
 * 2 GiB, containing both complete output areas and descriptor inputs.
 * Host storage may be unaligned. Round-to-nearest host binary32 is required.
 * Success 1 publishes both RAM areas; mapping failure -1 or unsupported
 * dimensions/rounding -2 changes nothing. These three arguments are by value.
 * No EE/VU/register/stack/FP-status or physical PS2 arithmetic is modelled.
 * Explicit descriptor values are inputs, not proof of live graphics startup;
 * this entry does not initialize the descriptor, player client or a frame. */
int dreadzone_display_projection_ram_005469A0(uint8_t *storage,uint32_t guest_base,uint32_t storage_bytes);

#ifdef __cplusplus
}
#endif
#endif
