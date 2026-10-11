/* cflags: -ffast-math -mno-split-addresses */
/* Complete original0068A3F8..0068A638 (576 bytes), NONMATCHING.
 * Copies output before reloading body pointer in mode0; aliases matter. */
#include "common.h"
typedef struct PositionOffsetVector { f32 x,y,z,w; }
    __attribute__((aligned(16))) PositionOffsetVector;
typedef unsigned int PositionOffsetQuad __attribute__((mode(TI)));
typedef union PositionOffsetLanes { PositionOffsetQuad q;f32 lane[4]; } PositionOffsetLanes;
typedef union PositionOffsetBits { f32 f;u32 bits; } PositionOffsetBits;
#define OFFSET_TRIG(value,s,c) do { \
    PositionOffsetBits inputBits;PositionOffsetLanes result;inputBits.f=(value); \
    __asm__ volatile("qmtc2.ni %1,$vf15\n\tvcallms 0xC60\n\tqmfc2.i %0,$vf15" \
        : "=r"(result.q) : "r"(inputBits.bits) : "memory"); \
    (s)=result.lane[0];(c)=result.lane[1]; \
} while(0)
void func_L01_0068A3F8(u8 *record,PositionOffsetVector *output,
                      const PositionOffsetVector *input,f32 amount) {
    u8 mode=record[0x2659];
    u8 *body;
    f32 sx,cx,sy,cy,sz,cz,xy,yx,yz,xz,s,c;
    PositionOffsetVector offset,col0,col1,col2;
    if(mode==0) {
        PositionOffsetBits dx,dy;
        __asm__ volatile("lqc2 $vf1,0(%1)\n\tsqc2 $vf1,0(%0)"
            : : "r"(output),"r"(input) : "memory");
        body=(u8 *)(*(volatile u32 *)(record+0x2EF0));
        OFFSET_TRIG(*(volatile f32 *)(body+0xF8),s,c);
        dy.f=s*amount;dx.f=c*amount;
        output->x=output->x+dx.f;
        output->y=output->y+dy.f;
    } else if(mode==1 || mode==2) {
        PositionOffsetLanes preserved;
        OFFSET_TRIG(0.f,s,c);
        __asm__ volatile("lqc2 $vf3,0(%1)\n\tsqc2 $vf3,0(%0)"
            : : "r"(&preserved),"r"(input) : "memory");
        offset.x=c*amount;offset.y=s*amount;offset.z=0.f;offset.w=0.f;
        body=(u8 *)(*(volatile u32 *)(record+0x2EF0));
        OFFSET_TRIG(*(volatile f32 *)(body+0xF0),sx,cx);
        OFFSET_TRIG(*(volatile f32 *)(body+0xF4),sy,cy);
        xy=sx*sy;yx=cx*sy;yz=cy*sx;xz=cx*cy;
        OFFSET_TRIG(*(volatile f32 *)(body+0xF8),sz,cz);
        col0.x=cy*cz;col0.y=cy*sz;col0.z=-sy;col0.w=0.f;
        col1.x=cz*xy-cx*sz;col1.y=cx*cz+xy*sz;col1.z=yz;col1.w=0.f;
        col2.x=cz*yx+sx*sz;col2.y=(-cz)*sx+yx*sz;col2.z=xz;col2.w=0.f;
        __asm__ volatile("lqc2 $vf2,0(%1)\n\tlqc2 $vf4,0(%2)\n\t"
            "vmulax.xyzw ACC,$vf4,$vf2x\n\tlqc2 $vf4,0(%3)\n\t"
            "vmadday.xyzw ACC,$vf4,$vf2y\n\tlqc2 $vf4,0(%4)\n\t"
            "vmaddaz.xyzw ACC,$vf4,$vf2z\n\tvmaddw.xyzw $vf1,$vf0,$vf2w\n\t"
            "lqc2 $vf3,0(%5)\n\tvadd.xyz $vf3,$vf3,$vf1\n\tsqc2 $vf3,0(%0)"
            : : "r"(output),"r"(&offset),"r"(&col0),"r"(&col1),"r"(&col2),"r"(&preserved)
            : "memory");
    }
}
#undef OFFSET_TRIG
