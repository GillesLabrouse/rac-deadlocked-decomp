#ifndef DEADLOCKED_DREADZONE_PLAYER_HISTORY_H
#define DEADLOCKED_DREADZONE_PLAYER_HISTORY_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct DreadzonePlayerHistoryRequest {
    uint8_t *storage;
    uint32_t guest_base,storage_bytes,record_guest;
} DreadzonePlayerHistoryRequest;
typedef struct DreadzonePlayerHistoryInfo {
    uint32_t angle_slot,position_slot,angle_written,position_written;
} DreadzonePlayerHistoryInfo;
enum { DREADZONE_PLAYER_HISTORY_COMPLETE=1,DREADZONE_PLAYER_HISTORY_INVALID=-1,
       DREADZONE_PLAYER_HISTORY_UNSUPPORTED=-2,DREADZONE_PLAYER_HISTORY_MEMORY=-4 };
/* Selected integer RAM effects of actual L01 68EFE8, or the ordered
 * 67C1B0..67C208 inline angle history followed by its real 68EFE8 call.
 * Supplied record backing is at least2C80 bytes with guest16 alignment.
 * Cursors are0..31 and counts0..32. These are bounded adapter preconditions,
 * not extra original game branches or recovered API/storage identities.
 * The original caller must supply these fields; this does not initialize them.
 *
 * One stable exclusive writable guest image <=16MiB below2GiB, excluding
 * scratch70000000..70003FFF. Host byte alignment is arbitrary. Capture request
 * before execution; it may alias backing. Info is disjoint from both.
 * Preserve actual source-before-store and counter-reload/store order, raw
 * angle bits and all position quad words (including NaNs/signed zeros).
 * Commit image/info only after complete supported execution; refusals are
 * atomic. There is no floating arithmetic or host rounding requirement.
 * Status/counts are adapter telemetry, not original path-dependent registers.
 * No physical FCSR, full GPR/FPR/VU/stack state, basis, locomotion, startup
 * ownership or complete playable frame is supplied by these interfaces.
 */
int dreadzone_player_position_history_0068EFE8(
    const DreadzonePlayerHistoryRequest *,DreadzonePlayerHistoryInfo *);
int dreadzone_player_history_before_basis(
    const DreadzonePlayerHistoryRequest *,DreadzonePlayerHistoryInfo *);
#ifdef __cplusplus
}
#endif
#endif
