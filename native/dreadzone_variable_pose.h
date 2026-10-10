#ifndef DEADLOCKED_VARIABLE_POSE_H
#define DEADLOCKED_VARIABLE_POSE_H
#include "common.h"
typedef struct DreadzoneVariablePose {
    u8 *scratch;
    u32 bytes;
    f32 fraction;
    const u8 *vu_data;
    u32 vu_data_bytes;
} DreadzoneVariablePose;
enum { DREADZONE_VARIABLE_POSE_APPROXIMATE=1,
       DREADZONE_VARIABLE_POSE_INVALID=-1,
       DREADZONE_VARIABLE_POSE_UNSUPPORTED=-2 };
/* CPU-visible normal decoder5894F0 + VU160/250 memory effects. The caller
 * provides initialized joint records and genuine fixed/current/next stream
 * banks at2000/3000/3800, plus explicit inherited VU data4096 bytes. Low VU
 * addresses can supply metadata lanes; their contents are not assumed zero.
 * High-address gathers must have been initialized by this decoder's own cache
 * writes. VU data/register side effects are not published by this memory adapter.
 * This entry does not run DMA, waits, blend330/620,
 * subsequent matrix generation738/858, a frame update or renderer.
 * All16384 bytes are retained except actual decoder writes, atomically.
 * Host binary32 arithmetic is explicitly approximate relative to PS2 VU.
 * Invalid/truncated streams, other computed-jump opcodes, zero rotation count,
 * nonfinite/denormal fraction and fractions outside[0,1] are refused. */
int dreadzone_variable_pose_decode(const DreadzoneVariablePose *request);
#endif
