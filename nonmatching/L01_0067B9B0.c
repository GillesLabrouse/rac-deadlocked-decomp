/* cflags: -ffast-math -mno-split-addresses */
/* asflags: --float-immediates */
/* Complete original0067B9B0..0067BC78 (712 bytes), NONMATCHING.
 * Metadata and globals reload around the genuine progression calls. */
#include "common.h"
#define R_FLOAT(o) (*(volatile f32 *)(record+(o)))
#define R_WORD(o) (*(volatile s32 *)(record+(o)))
#define R_BYTE(o) (*(volatile u8 *)(record+(o)))
#define R_HALF(o) (*(volatile s16 *)(record+(o)))
#define META_FLOAT(o) (*(volatile f32 *)(metadata+(o)))
extern volatile f32 D_0021DDE8,D_0021DDEC,D_0021DDF4;
f32 func_L01_00594AE8(f32 *,f32,f32);
f32 func_L01_00594D28(f32 *,f32 *,f32,f32,f32,f32);
static f32 radius_distance_xy(const u8 *left,const u8 *right) {
    union{u32 bits;f32 value;}scalar;
    f32 scratch[8] __attribute__((aligned(16)));
    __asm__ volatile("lqc2 $vf2,0(%1)\n\tlqc2 $vf1,0(%2)\n\t"
        "vsub.xyz $vf1,$vf1,$vf2\n\tvmul.xyzw $vf1,$vf1,$vf1\n\t"
        "vaddy.x $vf1,$vf1,$vf1y\n\tnop\n\tnop\n\tvsqrt Q,$vf1x\n\t"
        "sqc2 $vf1,0(%3)\n\tvwaitq\n\tvaddq.x $vf1,$vf0,Q\n\t"
        "qmfc2.ni %0,$vf1\n\tsqc2 $vf1,0x10(%3)"
        : "=r"(scalar.bits) : "r"(left),"r"(right),"r"(scratch) : "memory");
    return scalar.value;
}
void func_L01_0067B9B0(u8 *record,s32 initial) {
    u8 *metadata=(u8 *)(*(volatile u32 *)(record+0x2EEC));
    s32 previous=R_WORD(0x25D4),state;
    R_FLOAT(0x1F8)=META_FLOAT(0x14);
    R_FLOAT(0x1FC)=META_FLOAT(0x18);
    R_FLOAT(0x200)=META_FLOAT(0x10);
    if(previous==4) {
        if(R_HALF(0x1D1C)<R_WORD(0x370) && R_BYTE(0x1D0E)==0)
            R_FLOAT(0x1FC)=R_FLOAT(0x1D28);
    } else {
        state=R_WORD(0x25CC);
        if(state==6) R_FLOAT(0x1FC)=META_FLOAT(0x1C);
        else if(state==4 || R_BYTE(0x2ECD)!=0)
            R_FLOAT(0x1F8)=0.8f-META_FLOAT(0x10);
        else if(previous==17 || previous==18) {
            R_FLOAT(0x1F8)=META_FLOAT(0x14)-0.8f;
            R_FLOAT(0x1FC)=META_FLOAT(0x18)-0.7f;
        } else if(state==126) R_FLOAT(0x200)=0.8f;
    }
    if(initial!=0) {
        f32 radius=R_FLOAT(0x200);
        R_FLOAT(0x1F0)=R_FLOAT(0x1F8);R_FLOAT(0x1F4)=R_FLOAT(0x1FC);
        R_FLOAT(0x240)=radius*radius;R_FLOAT(0x204)=radius;
    }
    if(R_BYTE(0x226)!=0 && R_WORD(0x25DC)!=18 && R_WORD(0x25E8)!=18 &&
       !(R_FLOAT(0x1F4)<META_FLOAT(0x18)-0.17f) && R_WORD(0x25D4)!=17 && R_BYTE(0x434)==0 &&
       !(radius_distance_xy(record+0xA0,record+0x1E0)>R_FLOAT(0x204)*0.5f)) return;
    {
        f32 force,damping,maximum,scale,timing;
        func_L01_00594AE8((f32 *)(record+0x1F0),R_FLOAT(0x1F8),(D_0021DDE8*0.02f)*R_FLOAT(0x25C4));
        func_L01_00594AE8((f32 *)(record+0x1F4),R_FLOAT(0x1FC),(D_0021DDE8*0.02f)*R_FLOAT(0x25C4));
        timing=D_0021DDEC;force=timing*0.02f;damping=timing*0.3f;maximum=D_0021DDF4*4.f;
        scale=R_FLOAT(0x25C4);
        force=force*scale;damping=damping*scale;maximum=maximum*scale;
        func_L01_00594D28((f32 *)(record+0x204),(f32 *)(record+0x208),R_FLOAT(0x200),force,damping,maximum);
        R_FLOAT(0x240)=R_FLOAT(0x204)*R_FLOAT(0x204);
    }
}
