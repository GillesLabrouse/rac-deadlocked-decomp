#ifndef DEADLOCKED_DREADZONE_PAYLOAD_RELOCATION_H
#define DEADLOCKED_DREADZONE_PAYLOAD_RELOCATION_H
#include <stdint.h>
typedef struct DreadzonePayloadRelocation {
    uint8_t *storage;
    uint32_t guest_base,bytes,descriptors,payload_base,cursor_cell;
    uint32_t actor_begin,actor_end,actor_limit;
} DreadzonePayloadRelocation;
/* Complete supported0057BDC8 memory path with real0055D470 copy.
 * Global bounds222D14/222D2C are read before nonzero supplied overrides.
 * <=65536 actor slots/explicit limit, aligned actors, index<=65535;
 * copied lengths16..1MiB divisible16, total copy bytes<=RAM size. Direct copy
 * needs aligned destination/source and the actual rounded32 source footprint.
 * Wrapped descriptor/source address arithmetic is retained; copy ranges cannot
 * wrap. Actor begin/end and final rounded cursor must remain below2GiB;
 * source plus rounded footprint must be strictly below2GiB. These are API
 * limits; memory comparisons do not establish arbitrary hardware trapping. Forward paired loads and
 * stores preserve aliases, including mutable cursor/descriptor/actor data.
 * No external stack/register ABI, runtime mission selection/player claim.
 * Return1,-1invalid,-2unmapped,-3unsupported,-4allocation; atomic RAM. */
int dreadzone_payload_relocate(const DreadzonePayloadRelocation *);
#endif
