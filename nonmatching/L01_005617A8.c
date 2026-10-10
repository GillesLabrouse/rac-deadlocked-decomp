#include "common.h"

/* Nonmatching: complete original688 bytes, compiled500 bytes. */
/* Complete L01 005617A8..00561A58 (0x2B0 bytes), including the return.
 * ABI: a0 output (at least 0x150 bytes), a1 aligned position quad, a2 actor
 * address, f12 first angle, f13 second angle. There are no direct calls.
 * The three genuine VCALLMS C60 executions require the resident VU0 program
 * installed by the MPG packet at 0010F1E0. C60 evaluates a reflected odd
 * polynomial in x and y; it is not replaced by a host sin/cos call.
 * PROT3W selects word1, so the returned pair is vf15.x / vf15.y.
 *
 * Raw quad copies retain position.w. Position is reloaded twice, after the
 * preceding output writes; the final global read also remains after them.
 * Untouched output fields and both position loads' alias effects are retained.
 * Output and position require 16-byte alignment, as the original LQ/SQ do.
 * This source has no native entry or claim of physical-VU float equivalence.
 */
#ifndef DEADLOCKED_NATIVE
typedef unsigned int CameraInitQuad __attribute__((mode(TI)));
typedef union CameraInitLanes {
    CameraInitQuad quad;
    f32 lane[4];
} CameraInitLanes;
typedef union CameraInitBits { u32 bits; f32 value; } CameraInitBits;
typedef char CameraInitQuadSize[sizeof(CameraInitQuad)==16 ? 1 : -1];
extern volatile f32 D_0021E754;

#define CAMERA_WORD(p,o) (*(volatile u32 *)((p)+(o)))
#define CAMERA_HALF(p,o) (*(volatile u16 *)((p)+(o)))
#define CAMERA_FLOAT(p,o) (*(volatile f32 *)((p)+(o)))
#define CAMERA_QUAD(p,o) (*(volatile CameraInitQuad *)((p)+(o)))

#define CAMERA_ANGLE_PAIR(angle,s,c) do { \
    CameraInitBits input; CameraInitLanes pair; \
    input.value=(angle); \
    __asm__ volatile("qmtc2.ni %1,$vf15\n\tvcallms 0xC60\n\tqmfc2.i %0,$vf15" \
        : "=r"(pair.quad) : "r"(input.bits) : "memory"); \
    (s)=pair.lane[0]; (c)=pair.lane[1]; \
} while(0)

void func_L01_005617A8(u8 *output, CameraInitQuad *position, u8 *actor,
    f32 angle12, f32 angle13)
{
    f32 s0,c0,s13,c13,s12,c12;
    f32 f0,f1,f2,f3,f4,f5,f14,f15;
    CameraInitLanes first,second,third;
    CameraInitQuad positionQuad;
    CAMERA_WORD(output,0x44)=0;
    CAMERA_ANGLE_PAIR(CAMERA_FLOAT(output,0x44),s0,c0);
    CAMERA_ANGLE_PAIR(angle13,s13,c13);
    f0=s0*s13;
    CAMERA_ANGLE_PAIR(angle12,s12,c12);
    f2=c0*s13;
    f14=f0*s12;
    f5=c0*s12;
    f4=-c12;
    f0=c12*f0;
    f1=c0*c12;
    f15=f2*s12;
    f3=s0*s12;
    f2=f2*c12;
    f0=f0-f5;
    f1=f1+f14;
    f4=f4*s0;
    f2=f2+f3;
    f4=f4+f15;
    second.lane[0]=f0;
    second.lane[1]=f1;
    third.lane[0]=f2;
    third.lane[1]=f4;
    f0=c13*s12;
    f1=c13*c12;
    f3=-s13;
    f2=c13*s0;
    f5=c0*c13;
    first.lane[0]=f1;
    first.lane[1]=f0;
    first.lane[2]=f3;
    first.lane[3]=0.0f;
    second.lane[2]=f2;
    second.lane[3]=0.0f;
    third.lane[2]=f5;
    third.lane[3]=0.0f;

    CAMERA_WORD(output,0x48)=0x3DD67751U;
    CAMERA_WORD(output,0x4C)=0x3D567751U;
    CAMERA_WORD(output,0x50)=0x3B64C389U;
    CAMERA_WORD(output,0x54)=0x3AFE2E98U;
    CAMERA_WORD(output,0x70)=0x3F19999AU;
    CAMERA_WORD(output,0x74)=0x3F2A7EFAU;
    CAMERA_WORD(output,0x78)=0x3F59999AU;
    CAMERA_WORD(output,0x7C)=0x3EB2B8C2U;
    CAMERA_WORD(output,0x120)=0x42340000U;
    CAMERA_WORD(output,0x5C)=0x3F000000U;
    CAMERA_WORD(output,0x68)=0x3D0EFA36U;
    CAMERA_WORD(output,0x6C)=0x3ABEA2F2U;
    CAMERA_WORD(output,0xBC)=(u32)actor;
    CAMERA_WORD(output,0xB4)=0x3FBDE44EU;
    CAMERA_WORD(output,0xB8)=0xBFBDE44EU;
    CAMERA_FLOAT(output,0x40)=angle12;
    CAMERA_FLOAT(output,0x60)=angle13;
    CAMERA_WORD(output,0x64)=0;
    CAMERA_HALF(output,0x86)=0;
    CAMERA_WORD(output,0x88)=0;
    CAMERA_HALF(output,0x84)=0;
    CAMERA_WORD(output,0x80)=0;
    CAMERA_WORD(output,0x12C)=0;
    CAMERA_WORD(output,0x138)=0;
    CAMERA_WORD(output,0x8C)=0;
    CAMERA_WORD(output,0xB0)=0;
    CAMERA_QUAD(output,0xA0)=0;
    CAMERA_QUAD(output,0x10)=second.quad;
    CAMERA_QUAD(output,0x20)=third.quad;
    CAMERA_QUAD(output,0xF0)=first.quad;
    CAMERA_QUAD(output,0)=first.quad;
    positionQuad=*(volatile CameraInitQuad *)position;
    CAMERA_QUAD(output,0x30)=positionQuad;
    positionQuad=*(volatile CameraInitQuad *)position;
    CAMERA_QUAD(output,0x140)=positionQuad;
    CAMERA_FLOAT(output,0x134)=D_0021E754;
}
#endif
