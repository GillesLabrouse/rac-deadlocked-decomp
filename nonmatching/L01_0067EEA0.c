/* cflags: -ffast-math -mno-split-addresses */
/* Complete original0067EEA0..0067EEFC (92 bytes), NONMATCHING.
 * The second argument is a raw128-bit register value, not a pointer. */
#include "common.h"
typedef unsigned int PositionHeadingQuad __attribute__((mode(TI)));
typedef struct PositionHeadingVector { f32 x,y,z,w; }
    __attribute__((aligned(16))) PositionHeadingVector;
void func_L01_0068A3F8(u8 *,PositionHeadingVector *,const PositionHeadingVector *,f32);
f32 func_L01_0067EEA0(u8 *record,PositionHeadingQuad vector) {
    PositionHeadingVector offset={0.f,0.f,0.f,0.f};
    PositionHeadingVector scratch[2];
    u32 value;
    func_L01_0068A3F8(record,&offset,&offset,1.f);
    __asm__ volatile("qmtc2.ni %1,$vf3\n\tlqc2 $vf2,0(%2)\n\t"
        "vmaxw.xyzw $vf1,$vf0,$vf0w\n\tvmul.xyzw $vf3,$vf3,$vf2\n\t"
        "vadday.x ACC,$vf3,$vf3y\n\tvmaddz.x $vf1,$vf1,$vf3z\n\t"
        "qmfc2.ni %0,$vf1\n\tsqc2 $vf1,0(%3)"
        : "=r"(value) : "r"(vector),"r"(&offset),"r"(scratch) : "memory");
    { union {u32 bits;f32 f;}result;result.bits=value;return result.f; }
}
