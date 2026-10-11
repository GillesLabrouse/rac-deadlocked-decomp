/* asflags: --float-immediates */
/* Complete 508-byte slope response, including final padding. NONMATCHING.
 * Genuine gate fields, angle helper, planar VU normalization and two velocity
 * stores are retained. The live time word is read after the first store. */
#include "common.h"
#include "../include/dreadzone_actor_velocity_internal.h"

extern volatile f32 D_0021DDF8;
extern f32 func_L01_0055D708(f32,f32);
#define SLOPE_FLOAT(o) (*(volatile f32 *)(record+(o)))

static f32 slope_length_xy(const VelocityVector *normal) {
    VelocityBits value;
    __asm__ volatile("lqc2 $vf1,0(%1)\n\tvmul.xyzw $vf2,$vf1,$vf1\n\t"
        "vaddy.x $vf2,$vf2,$vf2y\n\tnop\n\tnop\n\t"
        "vsqrt Q,$vf2x\n\tvwaitq\n\tvaddq.x $vf1,$vf0,Q\n\t"
        "qmfc2.ni %0,$vf1" : "=r"(value.word) : "r"(normal) : "memory");
    return value.value;
}
static void slope_planar_normal(const VelocityVector *normal,VelocityVector *output) {
    VelocityVector scratch;u64 sum;
    __asm__ volatile("lqc2 $vf4,0(%2)\n\tqmtc2.ni $0,$vf1\n\t"
        "vmove.xyzw $vf2,$vf4\n\tvaddx.z $vf2,$vf0,$vf1x\n\t"
        "vmul.xyzw $vf3,$vf2,$vf2\n\tvmaxw.xyzw $vf1,$vf0,$vf0w\n\t"
        "vadday.x ACC,$vf3,$vf3y\n\tvmaddz.x $vf1,$vf1,$vf3z\n\t"
        "qmfc2.ni %0,$vf1\n\tsqc2 $vf2,0(%1)\n\tnop\n\tnop\n\t"
        "vrsqrt Q,$vf0w,$vf1x\n\t.set noreorder\n\tbeqz %0,1f\n\tnop\n\t"
        "vwaitq\n\tlqc2 $vf1,0(%1)\n\tvmulq.xyz $vf1,$vf1,Q\n\t"
        "b 2f\n\tsqc2 $vf1,0(%1)\n\t1:\n\tvsub.xyz $vf1,$vf0,$vf0\n\t"
        "sqc2 $vf1,0(%1)\n\t2:\n\tsqc2 $vf1,0(%3)\n\t.set reorder"
        : "=&r"(sum) : "r"(output),"r"(normal),"r"(&scratch) : "memory");
    /* The BEQ tests low64, including VF1.Y=1. It is not a vector-zero test. */
}
static void slope_add_velocity(u8 *record,const VelocityVector *normal,f32 amount) {
    VelocityBits scalar;scalar.value=amount;
    __asm__ volatile("lqc2 $vf2,0(%1)\n\tqmtc2.ni %2,$vf1\n\t"
        "vmulx.xyz $vf2,$vf2,$vf1x\n\tlqc2 $vf1,0x130(%0)\n\t"
        "vadd.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0x130(%0)"
        : : "r"(record),"r"(normal),"r"(scalar.word) : "memory");
}
static void slope_lower_velocity(u8 *record,f32 amount) {
    VelocityBits scalar;scalar.value=amount;
    __asm__ volatile("lqc2 $vf1,0x130(%0)\n\tqmtc2.ni %1,$vf2\n\t"
        "vsubx.z $vf1,$vf1,$vf2x\n\tsqc2 $vf1,0x130(%0)"
        : : "r"(record),"r"(scalar.word) : "memory");
}
void func_L01_0067D4E8(u8 *record) {
    VelocityVector normal,velocity;
    volatile f32 negative_x,product_x,product_y,dot,scale,time,rate,offset;
    if(*(volatile s16 *)(record+0x2FE)==0 || !(SLOPE_FLOAT(0x2CC)<0.15f) ||
       !(SLOPE_FLOAT(0x158)>0.f) || !record[0x226] || !(SLOPE_FLOAT(0x1D8)>0.f))return;
    if(!(func_L01_0055D708(SLOPE_FLOAT(0x1D8),slope_length_xy((const VelocityVector *)(record+0x1D0)))
         >=0.872664630413055419921875f))return;
    slope_planar_normal((const VelocityVector *)(record+0x1D0),&normal);
    *(VelocityQuad *)&velocity=*(volatile VelocityQuad *)(record+0x130);
    negative_x=-velocity.v[0];product_y=velocity.v[1]*normal.v[1];
    product_x=negative_x*normal.v[0];dot=product_x-product_y;
    if(!(dot>0.f))return;
    scale=SLOPE_FLOAT(0x25C4);
    slope_add_velocity(record,&normal,dot);
    time=D_0021DDF8;rate=time*54.f;offset=rate*scale;offset=offset+offset;
    slope_lower_velocity(record,offset);
}
