/* cflags: -ffast-math -mno-split-addresses */
/* asflags: --float-immediates */
/* Complete original0067EA88..0067EE9C (1044 bytes), NONMATCHING.
 * Genuine surface settling and height history; no probe is substituted. */
#include "common.h"
typedef unsigned int PositionSettleQuad __attribute__((mode(TI)));
#define S_FLOAT(o) (*(volatile f32 *)(record+(o)))
#define S_WORD(o) (*(volatile s32 *)(record+(o)))
#define S_HALF(o) (*(volatile s16 *)(record+(o)))
#define S_BYTE(o) (*(volatile u8 *)(record+(o)))
#define S_MIN(a,b) ((a)<(b)?(a):(b))
extern volatile f32 D_0021DDEC,D_0021DDF4;
/* Actual GP22597F-7B93 =0021DDEC; use that single original global. */
f32 func_L01_0068AB40(u8 *,const u8 *);
f32 func_L01_00594D28(f32 *,f32 *,f32,f32,f32,f32);
f32 func_L01_00594DC8(u8 *,const u8 *,f32 *,f32,f32,f32);
static f32 settle_xy_distance(const u8 *left,const u8 *right) {
    u32 result;
    f32 scratch[8] __attribute__((aligned(16)));
    __asm__ volatile("lqc2 $vf1,0(%1)\n\tlqc2 $vf2,0(%2)\n\t"
        "vsub.xyz $vf1,$vf1,$vf2\n\tvmul.xyzw $vf1,$vf1,$vf1\n\t"
        "vaddy.x $vf1,$vf1,$vf1y\n\tnop\n\tnop\n\tvsqrt Q,$vf1x\n\t"
        "sqc2 $vf1,0(%3)\n\tvwaitq\n\tvaddq.x $vf1,$vf0,Q\n\t"
        "qmfc2.ni %0,$vf1\n\tsqc2 $vf1,0x10(%3)"
        : "=r"(result) : "r"(left),"r"(right),"r"(scratch) : "memory");
    {union{u32 bits;f32 value;}r;r.bits=result;return r.value;}
}
void func_L01_0067EA88(u8 *record) {
    if(S_BYTE(0x2659)!=0) {
        if(S_HALF(0x2FE)!=0 || !(S_FLOAT(0x2CC)<0.f)) {S_WORD(0x2F4)=0;return;}
        if(__builtin_fabsf(S_FLOAT(0x2CC))<
                __builtin_fabsf(func_L01_0068AB40(record,record+0x130))+0.01f) {
            if(S_BYTE(0x226)==0 || settle_xy_distance(record+0x1E0,record+0xA0)>S_FLOAT(0x204)*0.5f) {
                *(PositionSettleQuad *)(record+0xA0)=*(PositionSettleQuad *)(record+0x280);
                return;
            }
        }
            {
                f32 timing=D_0021DDEC,scale=S_FLOAT(0x25C4);
                f32 force=timing*0.05f,damping=timing*0.21f,maximum=D_0021DDF4*5.f;
                force=force*scale;damping=damping*scale;maximum=maximum*scale;
                func_L01_00594DC8(record+0xA0,record+0x280,(f32 *)(record+0x2F4),force,damping,maximum);
            }
        return;
    }
    if(S_HALF(0x2FE)==0 && S_FLOAT(0xA8)<S_FLOAT(0x2C8)) {
        {
            if(__builtin_fabsf(S_FLOAT(0xA8)-S_FLOAT(0x2C8))<
                 __builtin_fabsf(S_FLOAT(0x138))+0.01f && (S_BYTE(0x226)==0 ||
                 settle_xy_distance(record+0x1E0,record+0xA0)>S_FLOAT(0x204)*0.5f)) {
                S_FLOAT(0xA8)=S_FLOAT(0x2C8);
                goto history;
            }
            if(S_FLOAT(0x168)<0.f) {
                S_FLOAT(0xA8)=S_FLOAT(0xA8)-S_FLOAT(0x168);
                S_FLOAT(0xA8)=S_MIN(S_FLOAT(0xA8),S_FLOAT(0x2C8));
            }
            {
                f32 timing=D_0021DDEC,maximum=D_0021DDF4*5.f;
                f32 force=timing*0.05f,damping=timing*0.17f,scale=S_FLOAT(0x25C4);
                force=force*scale;damping=damping*scale;maximum=maximum*scale;
                func_L01_00594D28((f32 *)(record+0xA8),(f32 *)(record+0x2F4),S_FLOAT(0x2C8),force,damping,maximum);
            }
            goto history;
        }
    }
    S_WORD(0x2F4)=0;
history:
    if((S_WORD(0x25D4)!=0 && S_WORD(0x25CC)!=2) || S_FLOAT(0x1B4)!=0.f) {
        S_WORD(0x308)=0;S_WORD(0x300)=0;S_WORD(0x304)=0;return;
    }
    if(S_FLOAT(0x304)<S_FLOAT(0xA8)) {S_FLOAT(0x304)=S_FLOAT(0xA8);return;}
    if(S_FLOAT(0xA8)<S_FLOAT(0x308) || S_FLOAT(0x308)==0.f) {
        S_FLOAT(0x308)=S_FLOAT(0xA8);return;
    }
    if(S_FLOAT(0x304)-S_FLOAT(0x308)<0.03f) {
        S_WORD(0x300)=1;S_FLOAT(0xA8)=S_FLOAT(0x304);return;
    }
    S_WORD(0x308)=0;S_WORD(0x300)=0;S_WORD(0x304)=0;
}
