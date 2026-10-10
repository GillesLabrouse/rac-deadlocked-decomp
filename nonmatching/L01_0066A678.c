/* asflags: --float-immediates */
#include "common.h"

/* Complete L01 0066A678..0066A6F8. The caller has already copied the
 * position to actor+10. States 7E, and99 with predecessor7E, run the actual
 * three-float oscillator; its two writable phase/history fields remain
 * record+1C58/1C5C. No native substitute for its VU/math callees is provided.
 * Nonmatching: the compiled function is 124 bytes versus 128 retail bytes. */
#ifndef DEADLOCKED_NATIVE
extern volatile f32 D_0021DDF4;
void func_L01_0059BDE8(u8 *,f32 *,f32 *,f32,f32,f32);
typedef union PositionOscillatorBits { u32 bits;f32 value; } PositionOscillatorBits;

void func_L01_0066A678(u8 *record)
{
    s32 state=*(volatile s32 *)(record+0x25CC);
    if(state==0x7E || (state==0x99 && *(volatile s32 *)(record+0x25D8)==0x7E)) {
        PositionOscillatorBits rate;
        f32 step;
        rate.bits=0x40DF66F3U;
        step=D_0021DDF4*rate.value;
        step=step**(volatile f32 *)(record+0x25C4);
        func_L01_0059BDE8((u8 *)*(volatile u32 *)(record+0x2EF0),
            (f32 *)(record+0x1C58),(f32 *)(record+0x1C5C),0.3f,step,-1.0f);
    }
}
#endif
