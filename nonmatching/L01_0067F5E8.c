/* asflags: --float-immediates */
#include "common.h"

/* Complete original 0067F5E8..0067FF38 (2384 bytes), NONMATCHING.
 * Unary record ABI; the incidental final v0 is not a position result.
 * Both position writes, all 19 calls and all state-dependent VU branches
 * remain. No contact, object response or settling call is substituted.
 *
 * 130 is the primary displacement, 140 the second contribution, A0 the
 * position. The first contact phase precedes the observed/projection fields
 * 160/170/180 and metrics 1B0..1C0; the second phase publishes 190/1C4.
 * Masked XYZ/XY operations retain raw W/Z lanes. QMFC2 predicates are
 * low64 comparisons, including the unsummed Y word, not scalar X tests.
 * The original 68B258 consumes only a0, despite stale a1/a2/a3 at F650.
 * 67EEA0 consumes a raw quad in a1. 67F518 and 67EA88 are unary.
 * Complex ground/object/settling dependencies remain genuine callees.
 */
#ifndef DEADLOCKED_NATIVE
typedef struct PositionUpdateVector { f32 x,y,z,w; }
    __attribute__((aligned(16))) PositionUpdateVector;
typedef unsigned int PositionUpdateQuad __attribute__((mode(TI)));
typedef union PositionUpdateScalar { f32 value;u32 bits; } PositionUpdateScalar;
typedef union PositionUpdatePacked {
    PositionUpdateVector vector;PositionUpdateQuad quad;u32 word[4];
} PositionUpdatePacked;

extern volatile s32 D_0021E824;
extern volatile f32 D_0021DDF4;
void func_L01_0068B258(u8 *);
f32 func_L01_0068A920(const u8 *,const PositionUpdateVector *);
f32 func_L01_0068AB40(const u8 *,const PositionUpdateVector *);
void func_L01_0068AC58(const u8 *,PositionUpdateVector *,const PositionUpdateVector *,f32);
void func_L01_0068ABA8(const u8 *,PositionUpdateVector *,const PositionUpdateVector *,f32);
void func_L01_0068A7F0(u8 *,PositionUpdateVector *,const PositionUpdateVector *);
void func_L01_0067F518(u8 *);
void func_L01_0067DDD0(u8 *,s32);
void func_L01_0067EA88(u8 *);
f32 func_L01_0067EEA0(u8 *,PositionUpdateQuad);

#define PU_FLOAT(o) (*(volatile f32 *)(record+(o)))
#define PU_WORD(o) (*(volatile s32 *)(record+(o)))
#define PU_HALF(o) (*(volatile s16 *)(record+(o)))
#define PU_BYTE(o) (*(volatile u8 *)(record+(o)))
#define PU_VECTOR(o) ((PositionUpdateVector *)(record+(o)))
#define PU_QUAD(o) (*(volatile PositionUpdateQuad *)(record+(o)))
#define PU_ONES(v) __asm__ volatile("vmaxw.xyzw " v ",$vf0,$vf0w\n\t" \
    "sqc2 " v ",0(%0)" : : "r"(&ones) : "memory")

/* Normalize the observed displacement, including the original zero branch
 * store to temporary10. The low64 result includes squared Y. */
#define PU_NORMALIZE_LOCAL(local) do { \
    __asm__ volatile("vmul.xyzw $vf2,$vf1,$vf1\n\t" \
        "lqc2 $vf3,0(%1)\n\tvadday.x ACC,$vf2,$vf2y\n\t" \
        "vmaddz.x $vf2,$vf3,$vf2z\n\tsqc2 $vf1,0(%2)\n\t" \
        "qmfc2.ni %0,$vf2\n\tsqc2 $vf2,0(%3)\n\tnop\n\tnop\n\t" \
        "vrsqrt Q,$vf0w,$vf2x" : "=&r"(predicate) \
        : "r"(&ones),"r"(&(local)),"r"(&scratch) : "memory"); \
    __asm__ volatile(".set noreorder\n\tbeqz %2,1f\n\tnop\n\t" \
        "vwaitq\n\tlqc2 $vf1,0(%0)\n\tvmulq.xyz $vf1,$vf1,Q\n\t" \
        "b 2f\n\tsqc2 $vf1,0(%0)\n\t1:\n\tvsub.xyz $vf1,$vf0,$vf0\n\t" \
        "sqc2 $vf1,0(%0)\n\tsqc2 $vf1,0(%1)\n\t2:\n\t.set reorder" \
        : : "r"(&(local)),"r"(&temporary10),"r"(predicate) : "memory"); \
} while(0)

/* Preserve the intermediate normalized-direction store, then replace it
 * with velocity scaled by max(dot(direction,velocity),0)/sqrt(norm2).
 * The norm destination starts as ones: Y remains one in the low64 test. */
#define PU_PROJECT(input,local,destination) do { \
    __asm__ volatile("lqc2 $vf2,0(%1)\n\tvmaxw.xyzw $vf4,$vf0,$vf0w\n\t" \
        "lqc2 $vf5,0(%2)\n\tvmul.xyzw $vf1,$vf5,$vf2\n\t" \
        "vmove.xyzw $vf7,$vf2\n\tvadday.x ACC,$vf1,$vf1y\n\t" \
        "lqc2 $vf2,0(%3)\n\tvmaddz.x $vf1,$vf2,$vf1z\n\t" \
        "vmove.xyzw $vf6,$vf7\n\tqmfc2.ni %0,$vf1\n\t" \
        "vmul.xyzw $vf3,$vf6,$vf6\n\tsqc2 $vf1,0(%4)" \
        : "=&r"(scalar.bits) : "r"(input),"r"(&(local)),"r"(&ones),"r"(&scratch) : "memory"); \
    __asm__ volatile("max.s %0,%1,%2" : "=f"(scalar.value) \
        : "f"(scalar.value),"f"(0.f)); \
    __asm__ volatile("vadday.x ACC,$vf3,$vf3y\n\t" \
        "vmaddz.x $vf4,$vf4,$vf3z\n\tqmtc2.ni %1,$vf2\n\t" \
        "sqc2 $vf5,0(%2)\n\tnop\n\tnop\n\tvrsqrt Q,$vf2x,$vf4x\n\t" \
        "qmfc2.ni %0,$vf4\n\tsqc2 $vf4,0(%3)" : "=&r"(predicate) \
        : "r"(scalar.bits),"r"(destination),"r"(&scratch) : "memory"); \
    __asm__ volatile(".set noreorder\n\tbeqz %0,1f\n\tnop\n\t" \
        "vwaitq\n\tvmove.xyzw $vf6,$vf7\n\tvmulq.xyz $vf6,$vf6,Q\n\t" \
        "b 2f\n\tnop\n\t1:\n\tvsub.xyz $vf6,$vf0,$vf0\n\t2:\n\t.set reorder" \
        : : "r"(predicate) : "memory"); \
    __asm__ volatile("sqc2 $vf6,0(%0)" : : "r"(destination) : "memory"); \
} while(0)

void func_L01_0067F5E8(u8 *record)
{
    PositionUpdateVector scratch,temporary10,observed,planarObserved,xyObserved,ones;
    PositionUpdatePacked savedPosition,savedSecond,planarVelocity,xyVelocity,second;
    PositionUpdateScalar scalar,length,limit;
    u64 predicate;
    s32 state,secondary,secondPhase;
    f32 maximumXY,threshold3,vertical;

    savedPosition.quad=PU_QUAD(0xA0);
    maximumXY=PU_FLOAT(0x204)-0.02f;
    threshold3=PU_FLOAT(0x240)-0.0004f;
    if(D_0021E824!=1)func_L01_0068B258(record);
    state=PU_WORD(0x25CC);
    if(state==0x14||state==0x83) {
        if(PU_HALF(0x3A0)==0 && maximumXY<func_L01_0068A920(record,PU_VECTOR(0x130))) {
            func_L01_0068AC58(record,PU_VECTOR(0x130),PU_VECTOR(0x130),maximumXY);
        }
        vertical=func_L01_0068AB40(record,PU_VECTOR(0x130));
        if(vertical<0.f) {
            f32 floor=-PU_FLOAT(0x2CC);
            if(vertical<floor) {
                floor=floor-0.1f;
                __asm__ volatile("max.s %0,%1,%2\n\tmin.s %0,%0,%3"
                    : "=&f"(vertical) : "f"(vertical),"f"(floor),"f"(0.f));
                func_L01_0068ABA8(record,PU_VECTOR(0x130),PU_VECTOR(0x130),vertical);
            }
        }
        state=PU_WORD(0x25CC);
        PU_ONES("$vf1");
        if(state==0x14) {
            __asm__ volatile("lqc2 $vf1,0x130(%1)\n\tvmul.xyzw $vf1,$vf1,$vf1\n\t"
                "lqc2 $vf2,0(%2)\n\tvadday.x ACC,$vf1,$vf1y\n\t"
                "vmaddz.x $vf1,$vf2,$vf1z\n\tnop\n\tnop\n\tvsqrt Q,$vf1x\n\t"
                "sqc2 $vf1,0(%3)\n\tvwaitq\n\tvaddq.x $vf1,$vf0,Q\n\t"
                "vmaxw.xyzw $vf3,$vf0,$vf0w\n\tqmfc2.ni %0,$vf1\n\t"
                "sqc2 $vf3,0(%2)\n\tsqc2 $vf1,0(%4)" : "=&r"(length.bits)
                : "r"(record),"r"(&ones),"r"(&scratch),"r"(&temporary10) : "memory");
            if(maximumXY<length.value) {
                scalar.value=maximumXY/length.value;
                __asm__ volatile("lqc2 $vf1,0x130(%0)\n\tqmtc2.ni %1,$vf2\n\t"
                    "vmulx.xyz $vf1,$vf1,$vf2x\n\tsqc2 $vf1,0x130(%0)"
                    : : "r"(record),"r"(scalar.bits) : "memory");
            }
        }
    } else {
        secondary=PU_WORD(0x25D4);
        if(secondary==15) {
            PU_ONES("$vf3");
            if(PU_HALF(0x3A0)==0) {
                __asm__ volatile("lqc2 $vf1,0x130(%1)\n\tvmul.xyzw $vf1,$vf1,$vf1\n\t"
                    "vaddy.x $vf1,$vf1,$vf1y\n\tnop\n\tnop\n\tvsqrt Q,$vf1x\n\t"
                    "sqc2 $vf1,0(%2)\n\tvwaitq\n\tvaddq.x $vf1,$vf0,Q\n\t"
                    "qmfc2.ni %0,$vf1\n\tsqc2 $vf1,0(%3)" : "=&r"(length.bits)
                    : "r"(record),"r"(&scratch),"r"(&temporary10) : "memory");
                limit.value=PU_FLOAT(0x204)-0.02f;
                PU_ONES("$vf4");
                if(limit.value<length.value) {
                    __asm__ volatile("lqc2 $vf4,0x130(%1)\n\tqmtc2.ni %2,$vf2\n\t"
                        "vmove.xyzw $vf3,$vf4\n\tvmul.xyzw $vf1,$vf3,$vf3\n\t"
                        "vaddy.x $vf1,$vf1,$vf1y\n\tnop\n\tnop\n\t"
                        "vrsqrt Q,$vf2x,$vf1x\n\tqmfc2.ni %0,$vf1\n\tsqc2 $vf1,0(%3)"
                        : "=&r"(predicate) : "r"(record),"r"(limit.bits),"r"(&scratch) : "memory");
                    if(predicate) {
                        __asm__ volatile("vwaitq\n\tvmove.xyzw $vf3,$vf4\n\tvmulq.xy $vf3,$vf3,Q" : : : "memory");
                    } else {
                        __asm__ volatile("vsub.xy $vf3,$vf0,$vf0" : : : "memory");
                    }
                    __asm__ volatile("sqc2 $vf3,0x130(%0)" : : "r"(record) : "memory");
                    PU_ONES("$vf1");
                }
            }
        } else {
            PU_ONES("$vf2");
            if(secondary!=13 && secondary!=14 && PU_HALF(0x3A0)==0) {
                __asm__ volatile("lqc2 $vf3,0x130(%1)\n\tvmul.xyzw $vf2,$vf3,$vf3\n\t"
                    "lqc2 $vf4,0(%2)\n\tvadday.x ACC,$vf2,$vf2y\n\t"
                    "vmaddz.x $vf1,$vf4,$vf2z\n\tqmfc2.ni %0,$vf1\n\t"
                    "sqc2 $vf1,0(%3)" : "=&r"(length.bits)
                    : "r"(record),"r"(&ones),"r"(&scratch) : "memory");
                if(threshold3<length.value) {
                    scalar.value=maximumXY;
                    __asm__ volatile("vadday.x ACC,$vf2,$vf2y\n\tlqc2 $vf4,0(%2)\n\t"
                        "vmaddz.x $vf1,$vf4,$vf2z\n\tqmtc2.ni %1,$vf2\n\t"
                        "qmfc2.ni %0,$vf1\n\tnop\n\tnop\n\tvrsqrt Q,$vf2x,$vf1x\n\t"
                        "sqc2 $vf1,0(%3)\n\tvmove.xyzw $vf2,$vf3"
                        : "=&r"(predicate) : "r"(scalar.bits),"r"(&ones),"r"(&scratch) : "memory");
                    if(predicate) {
                        __asm__ volatile("vwaitq\n\tvmove.xyzw $vf2,$vf3\n\tvmulq.xyz $vf2,$vf2,Q" : : : "memory");
                    } else {
                        __asm__ volatile("vsub.xyz $vf2,$vf0,$vf0" : : : "memory");
                    }
                    __asm__ volatile("sqc2 $vf2,0x130(%0)" : : "r"(record) : "memory");
                }
            }
        }
    }

    __asm__ volatile("lqc2 $vf2,0xA0(%0)\n\tlqc2 $vf1,0x130(%0)\n\t"
        "vadd.xyz $vf2,$vf2,$vf1\n\tsqc2 $vf2,0xA0(%0)" : : "r"(record) : "memory");
    if(PU_BYTE(0x2FCB)) {
        __asm__ volatile("lqc2 $vf1,0x1E60(%0)\n\tvadd.xyz $vf2,$vf2,$vf1\n\t"
            "sq $0,0x1E60(%0)\n\tsqc2 $vf2,0xA0(%0)" : : "r"(record) : "memory");
        PU_BYTE(0x2FCB)=0;
    }
    PU_BYTE(0x226)=0;
    __asm__ volatile("lqc2 $vf1,0x140(%1)\n\tvmul.xyzw $vf1,$vf1,$vf1\n\t"
        "vadday.x ACC,$vf1,$vf1y\n\tlqc2 $vf2,0(%2)\n\t"
        "vmaddz.x $vf1,$vf2,$vf1z\n\tqmfc2.ni %0,$vf1\n\tsqc2 $vf1,0(%3)"
        : "=&r"(length.bits) : "r"(record),"r"(&ones),"r"(&scratch) : "memory");
    PU_WORD(0x20C)=0;
    secondPhase=length.value>1e-8f;
    if(!secondPhase) {
        func_L01_0067F518(record);
        func_L01_0067DDD0(record,0);
        __asm__ volatile("lqc2 $vf3,0(%1)\n\tlqc2 $vf1,0xA0(%0)\n\t"
            "vsub.xyz $vf1,$vf1,$vf3\n\tsqc2 $vf1,0x150(%0)"
            : : "r"(record),"r"(&savedPosition) : "memory");
        func_L01_0067EA88(record);
    } else {
        func_L01_0067DDD0(record,0);
    }
    __asm__ volatile("lqc2 $vf4,0(%1)\n\tlqc2 $vf1,0xA0(%0)\n\t"
        "vsub.xyz $vf1,$vf1,$vf4\n\tsqc2 $vf1,0x160(%0)\n\t"
        "sqc2 $vf1,0x180(%0)\n\tsqc2 $vf1,0x170(%0)"
        : : "r"(record),"r"(&savedPosition) : "memory");
    PU_NORMALIZE_LOCAL(observed);
    PU_PROJECT(PU_VECTOR(0x130),observed,PU_VECTOR(0x160));
    planarVelocity.quad=PU_QUAD(0x130);
    func_L01_0068A7F0(record,&planarVelocity.vector,&planarVelocity.vector);
    func_L01_0068A7F0(record,PU_VECTOR(0x180),PU_VECTOR(0x180));
    __asm__ volatile("lqc2 $vf1,0x180(%0)" : : "r"(record) : "memory");
    PU_NORMALIZE_LOCAL(planarObserved);
    PU_PROJECT(&planarVelocity.vector,planarObserved,PU_VECTOR(0x180));
    xyVelocity.quad=PU_QUAD(0x130);
    func_L01_0068AC58(record,&xyVelocity.vector,&xyVelocity.vector,0.f);
    func_L01_0068AC58(record,PU_VECTOR(0x170),PU_VECTOR(0x170),0.f);
    __asm__ volatile("lqc2 $vf1,0x170(%0)" : : "r"(record) : "memory");
    PU_NORMALIZE_LOCAL(xyObserved);
    PU_PROJECT(&xyVelocity.vector,xyObserved,PU_VECTOR(0x170));
    __asm__ volatile("lqc2 $vf1,0x160(%1)\n\tvmul.xyzw $vf1,$vf1,$vf1\n\t"
        "vadday.x ACC,$vf1,$vf1y\n\tlqc2 $vf3,0(%2)\n\t"
        "vmaddz.x $vf1,$vf3,$vf1z\n\tnop\n\tnop\n\tvsqrt Q,$vf1x\n\t"
        "sqc2 $vf1,0(%3)\n\tvwaitq\n\tvaddq.x $vf1,$vf0,Q\n\t"
        "qmfc2.ni %0,$vf1\n\tsqc2 $vf1,0(%4)" : "=&r"(length.bits)
        : "r"(record),"r"(&ones),"r"(&scratch),"r"(&temporary10) : "memory");
    *(volatile u32 *)(record+0x1B0)=length.bits;
    PU_FLOAT(0x1B4)=func_L01_0068A920(record,PU_VECTOR(0x160));
    PU_FLOAT(0x1B8)=func_L01_0067EEA0(record,PU_QUAD(0x160));
    PU_FLOAT(0x1C0)=func_L01_0068AB40(record,PU_VECTOR(0x160));
    savedSecond.quad=PU_QUAD(0xA0);
    scalar.value=PU_FLOAT(0x1B8);
    __asm__ volatile("max.s %0,%1,%2" : "=f"(scalar.value) : "f"(scalar.value),"f"(0.f));
    PU_FLOAT(0x1B8)=scalar.value;
    if(secondPhase) {
        second.quad=PU_QUAD(0x140);
        __asm__ volatile("qmtc2.ni $0,$vf2\n\tsqc2 $vf2,0x140(%0)" : : "r"(record) : "memory");
        scalar.bits=second.word[3];
        __asm__ volatile("qmtc2.ni %2,$vf3\n\tlqc2 $vf1,0(%1)\n\t"
            "vadd.xyz $vf1,$vf1,$vf3\n\tsqc2 $vf1,0xA0(%0)"
            : : "r"(record),"r"(&savedSecond),"r"(second.quad) : "memory");
        /* Original PCPYUD/PROT3W selects word3, not the Z word. */
        second.quad=0;
        second.word[3]=scalar.bits;
        __asm__ volatile("lqc2 $vf4,0(%1)\n\tvmove.w $vf2,$vf4\n\tsqc2 $vf2,0x140(%0)"
            : : "r"(record),"r"(&second) : "memory");
        func_L01_0067F518(record);
        func_L01_0067DDD0(record,0);
        __asm__ volatile("lqc2 $vf2,0(%1)\n\tlqc2 $vf1,0xA0(%0)\n\t"
            "vsub.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0x150(%0)"
            : : "r"(record),"r"(&savedPosition) : "memory");
        func_L01_0067EA88(record);
        __asm__ volatile("lqc2 $vf1,0xA0(%1)\n\tlqc2 $vf3,0(%2)\n\t"
            "vsub.xyz $vf1,$vf1,$vf3\n\tsqc2 $vf1,0x190(%1)\n\t"
            "vmul.xyzw $vf2,$vf1,$vf1\n\tvadday.x ACC,$vf2,$vf2y\n\t"
            "lqc2 $vf4,0(%3)\n\tvmaddz.x $vf2,$vf4,$vf2z\n\tnop\n\tnop\n\t"
            "vsqrt Q,$vf2x\n\tsqc2 $vf2,0(%4)\n\tvwaitq\n\t"
            "lqc2 $vf3,0x140(%1)\n\tlqc2 $vf2,0x190(%1)\n\t"
            "vaddq.x $vf1,$vf0,Q\n\tvmove.w $vf2,$vf3\n\tqmtc2.ni $0,$vf4\n\t"
            "vmove.w $vf3,$vf4\n\tqmfc2.ni %0,$vf1\n\t"
            "sqc2 $vf2,0x190(%1)\n\tsqc2 $vf3,0x140(%1)\n\tsqc2 $vf1,0(%5)"
            : "=&r"(length.bits) : "r"(record),"r"(&savedSecond),"r"(&ones),
                "r"(&scratch),"r"(&temporary10) : "memory");
        *(volatile u32 *)(record+0x1C4)=length.bits;
    } else {
        PU_FLOAT(0x1C4)=0.f;
        PU_QUAD(0x190)=0;
    }
    length.value=PU_FLOAT(0x1B4);
    __asm__ volatile("lqc2 $vf2,0(%1)\n\tlqc2 $vf1,0xA0(%0)\n\t"
        "vsub.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0x150(%0)"
        : : "r"(record),"r"(&savedPosition) : "memory");
    PU_WORD(0x1BC)=0;
    if(length.value>0.004f) {
        __asm__ volatile("qmfc2.ni %0,$vf1" : "=r"(second.quad));
        scalar.bits=second.word[2];
        scalar.value=scalar.value/length.value;
        __asm__ volatile("min.s %0,%1,%2\n\tmax.s %0,%0,%3"
            : "=&f"(scalar.value) : "f"(scalar.value),"f"(0.5f),"f"(-0.5f));
        PU_FLOAT(0x1BC)=scalar.value;
    }
    vertical=PU_FLOAT(0x25C4);
    limit.value=D_0021DDF4*52.f*vertical;
    length.value=PU_FLOAT(0x1B0);
    if(limit.value<length.value) {
        scalar.value=limit.value/length.value;
        __asm__ volatile("lqc2 $vf1,0x150(%0)\n\tqmtc2.ni %1,$vf2\n\t"
            "vmulx.xyz $vf1,$vf1,$vf2x\n\tsqc2 $vf1,0x150(%0)"
            : : "r"(record),"r"(scalar.bits) : "memory");
        PU_FLOAT(0x1B0)=D_0021DDF4*52.f*vertical;
    }
}
#endif


