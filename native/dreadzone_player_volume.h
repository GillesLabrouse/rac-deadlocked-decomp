#ifndef DEADLOCKED_DREADZONE_PLAYER_VOLUME_H
#define DEADLOCKED_DREADZONE_PLAYER_VOLUME_H
#include <stdint.h>
typedef struct DreadzonePlayerVolume {
    uint8_t *storage;
    uint32_t guest_base,bytes,actor,index;
    float center[4],radius,height;
} DreadzonePlayerVolume;
/* Complete supported587188 memory effects. Entire actor mapped,
 * aligned volume header/row, index0..127. Original signed count can skip.
 * Canonical VF0 transposition, all consumed/intermediate values finite normal
 * or zero, nonzero scale. Ordered IEEE binary32 excludes original external stack/register ABI,
 * VU register effects and physical EE/VU state.
 * Return1 committed,0 original count skip,-1 invalid,-2 unmapped,-3 unsupported,
 * -4 allocation. Every failure is atomic; descriptor may borrow caller RAM. */
int dreadzone_player_volume_update(const DreadzonePlayerVolume *);
#endif
