#ifndef DEADLOCKED_DREADZONE_WALL_HELPERS_H
#define DEADLOCKED_DREADZONE_WALL_HELPERS_H
#include "common.h"

enum { DREADZONE_WALL_HELPER_INVALID=-1,DREADZONE_WALL_HELPER_UNSUPPORTED=-2 };

/* Success1 publishes the actual surface value, including absent=-1.
 * Reads signed packet.word1C before masking. Output may alias packet.
 */
int dreadzone_wall_surface_00542FF8(const u8 *packet,u32 bytes,s32 *out);

/* Complete CPU effect of 0068AFF8 and genuine frozen0055D708. Mode byte2659
 * selects raw mode0, full matrix60..90 for modes1/2 (real input W included),
 * or positive zero for>=3 without touching normal/matrix. Raw terrain normals
 * are unnormalized. Finite normal/zero ordered host-f32 and individually
 * rounded ACC/Q is an explicit approximation, not physical VU validation.
 * Every executed floating intermediate, including unused W square, must be
 * normal/zero and finite; unsupported intermediates refuse atomically.
 * Actual immutable L01 polynomial tables must remain the frozen library's
 * constants; callers with modified guest tables must refuse before this API.
 * Normal/record are captured before output; output may alias either, including
 * unaligned byte backing. No other caller memory is written. Success1 or a
 * negative refusal, preserving output bytes. Exclusive access is required.
 */
int dreadzone_wall_inclination_0068AFF8(const u8 *record,u32 bytes,
    const u8 *normal,u32 normal_bytes,f32 *out);
#endif
