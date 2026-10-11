#ifndef DEADLOCKED_DREADZONE_PLAYER_VOLUME_INTERNAL_H
#define DEADLOCKED_DREADZONE_PLAYER_VOLUME_INTERNAL_H
#include "dreadzone_player_volume.h"
/* Lower operation only for a captured, validated request and private RAM.
 * The enclosing composition owns atomic publication. Not a public entry. */
int dreadzone_player_volume_apply(const DreadzonePlayerVolume *);
#endif
