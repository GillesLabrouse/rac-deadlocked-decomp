/* Complete 24-byte original VU C78 scalar entry; NONMATCHING draft. */
#include "common.h"
f32 func_L01_0055D640(f32 angle) {
    union {u32 word; f32 value;} input,output;
    input.value=angle;
    __asm__ volatile("qmtc2.ni %1,$vf15\n\tvcallms 0xC78\n\tqmfc2.i %0,$vf15"
        :"=r"(output.word):"r"(input.word):"memory");
    return output.value;
}
