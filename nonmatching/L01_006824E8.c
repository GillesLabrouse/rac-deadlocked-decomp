/* cflags: -mno-split-addresses */
#include "common.h"

/* Complete 006824E8..006826E4, 508 bytes. The genuine body Euler words
 * F0/F4/F8 select three C60 executions. The resulting transposed rotation
 * transforms the captured input quad, then two real 55D708 calls publish
 * output X followed by Y. Output Z/W are untouched. No mode is suppressed.
 */
#ifndef DEADLOCKED_NATIVE
typedef unsigned int SurfaceEulerQuad __attribute__((mode(TI)));
typedef union SurfaceEulerLanes { SurfaceEulerQuad q;f32 lane[4]; } SurfaceEulerLanes;
typedef struct SurfaceEulerVector { f32 x,y,z,w; }
    __attribute__((aligned(16))) SurfaceEulerVector;
typedef union SurfaceEulerBits { f32 value;u32 bits; } SurfaceEulerBits;
f32 func_L01_0055D708(f32,f32);
#define SURFACE_TRIG(angle,s,c) do { \
    SurfaceEulerBits inputBits;SurfaceEulerLanes trig;inputBits.value=(angle); \
    __asm__ volatile("qmtc2.ni %1,$vf15\n\tvcallms 0xC60\n\tqmfc2.i %0,$vf15" \
        : "=r"(trig.q) : "r"(inputBits.bits) : "memory"); \
    (s)=trig.lane[0];(c)=trig.lane[1]; \
} while(0)
void func_L01_006824E8(u8 *record,const u8 *input,u8 *output) {
    u8 *body=(u8 *)*(volatile u32 *)(record+0x2EF0);
    f32 sx,cx,sy,cy,sz,cz,a,b,c,d,e,f,g,h,magnitude,pitch;
    SurfaceEulerVector col0,col1,col2,row0,row1,row2,transformed;
    SurfaceEulerLanes captured;
    __asm__ volatile("lqc2 $vf1,0(%1)\n\tqmfc2.ni %0,$vf1"
        : "=r"(captured.q) : "r"(input) : "memory");
    SURFACE_TRIG(*(volatile f32 *)(body+0xF0),sx,cx);
    SURFACE_TRIG(*(volatile f32 *)(body+0xF4),sy,cy);
    a=sx*sy;b=cx*sy;c=cy*sx;d=cx*cy;
    SURFACE_TRIG(*(volatile f32 *)(body+0xF8),sz,cz);
    e=a*sz;f=cx*sz;g=cz*a;h=cx*cz;
    col1.x=g-f;col1.y=h+e;col1.z=c;col1.w=0.f;
    e=b*sz;f=(-cz)*sx;g=sx*sz;h=b*cz;
    col2.x=h+g;col2.y=f+e;col2.z=d;col2.w=0.f;
    col0.x=cy*cz;col0.y=cy*sz;col0.z=-sy;col0.w=0.f;
    /* Transpose the real rotation columns; the homogeneous column is VF0. */
    row0.x=col0.x;row0.y=col1.x;row0.z=col2.x;row0.w=0.f;
    row1.x=col0.y;row1.y=col1.y;row1.z=col2.y;row1.w=0.f;
    row2.x=col0.z;row2.y=col1.z;row2.z=col2.z;row2.w=0.f;
    __asm__ volatile("qmtc2.ni %1,$vf1\n\tlqc2 $vf2,0(%2)\n\t"
        "vmulax.xyzw ACC,$vf2,$vf1x\n\tlqc2 $vf2,0(%3)\n\t"
        "vmadday.xyzw ACC,$vf2,$vf1y\n\tlqc2 $vf2,0(%4)\n\t"
        "vmaddaz.xyzw ACC,$vf2,$vf1z\n\tvmaddw.xyzw $vf1,$vf0,$vf1w\n\t"
        "sqc2 $vf1,0(%0)" : : "r"(&transformed),"r"(captured.q),
        "r"(&row0),"r"(&row1),"r"(&row2) : "memory");
    magnitude=transformed.x*transformed.x+transformed.z*transformed.z;
    __asm__ volatile("sqrt.s %0,%1" : "=f"(magnitude) : "f"(magnitude));
    pitch=func_L01_0055D708(magnitude,transformed.y);
    *(volatile f32 *)output=pitch;
    *(volatile f32 *)(output+4)=-func_L01_0055D708(transformed.z,transformed.x);
}
#undef SURFACE_TRIG
#endif
