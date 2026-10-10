#include "common.h"

/* Actual L01 00575180..005752B0, 304 instruction bytes. This preserves the
 * complete call order and the unchecked hardware callees. The draft compiles
 * to the same 304-byte size but differs in argument scheduling around the
 * second packet builder; it is not an exact linked match. The input format
 * is ignored. The final busy-wait leaves v0 == 0 when it returns; intermediate
 * upload/readback status values are not tested by this function.
 *
 * No native definition is provided here: a separate bounded CPU preparation
 * API explicitly distinguishes its memory effect from this hardware routine.
 */
#ifndef DEADLOCKED_NATIVE
s32 func_00138E08(u8 *, s32, s32, s32, s32, s32, s32, s32);
s32 func_00138FF0(u8 *, s32, s32, s32, s32, s32, s32, s32);
s32 func_00126E20(s32);
s32 func_00139130(u8 *, u8 *);
s32 func_001392B0(u8 *, u8 *);
s32 func_00131A08(s32, s32);

s32 func_L01_00575180(u8 *pixels, s32 unused_format, s32 width, s32 height) {
    u8 packets[0xD0] __attribute__((aligned(16)));
    s32 base256, upload_width64, readback_width64;
    s32 half_width, half_height;
    base256 = (s16)((0x00400000U - (u32)width * (u32)height) >> 8);
    upload_width64 = width >> 6;
    if (upload_width64 <= 0) upload_width64 = 1;
    func_00138E08(packets, base256, (s16)upload_width64, 0x13, 0, 0,
        (s16)width, (s16)height);
    func_00126E20(0);
    func_00139130(packets, pixels);
    func_00131A08(0, 0);
    readback_width64 = width >> 7;
    if (readback_width64 <= 0) readback_width64 = 1;
    half_width = (s32)((u32)width << 15) >> 16;
    half_height = (s32)((u32)height << 15) >> 16;
    func_00138FF0(packets + 0x60, base256, (s16)readback_width64, 0, 0, 0,
        half_width, half_height);
    func_00126E20(0);
    func_001392B0(packets + 0x60, pixels);
    return func_00131A08(0, 0);
}
#endif
