/* asflags: --float-immediates */
#include "common.h"

/* Complete L01 0066A6F8..0066A7B0. Applies the genuine rotation oscillator,
 * then conditionally overrides actor.F4 from record.520. The second read
 * of actor and all record reloads follow the observed call boundary.
 * Nonmatching: both extents are 184 bytes; instruction allocation differs. */
#ifndef DEADLOCKED_NATIVE
extern volatile f32 D_0021DDF4;
void func_L01_0059BEF0(u8 *,f32 *,f32 *,f32,f32,f32);
typedef union RotationOscillatorBits { u32 bits;f32 value; } RotationOscillatorBits;

void func_L01_0066A6F8(u8 *record)
{
    s32 state=*(volatile s32 *)(record+0x25CC);
    if(state==0x7E || (state==0x99 && *(volatile s32 *)(record+0x25D8)==0x7E)) {
        RotationOscillatorBits rateX,rateY,amplitude;
        f32 dt,stepX,stepY,scale;
        rateX.bits=0x3FC90FDBU;rateY.bits=0x40060A92U;
        amplitude.bits=0x3E32B8C2U;
        dt=D_0021DDF4;
        stepX=dt*rateX.value;
        scale=*(volatile f32 *)(record+0x25C4);
        stepY=dt*rateY.value;
        stepX=stepX*scale;
        stepY=stepY*scale;
        func_L01_0059BEF0((u8 *)*(volatile u32 *)(record+0x2EF0),
            (f32 *)(record+0x1C60),(f32 *)(record+0x1C64),
            amplitude.value,stepX,stepY);
    }
    if(*(volatile s16 *)(record+0x52C)!=0 && *(volatile u32 *)(record+0x4D0)==0) {
        f32 rotation=*(volatile f32 *)(record+0x520);
        u8 *actor=(u8 *)*(volatile u32 *)(record+0x2EF0);
        *(volatile f32 *)(actor+0xF4)=rotation;
    }
}
#endif
