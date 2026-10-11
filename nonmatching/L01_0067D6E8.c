/* asflags: --float-immediates */
#include "common.h"

/* Complete NONMATCHING wall response 0067D6E8..0067DBE8, 0x500 bytes.
 * Integer ABI: (record, force); no incoming floating argument is consumed.
 * All seven genuine calls, scheduling gates, live packet reloads and VU/Q
 * effects remain. There is no native entry or captured collision result.
 *
 * record130 is velocity, record20..50 is a four-quad transform, recordA0 is
 * position. Two independent mask2 probes overwrite the real packet242340.
 * The FIRST surface is retained across the second probe; the SECOND packet
 * supplies inclination, object18 and normal40. Surfaces8/12 or inclination
 * >=50 degrees select projection, normalization and inward-velocity removal.
 * QMFC2 zero predicates compare low64, including the unchanged y word,
 * rather than treating the computed norm's x word as a scalar zero test.
 * Unwritten scratch/quad W lanes retain the actual VU masked operations.
 */
#ifndef DEADLOCKED_NATIVE
typedef struct WallResponseVector { f32 x,y,z,w; }
    __attribute__((aligned(16))) WallResponseVector;
typedef unsigned int WallResponseQuad __attribute__((mode(TI)));
typedef union WallResponseBits { f32 value;u32 bits; } WallResponseBits;

extern volatile f32 D_0021DDF4;
extern volatile s32 D_0021DDB8;
extern volatile u8 D_00220648;
extern u8 D_00242340[];

f32 func_L01_0068A920(const u8 *,const WallResponseVector *);
void func_L01_0068ABA8(u8 *,WallResponseVector *,const WallResponseVector *,f32);
s32 func_L01_0053DC70(u8 *,u8 *,s32,u8 *,s32);
s32 func_L01_00542FF8(void);
f32 func_L01_0068AFF8(u8 *,const WallResponseVector *);

#define WRWORD(p,o) (*(volatile u32 *)((u8 *)(p)+(o)))
#define WRSWORD(p,o) (*(volatile s32 *)((u8 *)(p)+(o)))
#define WRBYTE(p,o) (*(volatile u8 *)((u8 *)(p)+(o)))
#define WRFLOAT(p,o) (*(volatile f32 *)((u8 *)(p)+(o)))

void func_L01_0067D6E8(u8 *record,s32 force)
{
    WallResponseVector scratch,temporary10,temporary20,firstDelta,firstStart;
    WallResponseVector firstEnd,secondStart,normal,ones;
    WallResponseVector localOffset,localShortOffset;
    WallResponseBits scalar,length;
    u64 predicate;
    f32 height,radius,inclination;
    s32 surface=-1;
    u8 *packet=D_00242340;

    if(func_L01_0068A920(record,(const WallResponseVector *)(record+0x130))<
        D_0021DDF4*0.1f*WRFLOAT(record,0x25C4))return;
    if(!WRBYTE(record,0x266D)) {
        u8 scheduling=D_00220648;
        s32 parity=D_0021DDB8; /* Actual delay-slot load even if byte648 is0. */
        if(scheduling && (WRWORD(record,0x2F10)&7U)!=((u32)parity&7U) &&
            WRSWORD(record,0x2F5C)<0 && !WRWORD(record,0x1848))return;
    }
    height=0.37f;radius=0.5f;
    if((u32)(WRWORD(record,0x25D4)-17U)<2U){height=0.f;radius=0.7f;}
    localOffset.x=0.f;localOffset.y=0.f;localOffset.z=height;localOffset.w=0.f;
    localShortOffset.x=0.f;localShortOffset.y=0.f;
    localShortOffset.z=height*0.3f;localShortOffset.w=0.f;
    __asm__ volatile("lqc2 $vf2,0(%1)\n\tlqc2 $vf6,0x20(%0)\n\t"
        "sqc2 $vf2,0(%2)\n\tlqc2 $vf1,0x30(%0)\n\t"
        "vmulax.xyzw ACC,$vf6,$vf2x\n\tlqc2 $vf3,0x40(%0)\n\t"
        "vmadday.xyzw ACC,$vf1,$vf2y\n\tlqc2 $vf1,0x50(%0)\n\t"
        "vmaddaz.xyzw ACC,$vf3,$vf2z\n\tvmaddw.xyzw $vf1,$vf1,$vf2w\n\t"
        "lqc2 $vf3,0(%3)\n\tsqc2 $vf1,0(%4)\n\t"
        "vmulax.xyzw ACC,$vf6,$vf3x\n\tsqc2 $vf1,0(%2)\n\t"
        "lqc2 $vf5,0xA0(%0)\n\tvadd.xyz $vf1,$vf1,$vf5\n\t"
        "sqc2 $vf3,0(%5)\n\tsqc2 $vf1,0(%2)\n\t"
        "lqc2 $vf2,0x30(%0)\n\tlqc2 $vf4,0x40(%0)\n\t"
        "vmadday.xyzw ACC,$vf2,$vf3y\n\tlqc2 $vf1,0x50(%0)\n\t"
        "vmaddaz.xyzw ACC,$vf4,$vf3z\n\tvmaddw.xyzw $vf1,$vf1,$vf3w\n\t"
        "sqc2 $vf1,0(%5)\n\tsqc2 $vf1,0(%4)\n\t"
        "vadd.xyz $vf1,$vf1,$vf5\n\tsqc2 $vf1,0(%5)"
        : : "r"(record),"r"(&localOffset),"r"(&secondStart),
            "r"(&localShortOffset),"r"(&scratch),"r"(&firstStart) : "memory");

    length.value=0.f;
    if(!force) {
        __asm__ volatile("lqc2 $vf1,0x130(%1)\n\tvmul.xyzw $vf1,$vf1,$vf1\n\t"
            "vaddy.x $vf1,$vf1,$vf1y\n\tnop\n\tnop\n\tvsqrt Q,$vf1x\n\t"
            "sqc2 $vf1,0(%2)\n\tvwaitq\n\tvaddq.x $vf1,$vf0,Q\n\t"
            "qmfc2.ni %0,$vf1\n\tsqc2 $vf1,0(%3)"
            : "=&r"(length.bits) : "r"(record),"r"(&temporary10),"r"(&temporary20) : "memory");
    }
    if(force||length.value<0.01f) {
        scalar.value=radius;
        __asm__ volatile("vmove.xyzw $vf5,$vf6\n\tvmaxw.xyzw $vf1,$vf0,$vf0w\n\t"
            "sqc2 $vf1,0(%1)\n\tvmove.xyzw $vf4,$vf5\n\tqmtc2.ni %2,$vf3\n\t"
            "vmul.xyzw $vf2,$vf4,$vf4\n\tvadday.x ACC,$vf2,$vf2y\n\t"
            "vmaddz.x $vf1,$vf1,$vf2z\n\tnop\n\tnop\n\t"
            "vrsqrt Q,$vf3x,$vf1x\n\tqmfc2.ni %0,$vf1\n\tsqc2 $vf1,0(%3)"
            : "=&r"(predicate) : "r"(&ones),"r"(scalar.bits),"r"(&scratch) : "memory");
        if(predicate) {
            __asm__ volatile("vwaitq\n\tvmove.xyzw $vf4,$vf5\n\tvmulq.xyz $vf4,$vf4,Q" : : : "memory");
        } else {
            __asm__ volatile("vsub.xyz $vf4,$vf0,$vf0" : : : "memory");
        }
        scalar.value=radius*1.4f;
        __asm__ volatile("lqc2 $vf5,0x20(%1)\n\tlqc2 $vf2,0(%2)\n\t"
            "vmove.xyzw $vf3,$vf5\n\tvmul.xyzw $vf1,$vf3,$vf3\n\t"
            "vadday.x ACC,$vf1,$vf1y\n\tsqc2 $vf4,0(%3)\n\t"
            "vmaddz.x $vf1,$vf2,$vf1z\n\tqmtc2.ni %4,$vf2\n\t"
            "sqc2 $vf1,0(%5)\n\tnop\n\tnop\n\tvrsqrt Q,$vf2x,$vf1x\n\t"
            "qmfc2.ni %0,$vf1" : "=&r"(predicate) : "r"(record),"r"(&ones),
            "r"(&firstDelta),"r"(scalar.bits),"r"(&scratch) : "memory");
        if(predicate) {
            __asm__ volatile("vwaitq\n\tvmove.xyzw $vf3,$vf5\n\tvmulq.xyz $vf3,$vf3,Q\n\t"
                "sqc2 $vf3,0(%0)" : : "r"(&firstEnd) : "memory");
        } else {
            __asm__ volatile("vsub.xyz $vf3,$vf0,$vf0\n\tsqc2 $vf3,0(%0)" : : "r"(&firstEnd) : "memory");
        }
    } else {
        func_L01_0068ABA8(record,&firstDelta,(const WallResponseVector *)(record+0x130),0.f);
        scalar.value=radius;
        __asm__ volatile("lqc2 $vf5,0(%1)\n\tvmaxw.xyzw $vf1,$vf0,$vf0w\n\t"
            "sqc2 $vf1,0(%2)\n\tqmtc2.ni %3,$vf3\n\tvmove.xyzw $vf4,$vf5\n\t"
            "vmul.xyzw $vf2,$vf4,$vf4\n\tvadday.x ACC,$vf2,$vf2y\n\t"
            "vmaddz.x $vf1,$vf1,$vf2z\n\tnop\n\tnop\n\t"
            "vrsqrt Q,$vf3x,$vf1x\n\tqmfc2.ni %0,$vf1\n\tsqc2 $vf1,0(%4)"
            : "=&r"(predicate) : "r"(&firstDelta),"r"(&ones),"r"(scalar.bits),"r"(&scratch) : "memory");
        if(predicate) {
            __asm__ volatile("vwaitq\n\tvmove.xyzw $vf4,$vf5\n\tvmulq.xyz $vf4,$vf4,Q" : : : "memory");
        } else {
            __asm__ volatile("vsub.xyz $vf4,$vf0,$vf0" : : : "memory");
        }
        scalar.value=1.4f;
        __asm__ volatile("sqc2 $vf4,0(%0)\n\tqmtc2.ni %2,$vf3\n\t"
            "vmulx.xyz $vf4,$vf4,$vf3x\n\tsqc2 $vf4,0(%1)"
            : : "r"(&firstDelta),"r"(&firstEnd),"r"(scalar.bits) : "memory");
    }
    __asm__ volatile("lqc2 $vf2,0(%0)\n\tlqc2 $vf3,0(%1)\n\t"
        "lqc2 $vf4,0(%2)\n\tlqc2 $vf1,0(%3)\n\tvadd.xyz $vf2,$vf2,$vf4\n\t"
        "vadd.xyz $vf3,$vf3,$vf1\n\tsqc2 $vf2,0(%0)\n\tsqc2 $vf3,0(%1)"
        : : "r"(&firstDelta),"r"(&firstEnd),"r"(&secondStart),"r"(&firstStart) : "memory");
    if(func_L01_0053DC70((u8 *)&firstStart,(u8 *)&firstEnd,2,
        (u8 *)WRWORD(record,0x2EF0),0) && WRSWORD(packet,0x1C)>0) {
        surface=func_L01_00542FF8();
    }
    inclination=0.f;
    if(func_L01_0053DC70((u8 *)&secondStart,(u8 *)&firstDelta,2,
        (u8 *)WRWORD(record,0x2EF0),0) && WRSWORD(packet,0x1C)>0) {
        inclination=func_L01_0068AFF8(record,(const WallResponseVector *)(packet+0x40));
    }
    if(!(0.87266463f<=inclination)&&surface!=8&&surface!=12)return;
    WRWORD(record,0x210)=WRWORD(packet,0x18);
    func_L01_0068ABA8(record,(WallResponseVector *)(packet+0x40),
        (const WallResponseVector *)(packet+0x40),0.f);
    __asm__ volatile("lqc2 $vf1,0x40(%1)\n\tvmul.xyzw $vf2,$vf1,$vf1\n\t"
        "vadday.x ACC,$vf2,$vf2y\n\tsqc2 $vf1,0(%2)\n\tlqc2 $vf1,0(%3)\n\t"
        "vmaddz.x $vf2,$vf1,$vf2z\n\tqmfc2.ni %0,$vf2\n\tsqc2 $vf2,0(%4)\n\t"
        "nop\n\tnop\n\tvrsqrt Q,$vf0w,$vf2x" : "=&r"(predicate)
        : "r"(packet),"r"(&normal),"r"(&ones),"r"(&scratch) : "memory");
    if(predicate) {
        __asm__ volatile("vwaitq\n\tlqc2 $vf1,0(%0)\n\tvmulq.xyz $vf1,$vf1,Q\n\t"
            "sqc2 $vf1,0(%0)" : : "r"(&normal) : "memory");
    } else {
        __asm__ volatile("vsub.xyz $vf1,$vf0,$vf0\n\tsqc2 $vf1,0(%0)\n\t"
            "sqc2 $vf1,0(%1)" : : "r"(&normal),"r"(&temporary10) : "memory");
    }
    *(volatile WallResponseQuad *)(packet+0x40)=*(WallResponseQuad *)&normal;
    __asm__ volatile("lqc2 $vf2,0x40(%1)\n\tlqc2 $vf1,0x130(%2)\n\t"
        "vmul.xyzw $vf1,$vf2,$vf1\n\tlqc2 $vf3,0(%3)\n\tvadday.x ACC,$vf1,$vf1y\n\t"
        "vmaddz.x $vf1,$vf3,$vf1z\n\tqmfc2.ni %0,$vf1"
        : "=&r"(scalar.bits) : "r"(packet),"r"(record),"r"(&ones) : "memory");
    __asm__ volatile("sqc2 $vf1,0(%0)" : : "r"(&scratch) : "memory");
    if(scalar.value<0.f) {
        scalar.value=-scalar.value;
        __asm__ volatile("qmtc2.ni %2,$vf1\n\tvmulx.xyz $vf2,$vf2,$vf1x\n\t"
            "sqc2 $vf2,0x40(%0)\n\tlqc2 $vf1,0x130(%1)\n\t"
            "vadd.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0x130(%1)"
            : : "r"(packet),"r"(record),"r"(scalar.bits) : "memory");
    }
}
#endif
