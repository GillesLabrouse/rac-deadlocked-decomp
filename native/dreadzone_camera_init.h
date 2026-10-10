#ifndef DEADLOCKED_DREADZONE_CAMERA_INIT_H
#define DEADLOCKED_DREADZONE_CAMERA_INIT_H
#include "dreadzone_angle_pair.h"

enum {
    DREADZONE_CAMERA_APPROXIMATE = 1,
    DREADZONE_CAMERA_INVALID = -1,
    DREADZONE_CAMERA_UNSUPPORTED = -2,
    DREADZONE_CAMERA_UNMAPPED = -3,
    DREADZONE_CAMERA_OUTPUT_BYTES = 0x150
};
typedef struct DreadzoneCameraView {
    u32 guest_base;
    u32 bytes;
    u8 *storage;
} DreadzoneCameraView;
typedef struct DreadzoneCameraInit {
    const DreadzoneCameraView *views;
    u32 view_count;
    u32 output;
    u32 position;
    u32 actor;
    float angle12;
    float angle13;
} DreadzoneCameraInit;

/* Complete CPU-visible output of 005617A8 under the bounded angle-pair
 * approximation. No callee is captured or skipped. Two position reloads and
 * the late read at guest0021E754 see earlier output writes when aliased.
 * Views (1..8) are disjoint in both host and guest space; output/position
 * guest addresses are quad-aligned. Host backing may be unaligned.
 * Descriptors may not overlap the output backing. Unsupported arithmetic or
 * mappings leave all backing unchanged. APPROXIMATE excludes physical VU/EE
 * rounding, VU register effects, stack spills, timing and control registers.
 * Actor is a raw guest32 value stored at output+BC, never dereferenced here.
 */
int dreadzone_camera_init(const DreadzoneCameraInit *request);
#endif
