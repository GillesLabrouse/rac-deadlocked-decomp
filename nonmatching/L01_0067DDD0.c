/* asflags: --float-immediates */
#include "common.h"

/* Complete actor-state update 0067DDD0..0067EA88 (0xCB8 bytes), including
 * mode/parity scheduling, ground/surface collision, recovery and two side
 * collision probes. All real calls and VU/Q operations remain dependencies.
 * This source deliberately has no native entry or successful collision stub.
 * Integer arguments are (record, force); floating arguments belong only to
 * the genuine callees indicated below. Quad-by-value calls use a real128-bit
 * a0 argument, not a pointer or four invented integer arguments.
 *
 * record has storage through2F5C and a valid actor at2EF0. Shared collision
 * packet242340 and globals are live: reloads after callbacks are preserved.
 * The two side-probe vectors are NOT initialized in the original frame;
 * 68A340 actually reads their backing before updating them. This PS2 source
 * leaves those locals uninitialized, rather than fabricating their W fields.
 * Any future native complete entry must supply real prior stack backing or
 * establish a whole-return domain that does not consume it.
 * CVT.W.S is preserved, rather than changed to a truncating C integer cast.
 */
#ifndef DEADLOCKED_NATIVE
typedef unsigned int ActorStateQuad __attribute__((mode(TI)));
typedef struct ActorStateVector { f32 x,y,z,w; }
    __attribute__((aligned(16))) ActorStateVector;
typedef union ActorStateBits { u32 bits;s32 integer;f32 value; } ActorStateBits;

extern volatile s32 D_0021E824,D_0021E878,D_0021DF60,D_0021DDB8;
extern volatile u8 D_00220647;
extern u8 D_00242340[];

s32 func_L01_00673A00(u8 *);
void func_L01_0067DBE8(u8 *,u8 *,u8 *,s32,f32,f32);
s32 func_L01_0053DC70(u8 *,u8 *,s32,u8 *,s32);
s32 func_L01_00543028(void);
s32 func_L01_00542FF8(void);
f32 func_L01_00593478(ActorStateQuad,u8 *);
s32 func_L01_00596E48(u8 *);
s32 func_L01_00595DA0(u8 *);
u8 *func_L01_00598830(u8 *);
f32 func_L01_0059F780(ActorStateQuad);
f32 func_L01_0068AFF8(u8 *,u8 *);
f32 func_L01_0055D708(f32,f32);
s32 func_L01_00686F98(u8 *,s32,s32);
f32 func_L01_00594AE8(f32 *,f32,f32);
void func_L01_006824E8(u8 *,u8 *,u8 *);
void func_L01_0068A340(u8 *,u8 *,s32,f32,f32,f32);

#define ASWORD(p,o) (*(volatile u32 *)((u8 *)(p)+(o)))
#define ASSWORD(p,o) (*(volatile s32 *)((u8 *)(p)+(o)))
#define ASHALF(p,o) (*(volatile s16 *)((u8 *)(p)+(o)))
#define ASBYTE(p,o) (*(volatile u8 *)((u8 *)(p)+(o)))
#define ASFLOAT(p,o) (*(volatile f32 *)((u8 *)(p)+(o)))
#define ASQUAD(p,o) (*(volatile ActorStateQuad *)((u8 *)(p)+(o)))
#define ASPTR(p,o) ((u8 *)ASWORD(p,o))
#define ASACTOR() ASPTR(record,0x2EF0)
#define AS_ROUND(out,operand) do { \
    ActorStateBits roundingBits; \
    __asm__ volatile("cvt.w.s %0,%1" : "=f"(roundingBits.value) : "f"(operand)); \
    (out)=roundingBits.integer; \
} while(0)

void func_L01_0067DDD0(u8 *record,s32 force)
{
    ActorStateVector temporary0,temporary10,temporary20,temporary30;
    ActorStateVector first,second,normal,sideFirst,sideSecond;
    ActorStateVector sideNormal,sideEuler,ones,zero;
    ActorStateBits scalar;
    u8 *packet=D_00242340,*vector=packet+0x20,*hitObject;
    s32 mode=D_0021E824,state,rounded,index;
    f32 height,depth,value,delta,firstHeight;
    u64 predicate;

    if(mode==1 && force==0) {
        if(ASBYTE(record,0x266D)==0 || ASFLOAT(record,0x2D4)>-0.1f ||
            D_0021DF60==30) {
            if(ASSWORD(record,0x2F10)%2==D_0021DDB8%2) {
                state=ASSWORD(record,0x25CC);
                if((state!=2 || ASFLOAT(record,0x2E08)>0.5f) && state!=3 &&
                    ASBYTE(record,0x2659)!=1) {
                    if(ASSWORD(record,0x2F5C)!=-1) return;
                    if(D_00220647==0) return;
                }
            }
        }
    }
    if(mode==0 && force==0 && ASFLOAT(record,0x2D4)>-0.1f) {
        if(ASSWORD(record,0x2F10)%2==D_0021DDB8%2) {
            state=ASSWORD(record,0x25CC);
            if((state!=2 || ASFLOAT(record,0x2E08)>0.5f) && state!=3 &&
                ASBYTE(record,0x2659)!=1 && ASWORD(record,0x2EC)==0) return;
        }
    }
    __asm__ volatile("vmr32.xyzw $vf1,$vf0\n\tsqc2 $vf1,0x250(%0)\n\t"
        "sqc2 $vf1,0(%1)" : : "r"(record),"r"(&temporary0) : "memory");
    value=(f32)ASHALF(record,0x2FE)+ASFLOAT(record,0x25C4);
    AS_ROUND(rounded,value);ASHALF(record,0x2FE)=(s16)rounded;
    value=(f32)ASHALF(record,0x2FC)+ASFLOAT(record,0x25C4);
    AS_ROUND(rounded,value);ASHALF(record,0x2FC)=(s16)rounded;
    ASFLOAT(record,0x2CC)=32.0f;
    ASWORD(record,0x2D0)=0;
    ASWORD(record,0x2C8)=0;
    ASQUAD(record,0x280)=0;
    ASHALF(record,0x2FA)=0;
    ASWORD(record,0x2EC)=0;
    if(ASSWORD(record,0x2E8)!=0) {
        value=(f32)ASSWORD(record,0x2E8)+ASFLOAT(record,0x25C4);
        AS_ROUND(rounded,value);ASSWORD(record,0x2E8)=rounded;
    }
    height=ASFLOAT(record,0x204)*2.2f;
    if(ASSWORD(record,0x25D4)==2) height=ASFLOAT(record,0x204)*2.9f;
    mode=D_0021E824;
    if(mode!=1) {
        if(func_L01_00673A00(record)!=0) height=0.0f;
        mode=D_0021E824;
    }
    depth=mode==1 ? -20.0f : -48.0f;
    func_L01_0067DBE8(record,(u8 *)&first,(u8 *)&second,1,height,depth);
    if(!func_L01_0053DC70((u8 *)&first,(u8 *)&second,2,ASACTOR(),0))
        goto ground_done;
    ASBYTE(record,0x43B)=(u8)func_L01_00543028();
    if(ASWORD(packet,0x18)!=0) ASBYTE(record,0x43B)=0;
    ASHALF(record,0x430)=(s16)func_L01_00542FF8();
    if(ASHALF(record,0x430)==0) {
        value=func_L01_00593478(ASQUAD(packet,0x20),record+0x260);
        ASFLOAT(record,0x2E0)=value;
        if(ASFLOAT(record,0xA8)<value && ASSWORD(record,0x2E8)<=0)
            ASSWORD(record,0x2E8)=1;
        if(!func_L01_0053DC70((u8 *)&first,(u8 *)&second,0x24,ASACTOR(),0))
            goto ground_done;
        ASBYTE(record,0x43B)=(u8)func_L01_00543028();
    }
    if(ASHALF(record,0x430)==13) {
        ASFLOAT(record,0x2E0)=ASFLOAT(vector,8);
        scalar.value=ASFLOAT(vector,8)-0.01f;
        __asm__ volatile("lqc2 $vf1,0(%0)\n\tqmtc2.ni %1,$vf2\n\t"
            "vaddx.z $vf1,$vf0,$vf2x\n\tsqc2 $vf1,0(%0)"
            : : "r"(&first),"r"(scalar.bits) : "memory");
        if(!func_L01_0053DC70((u8 *)&first,(u8 *)&second,4,ASACTOR(),0))
            goto ground_done;
    }
    if(ASHALF(record,0x430)==3) {
        ASFLOAT(record,0x2E4)=ASFLOAT(vector,8);
        scalar.value=ASFLOAT(vector,8)-0.01f;
        __asm__ volatile("lqc2 $vf1,0(%0)\n\tqmtc2.ni %1,$vf2\n\t"
            "vaddx.z $vf1,$vf0,$vf2x\n\tsqc2 $vf1,0(%0)"
            : : "r"(&first),"r"(scalar.bits) : "memory");
        if(!func_L01_0053DC70((u8 *)&first,(u8 *)&second,4,ASACTOR(),0))
            goto ground_done;
    }
    if(ASHALF(record,0x430)==11 ||
        (ASHALF(record,0x430)==1 && ASSWORD(record,0x25CC)==0x7B))
        goto ground_done;
    if(ASHALF(record,0x430)==4) {
        ASFLOAT(record,0x2E4)=ASFLOAT(vector,8);
        goto ground_done;
    }
    if(ASWORD(packet,0x18)!=0) {
        if(func_L01_00596E48(ASPTR(packet,0x18))) ASHALF(record,0x2FA)=1;
        if(func_L01_00595DA0(ASPTR(packet,0x18)) && (ASWORD(packet,0x10)&8))
            ASHALF(record,0x2FA)=1;
        hitObject=func_L01_00598830(ASPTR(packet,0x18));
        if(hitObject!=NULL && (ASWORD(hitObject,0x3C)&8)) ASHALF(record,0x2FA)=1;
    }
    if(ASSWORD(packet,0x1C)<=0) goto ground_done;
    ASQUAD(record,0x280)=ASQUAD(packet,0x20);
    ASFLOAT(record,0x2C8)=ASFLOAT(packet,0x28);
    if(D_0021E878!=0) ASFLOAT(record,0x2C8)=func_L01_0059F780(ASQUAD(packet,0x20));
    __asm__ volatile("lqc2 $vf3,0x280(%1)\n\tvmaxw.xyzw $vf2,$vf0,$vf0w\n\t"
        "lqc2 $vf1,0xA0(%1)\n\tvsub.xyz $vf1,$vf1,$vf3\n\t"
        "vmul.xyzw $vf1,$vf1,$vf1\n\tvadday.x ACC,$vf1,$vf1y\n\t"
        "vmaddz.x $vf2,$vf2,$vf1z\n\tnop\n\tnop\n\t"
        "vsqrt Q,$vf2x\n\tsqc2 $vf2,0(%2)\n\tvwaitq\n\t"
        "vaddq.x $vf1,$vf0,Q\n\tvmaxw.xyzw $vf2,$vf0,$vf0w\n\tqmfc2.ni %0,$vf1\n\t"
        "sqc2 $vf1,0(%3)\n\tsqc2 $vf2,0(%4)"
        : "=&r"(scalar.bits) : "r"(record),"r"(&temporary10),
          "r"(&temporary20),"r"(&ones) : "memory");
    ASFLOAT(record,0x2CC)=scalar.value;
    ASWORD(record,0x2EC)=ASWORD(packet,0x18);
    __asm__ volatile("lqc2 $vf1,0x40(%1)\n\tvmul.xyzw $vf3,$vf1,$vf1\n\t"
        "vadday.x ACC,$vf3,$vf3y\n\tvmaddz.x $vf2,$vf2,$vf3z\n\t"
        "sqc2 $vf1,0(%2)\n\tqmfc2.ni %0,$vf2\n\tsqc2 $vf2,0(%3)\n\t"
        "nop\n\tnop\n\tvrsqrt Q,$vf0w,$vf2x"
        : "=&r"(predicate) : "r"(packet),"r"(&normal),"r"(&temporary0) : "memory");
    if(predicate!=0) {
        __asm__ volatile("vwaitq\n\tlqc2 $vf1,0(%0)\n\tvmulq.xyz $vf1,$vf1,Q\n\t"
            "sqc2 $vf1,0(%0)" : : "r"(&normal) : "memory");
    } else {
        __asm__ volatile("vsub.xyz $vf1,$vf0,$vf0\n\tsqc2 $vf1,0(%0)\n\t"
            "sqc2 $vf1,0(%1)" : : "r"(&normal),"r"(&temporary10) : "memory");
    }
    ASQUAD(record,0x250)=ASQUAD(&normal,0);
    ASFLOAT(record,0x2D0)=func_L01_0068AFF8(record,packet+0x40);
    ASFLOAT(record,0x2DC)=func_L01_0055D708(ASFLOAT(packet,0x40),ASFLOAT(packet,0x44));
    __asm__ volatile("lqc2 $vf3,0xA0(%1)\n\tlqc2 $vf1,0x280(%1)\n\t"
        "lqc2 $vf2,0x250(%1)\n\tvsub.xyz $vf1,$vf1,$vf3\n\t"
        "vmul.xyzw $vf1,$vf1,$vf2\n\tvadday.x ACC,$vf1,$vf1y\n\t"
        "lqc2 $vf2,0(%2)\n\tvmaddz.x $vf1,$vf2,$vf1z\n\t"
        "qmfc2.ni %0,$vf1\n\tsqc2 $vf1,0(%3)"
        : "=&r"(scalar.bits) : "r"(record),"r"(&ones),"r"(&temporary0) : "memory");
    if(scalar.value>0.0f) ASFLOAT(record,0x2CC)=-ASFLOAT(record,0x2CC);
    if(ASFLOAT(record,0x2CC)<0.02f) {
        ASHALF(record,0x2FC)=0;
        if(ASFLOAT(record,0x2D0)<=0.872664630413055419921875f ||
            ASBYTE(record,0x2659)!=0) {
            ASHALF(record,0x2FE)=0;
            ASWORD(record,0x630)=ASWORD(packet,0x18);
        }
    }

ground_done:
    if(ASFLOAT(record,0x2E0)<ASFLOAT(record,0xA8)) ASWORD(record,0x2E8)=0;
    if(ASHALF(record,0x430)==4 && ASHALF(record,0x2FE)==0) {
        value=(f32)ASHALF(record,0x2FE)+ASFLOAT(record,0x25C4);
        AS_ROUND(rounded,value);ASHALF(record,0x2FE)=(s16)rounded;
    }
    if(ASHALF(record,0x2FE)!=0) ASWORD(record,0x2F0)=0;
    else {
        value=(f32)ASSWORD(record,0x2F0)+ASFLOAT(record,0x25C4);
        AS_ROUND(rounded,value);ASSWORD(record,0x2F0)=rounded;
        ASQUAD(record,0x290)=ASQUAD(record,0x280);
        if(!func_L01_00686F98(record,0x38,0x78)) ASWORD(record,0x1F70)=0;
        else if(ASHALF(record,0x420)!=0) ASWORD(record,0x1F70)=0;
    }
    __asm__ volatile("qmtc2.ni $0,$vf1\n\tsqc2 $vf1,0x270(%0)"
        : : "r"(record) : "memory");
    state=ASBYTE(record,0x2659);
    if(state==0) {
        scalar.bits=0xBF800000U;
        __asm__ volatile("qmtc2.ni %1,$vf2\n\tvaddx.z $vf1,$vf0,$vf2x\n\t"
            "sqc2 $vf1,0x270(%0)" : : "r"(record),"r"(scalar.bits) : "memory");
    } else if(state==1) {
        if(ASFLOAT(record,0x2AC)==0.0f) {
            scalar.bits=0xBF800000U;
            __asm__ volatile("lqc2 $vf1,0x250(%0)\n\tqmtc2.ni %1,$vf2\n\t"
                "sqc2 $vf1,0x270(%0)\n\tvmulx.xyz $vf1,$vf1,$vf2x\n\t"
                "sqc2 $vf1,0x270(%0)" : : "r"(record),"r"(scalar.bits) : "memory");
        } else {
            func_L01_00594AE8((f32 *)(record+0x2AC),0.0f,1.0f);
            scalar.bits=0xBF800000U;
            __asm__ volatile("lqc2 $vf1,0x2A0(%0)\n\tqmtc2.ni %1,$vf2\n\t"
                "sqc2 $vf1,0x270(%0)\n\tvmulx.xyz $vf1,$vf1,$vf2x\n\t"
                "sqc2 $vf1,0x250(%0)" : : "r"(record),"r"(scalar.bits) : "memory");
        }
    } else if(state==2) {
        scalar.value=ASFLOAT(record,0xDC)>0.0f ? 1.0f : -1.0f;
        __asm__ volatile("lqc2 $vf3,0xD0(%1)\n\tlqc2 $vf1,0xA0(%1)\n\t"
            "vmaxw.xyzw $vf2,$vf0,$vf0w\n\tvsub.xyz $vf3,$vf3,$vf1\n\t"
            "qmtc2.ni %2,$vf4\n\tvmove.xyzw $vf6,$vf3\n\t"
            "sqc2 $vf3,0x270(%1)\n\tvmove.xyzw $vf5,$vf6\n\t"
            "vmul.xyzw $vf1,$vf5,$vf5\n\tvadday.x ACC,$vf1,$vf1y\n\t"
            "vmaddz.x $vf2,$vf2,$vf1z\n\tnop\n\tnop\n\t"
            "vrsqrt Q,$vf4x,$vf2x\n\tqmfc2.ni %0,$vf2\n\tsqc2 $vf2,0(%3)"
            : "=&r"(predicate) : "r"(record),"r"(scalar.bits),"r"(&temporary0) : "memory");
        if(predicate!=0) {
            __asm__ volatile("vwaitq\n\tvmove.xyzw $vf5,$vf6\n\t"
                "vmulq.xyz $vf5,$vf5,Q\n\tsqc2 $vf5,0x270(%0)"
                : : "r"(record) : "memory");
        } else {
            __asm__ volatile("vsub.xyz $vf5,$vf0,$vf0\n\tsqc2 $vf5,0x270(%0)"
                : : "r"(record) : "memory");
        }
    }
    state=ASBYTE(record,0x266D);
    ASWORD(record,0x2D4)=0;ASWORD(record,0x2D8)=0;
    ASWORD(record,0x2C0)=0;ASWORD(record,0x2C4)=0;
    ASWORD(record,0x2B8)=0;ASWORD(record,0x2BC)=0;
    ASWORD(record,0x2B0)=0;ASWORD(record,0x2B4)=0;
    if(state==0 && (ASWORD(record,0x2F10)&7)!=((u32)D_0021DDB8&7)) return;
    if(!(ASFLOAT(record,0x2CC)<0.25f)) return;
    state=ASSWORD(record,0x25D4);
    if(state!=0 && state!=12 && state!=9 && state!=7 &&
        !(state==4 && ASBYTE(record,0x1D0C)!=0) && state!=1) return;
    func_L01_006824E8(record,record+0x250,(u8 *)&temporary0);
    if(temporary0.y>-0.785398185253143310546875f &&
        temporary0.y<0.785398185253143310546875f) ASFLOAT(record,0x2D4)=temporary0.y;
    if(temporary0.x>-0.785398185253143310546875f &&
        temporary0.x<0.785398185253143310546875f) ASFLOAT(record,0x2D8)=temporary0.x;
    __asm__ volatile("vmaxw.xyzw $vf1,$vf0,$vf0w\n\tvsub.xyz $vf2,$vf0,$vf0\n\t"
        "sqc2 $vf1,0(%0)\n\tsqc2 $vf2,0(%1)" : : "r"(&ones),"r"(&zero) : "memory");
    for(index=0;index<2;index++) {
        height=index==1 ? -1.57079637050628662109375f : 1.57079637050628662109375f;
        func_L01_0068A340(record,(u8 *)&sideFirst,1,0.2f,height,0.449999988079071044921875f);
        func_L01_0068A340(record,(u8 *)&sideSecond,1,0.2f,height,-0.449999988079071044921875f);
        if(func_L01_0053DC70((u8 *)&sideFirst,(u8 *)&sideSecond,0x22,ASACTOR(),0)) {
            if(D_0021E878!=0) {
                firstHeight=func_L01_0059F780(ASQUAD(packet,0x20));
                delta=firstHeight-func_L01_0059F780(ASQUAD(record,0x280));
            } else delta=ASFLOAT(vector,8)-ASFLOAT(record,0x2C8);
            ASFLOAT(record,0x2B0+index*4)=delta;
            __asm__ volatile("abs.s %0,%1" : "=f"(value) : "f"(delta));
            if(value>0.4f) ASWORD(record,0x2B0+index*4)=0;
            __asm__ volatile("lqc2 $vf1,0x40(%1)\n\tvmul.xyzw $vf2,$vf1,$vf1\n\t"
                "vadday.x ACC,$vf2,$vf2y\n\tsqc2 $vf1,0(%2)\n\t"
                "lqc2 $vf1,0(%3)\n\tvmaddz.x $vf2,$vf1,$vf2z\n\t"
                "qmfc2.ni %0,$vf2\n\tsqc2 $vf2,0(%4)\n\tnop\n\tnop\n\t"
                "vrsqrt Q,$vf0w,$vf2x" : "=&r"(predicate) : "r"(packet),
                "r"(&sideNormal),"r"(&ones),"r"(&temporary30) : "memory");
            if(predicate!=0) {
                __asm__ volatile("vwaitq\n\tlqc2 $vf1,0(%0)\n\tvmulq.xyz $vf1,$vf1,Q\n\t"
                    "sqc2 $vf1,0(%0)" : : "r"(&sideNormal) : "memory");
            } else ASQUAD(&sideNormal,0)=ASQUAD(&zero,0);
            ASQUAD(&sideEuler,0)=ASQUAD(&sideNormal,0);
            func_L01_006824E8(record,(u8 *)&sideEuler,(u8 *)&temporary10);
            if(temporary10.y>-0.785398185253143310546875f &&
                temporary10.y<0.785398185253143310546875f)
                ASFLOAT(record,0x2B8+index*4)=temporary10.y;
            if(temporary10.x>-0.785398185253143310546875f &&
                temporary10.x<0.785398185253143310546875f)
                ASFLOAT(record,0x2C0+index*4)=temporary10.x;
        } else {
            ASWORD(record,0x2B0+index*4)=0;
            ASFLOAT(record,0x2B8+index*4)=ASFLOAT(record,0x2D4);
            ASFLOAT(record,0x2C0+index*4)=ASFLOAT(record,0x2D8);
        }
    }
}
#endif
