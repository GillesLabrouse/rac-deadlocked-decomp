/* asflags: --float-immediates */
#include "common.h"

/* Complete PS2 constructor, actual L01 00668070..00669078 (0x1008 bytes).
 * All internal branches remain within that span; the single retail return
 * is at00669070. Five input arguments are consumed. The retail has26 direct
 * call sites, one virtual call through record+14 / vtable+28, and nine
 * VCALLMS C60 sites in three orientation constructions.
 *
 * This source has no native entry. Real collision, VU microcode, actor/model
 * allocation, inventory/item setup and animation dependencies remain calls.
 * In particular it does not claim that a record reset, model-pool allocation
 * or parameter initializer alone constructs a usable actor.
 *
 * Domain: record storage through31F0, aligned vectors, valid vtable+28,
 * initialized actor-pool bounds/class tables and installed VU0 programs.
 * Successful allocation is an actual precondition of the immediate actor
 * dereferences; no null guard or successful substitute was invented. If the
 * scan finds no class171C/23F7, retail continues with record+2EF0 as it is.
 * The input position/rotation are used only for a supplied actor. The null
 * actor path gets its vectors from the actual actor pool and collision data.
 *
 * Integer/float argument banks are independent in SN GCC: the declarations
 * for53EC88 and5617A8 were checked using a private compiler ABI probe. Their
 * inputs include real f12/f13 operands, not stale integer argument registers.
 * VU arithmetic below remains VU arithmetic, including ACC, finite Q behavior,
 * masked W preservation and64-bit QMFC2 branch predicates. Ordinary C sin,
 * cos or sqrt would not establish equivalent PS2 semantics.
 */
#ifndef DEADLOCKED_NATIVE
typedef struct ConstructorVector { f32 x,y,z,w; }
    __attribute__((aligned(16))) ConstructorVector;
typedef unsigned int ConstructorQuad __attribute__((mode(TI)));
typedef union ConstructorLanes {
    ConstructorQuad quad;
    f32 lane[4];
} ConstructorLanes;
typedef union ConstructorFloatBits { f32 f;u32 bits; } ConstructorFloatBits;
typedef char ConstructorVectorExtent[sizeof(ConstructorVector)==16 ? 1 : -1];
typedef char ConstructorQuadExtent[sizeof(ConstructorQuad)==16 ? 1 : -1];

extern u8 *D_00222D14 __attribute__((section(".data")));
extern u8 *D_00222D2C;
extern u16 D_002206B0;
extern volatile s32 D_0021E824,D_0021E828,D_0021E878;
extern s32 D_0021EBD8,D_0021DE10;
extern u8 D_00171B40[],D_00171DE0[];
extern ConstructorVector D_00242360,D_00242370,D_00222B60;

void func_L01_00669A20(u8 *);
s32 func_L01_00592588(u8 *,s32);
void func_L01_005A8630(u8 *,s32);
u8 *func_L01_005869E0(s32,s32);
void func_L01_005A12A8(u8 *,u8 *);
void func_L01_00575BD0(u8 *);
void func_L01_00575B88(s32);
s32 func_L01_0053EC88(ConstructorVector *,s32,u8 *,f32);
s32 func_L01_0053DC70(ConstructorVector *,ConstructorVector *,s32,u8 *,s32);
void func_L01_00667050(u8 *,u8 *);
void func_L01_0067DDD0(u8 *,s32);
void func_L01_0066E580(u8 *);
void func_L01_00688FE0(u8 *,s32,s32);
void func_L01_00688578(u8 *,s32);
s32 func_L01_0058FF38(s32,s32);
void func_L01_005617A8(u8 *,ConstructorVector *,u8 *,f32,f32);
void func_L01_006909C8(u8 *);
void func_L01_00690B60(u8 *);
void func_L01_00690C88(u8 *);
/* Address-only callback installation below; callback ABI is not recovered. */
void func_L01_0068FE70(void);

/* Volatile serialized accesses preserve the reloads around callbacks and
 * the original pool/list aliases. Quad copies retain all four raw lanes. */
#define CWORD(p,o) (*(volatile u32 *)((p)+(o)))
#define CHALF(p,o) (*(volatile u16 *)((p)+(o)))
#define CSIGNED_HALF(p,o) (*(volatile s16 *)((p)+(o)))
#define CBYTE(p,o) (*(volatile u8 *)((p)+(o)))
#define CFLOAT(p,o) (*(volatile f32 *)((p)+(o)))
#define CPTR(p,o) ((u8 *)CWORD(p,o))
#define CACTOR() CPTR(record,0x2EF0)
#define COPY_QUAD(dst,src) (*(ConstructorQuad *)(dst)=*(ConstructorQuad *)(src))

/* VCALLMS C60 consumes vf15.x and returns the lanes actually selected by
 * QMFC2/PROT3W: lane0 and lane2. Other returned lanes are not fabricated.
 * The installed microprogram is intentionally still required. */
#define CONSTRUCTOR_TRIG(angle,sine,cosine) do { \
    ConstructorFloatBits input;ConstructorLanes result; \
    input.f=(angle); \
    __asm__ volatile("qmtc2.ni %1,$vf15\n\tvcallms 0xC60\n\tqmfc2.i %0,$vf15" \
        : "=r"(result.quad) : "r"(input.bits) : "memory"); \
    (sine)=result.lane[0];(cosine)=result.lane[2]; \
} while(0)

#define CONSTRUCTOR_BASIS(actor) do { \
    f32 sx,cx,sy,cy,sz,cz,xy,yx,yz,xz;u32 scale=0xC0400000U; \
    __asm__ volatile("qmtc2.ni %0,$vf2" : : "r"(scale) : "memory"); \
    CONSTRUCTOR_TRIG(CFLOAT(actor,0xF0),sx,cx); \
    CONSTRUCTOR_TRIG(CFLOAT(actor,0xF4),sy,cy); \
    xy=sx*sy;yx=cx*sy;yz=cy*sx;xz=cx*cy; \
    CONSTRUCTOR_TRIG(CFLOAT(actor,0xF8),sz,cz); \
    CFLOAT(actor,0xD0)=cz*xy-cx*sz; \
    CFLOAT(actor,0xD4)=cx*cz+xy*sz; \
    CFLOAT(actor,0xD8)=yz;CWORD(actor,0xDC)=0; \
    CFLOAT(actor,0xE0)=cz*yx+sx*sz; \
    CFLOAT(actor,0xE4)=(-cz)*sx+yx*sz; \
    CFLOAT(actor,0xE8)=xz;CWORD(actor,0xEC)=0; \
    CFLOAT(actor,0xC0)=cy*cz;CFLOAT(actor,0xC4)=cy*sz; \
    CFLOAT(actor,0xC8)=-sy;CWORD(actor,0xCC)=0; \
} while(0)

/* First vector is E0 scaled by Q=-3/sqrt(dot(E0,E0)). Second is the
 * unit vector produced by the separate Q calculation. Both apparent zero
 * branches test the low64 bits after QMFC2, not a C float dot==0 test.
 * The zero paths retain the actual masked W lanes and the scratch store. */
#define CONSTRUCTOR_DIRECTIONS(input,ones,scaled,unit,scratch) do { \
    volatile u64 observed; \
    __asm__ volatile( \
        "lqc2 $vf4,0(%1)\n\tlqc2 $vf5,0(%2)\n\t" \
        "vmove.xyzw $vf3,$vf4\n\tvmul.xyzw $vf1,$vf3,$vf3\n\t" \
        "vadday.x ACC,$vf1,$vf1y\n\tvmaddz.x $vf1,$vf5,$vf1z\n\t" \
        "nop\n\tnop\n\tvrsqrt Q,$vf2x,$vf1x\n\tqmfc2.ni %0,$vf1\n\t" \
        "sqc2 $vf1,0(%3)" \
        : "=&r"(observed) : "r"(input),"r"(ones),"r"(scratch) : "memory"); \
    if(observed!=0) { \
        __asm__ volatile("vwaitq\n\tvmove.xyzw $vf3,$vf4\n\t" \
            "vmulq.xyz $vf3,$vf3,Q\n\tsqc2 $vf3,0(%0)" \
            : : "r"(scaled) : "memory"); \
    } else { \
        __asm__ volatile("vsub.xyz $vf3,$vf0,$vf0\n\tsqc2 $vf3,0(%0)" \
            : : "r"(scaled) : "memory"); \
    } \
    __asm__ volatile( \
        "lqc2 $vf1,0(%1)\n\tvmul.xyzw $vf2,$vf1,$vf1\n\t" \
        "vadday.x ACC,$vf2,$vf2y\n\tsqc2 $vf1,0(%2)\n\t" \
        "lqc2 $vf1,0(%3)\n\tvmaddz.x $vf2,$vf1,$vf2z\n\t" \
        "qmfc2.ni %0,$vf2\n\tsqc2 $vf2,0(%4)\n\tnop\n\tnop\n\t" \
        "vrsqrt Q,$vf0w,$vf2x" \
        : "=&r"(observed) : "r"(input),"r"(unit),"r"(ones),"r"(scratch) : "memory"); \
    if(observed!=0) { \
        __asm__ volatile("vwaitq\n\tlqc2 $vf1,0(%0)\n\t" \
            "vmulq.xyz $vf1,$vf1,Q\n\tsqc2 $vf1,0(%0)" \
            : : "r"(unit) : "memory"); \
    } else { \
        __asm__ volatile("vsub.xyz $vf1,$vf0,$vf0\n\t" \
            "sqc2 $vf1,0(%0)\n\tsqc2 $vf1,0(%1)" \
            : : "r"(unit),"r"(scratch) : "memory"); \
    } \
} while(0)

#define CONSTRUCTOR_ACTOR_ENDPOINTS(start,end,unit,scaled,position) \
    __asm__ volatile("lqc2 $vf3,0(%2)\n\tlqc2 $vf2,0(%3)\n\t" \
        "sqc2 $vf3,0(%0)\n\tlqc2 $vf1,0(%4)\n\t" \
        "vadd.xyz $vf3,$vf3,$vf1\n\tsqc2 $vf3,0(%0)\n\t" \
        "lqc2 $vf1,0(%4)\n\tvadd.xyz $vf2,$vf2,$vf1\n\tsqc2 $vf2,0(%1)" \
        : : "r"(start),"r"(end),"r"(unit),"r"(scaled),"r"(position) : "memory")

#define CONSTRUCTOR_RECORD_ENDPOINTS(start,end,unit,scaled,position) \
    __asm__ volatile("lqc2 $vf1,0(%2)\n\tlqc2 $vf3,0(%4)\n\t" \
        "lqc2 $vf2,0(%3)\n\tsqc2 $vf1,0(%0)\n\t" \
        "vadd.xyz $vf2,$vf2,$vf3\n\tvadd.xyz $vf1,$vf1,$vf3\n\t" \
        "sqc2 $vf1,0(%0)\n\tsqc2 $vf2,0(%1)" \
        : : "r"(start),"r"(end),"r"(unit),"r"(scaled),"r"(position) : "memory")

void func_L01_00668070(u8 *record,u8 *suppliedActor,
                      ConstructorVector *position,ConstructorVector *rotation,s32 index) {
    volatile u32 savedCounts[6];
    ConstructorVector ones,scratch,scaled,unit,start,end,candidate,right;
    u8 *actor,*scan,*scanEnd,*reference;
    u32 scanOffset;
    s32 i,kind,attempts;
    __asm__ volatile("vmaxw.xyzw $vf1,$vf0,$vf0w\n\tsqc2 $vf1,0(%0)"
                     : : "r"(&ones) : "memory");
    for(i=0;i<6;i++) savedCounts[i]=CWORD(record,0x2318+i*0x50);
    ((void (*)(u8 *))CWORD(CPTR(record,0x14),0x28))(record);
    CWORD(record,0x2F10)=(u32)index;
    func_L01_00669A20(record);
    for(i=0;i<6;i++) savedCounts[i]=CWORD(record,0x2318+i*0x50);
    if(D_0021E878!=0) {
        CBYTE(record,0x2659)=2;
        COPY_QUAD(record+0xD0,&D_00222B60);
    }
    if(suppliedActor!=NULL) {
        CWORD(record,0x2EF0)=(u32)suppliedActor;
        COPY_QUAD(suppliedActor+0x10,position);
        COPY_QUAD(CACTOR()+0xF0,rotation);
        COPY_QUAD(record+0xA0,position);
        COPY_QUAD(record+0xB0,rotation);
    } else {
        scan=D_00222D14;
        scanEnd=D_00222D2C;
        scanOffset=0;
        while((u32)scan<(u32)scanEnd) {
            kind=CSIGNED_HALF(scan,0xBC);
            if(kind==0x171C || kind==0x23F7) {
                if(index==0) {
                    if(kind==0x171C) {
                        func_L01_00592588(scan,0x23F7);
                        actor=D_00222D14+scanOffset;
                    } else actor=scan;
                    CWORD(record,0x2EF0)=(u32)actor;
                    CHALF(actor,0x74)|=D_002206B0;
                    CWORD(record,0x2F14)=1;
                    func_L01_005A8630(CACTOR(),1);
                } else {
                    actor=func_L01_005869E0(0x23F7,0xF0);
                    func_L01_005A12A8(actor,NULL);
                    CWORD(record,0x2EF0)=(u32)actor;
                    CHALF(actor,0x74)|=D_002206B0;
                    func_L01_00575BD0(actor);
                    func_L01_00575B88(CSIGNED_HALF(D_00171B40,0x34));
                    if(kind==0x23F7 && D_0021EBD8==2 && D_0021DE10==0x1F) {
                        ConstructorVector fixed;
                        actor=CACTOR();
                        /* Retail VMOVE.xyz preserves the actor position W. */
                        fixed.x=544.f;fixed.y=545.f;fixed.z=100.f;fixed.w=0.f;
                        __asm__ volatile("lqc2 $vf1,0(%0)\n\tlqc2 $vf2,0(%1)\n\t"
                            "vmove.xyz $vf1,$vf2\n\tsqc2 $vf1,0(%0)"
                            : : "r"(actor+0x10),"r"(&fixed) : "memory");
                        CFLOAT(CACTOR(),0xF8)=2.599999904632568359375f;
                        COPY_QUAD(record+0xA0,CACTOR()+0x10);
                    } else {
                        ConstructorFloatBits spread,offset;
                        s32 collision;
                        f32 distance=(f32)index*0.449999988079071044921875f;
                        distance=distance+distance;
                        spread.f=distance;offset.bits=0x3EF1EB84;
                        reference=D_00222D14+scanOffset;
                        __asm__ volatile("lqc2 $vf2,0(%1)\n\tlqc2 $vf1,0(%2)\n\t"
                            "qmtc2.ni %3,$vf3\n\tvmulx.xyz $vf2,$vf2,$vf3x\n\t"
                            "vadd.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0(%0)"
                            : : "r"(&candidate),"r"(reference+0xD0),"r"(reference+0x10),"r"(spread.bits) : "memory");
                        COPY_QUAD(CACTOR()+0xF0,reference+0xF0);
                        reference=D_00222D14+scanOffset;
                        __asm__ volatile("lqc2 $vf1,0(%0)\n\tlqc2 $vf3,0(%2)\n\t"
                            "qmtc2.ni %3,$vf5\n\tvmulx.xyz $vf3,$vf3,$vf5x\n\t"
                            "vadd.xyz $vf1,$vf1,$vf3\n\tsqc2 $vf3,0(%1)\n\tsqc2 $vf1,0(%0)"
                            : : "r"(&candidate),"r"(&right),"r"(reference+0xE0),"r"(offset.bits) : "memory");
                        attempts=0;
                        for(;;) {
                            /* Retail preserves vf3 across the real collision
                             * callee through its stack140 vector each time. */
                            __asm__ volatile("sqc2 $vf3,0(%0)" : : "r"(&right) : "memory");
                            collision=func_L01_0053EC88(&candidate,0x24,CACTOR(),
                                      0.449999988079071044921875f);
                            __asm__ volatile("lqc2 $vf3,0(%0)" : : "r"(&right) : "memory");
                            if(collision==0) break;
                            __asm__ volatile("lqc2 $vf2,0(%0)" : : "r"(&candidate) : "memory");
                            if(attempts>=4) break;
                            attempts++;
                            __asm__ volatile("lqc2 $vf1,0(%1)\n\t"
                                "vsub.xyz $vf1,$vf1,$vf2\n\tvadd.xyz $vf2,$vf2,$vf1\n\t"
                                "sqc2 $vf2,0(%0)" : : "r"(&candidate),"r"(&D_00242370) : "memory");
                        }
                        __asm__ volatile("lqc2 $vf1,0(%0)\n\tlqc2 $vf3,0(%1)\n\t"
                            "vsub.xyz $vf1,$vf1,$vf3\n\tsqc2 $vf1,0(%0)\n\tsqc2 $vf1,0(%2)"
                            : : "r"(&candidate),"r"(&right),"r"(CACTOR()+0x10) : "memory");
                        COPY_QUAD(record+0xA0,CACTOR()+0x10);
                    }
                }
                actor=CACTOR();
                CONSTRUCTOR_BASIS(actor);
                COPY_QUAD(record+0xB0,CACTOR()+0xF0);
                CONSTRUCTOR_DIRECTIONS(CACTOR()+0xE0,&ones,&scaled,&unit,&scratch);
                CONSTRUCTOR_ACTOR_ENDPOINTS(&start,&end,&unit,&scaled,CACTOR()+0x10);
                if(func_L01_0053DC70(&start,&end,2,CACTOR(),0)!=0)
                    COPY_QUAD(CACTOR()+0x10,&D_00242360);
                COPY_QUAD(record+0xA0,CACTOR()+0x10);
                break;
            }
            scan+=0x100;
            scanOffset+=0x100;
        }
    }
    CHALF(CACTOR(),0x34)|=8;
    func_L01_00667050(record,CACTOR());
    actor=CACTOR();
    CONSTRUCTOR_BASIS(actor);
    CONSTRUCTOR_DIRECTIONS(CACTOR()+0xE0,&ones,&scaled,&unit,&scratch);
    CONSTRUCTOR_RECORD_ENDPOINTS(&start,&end,&unit,&scaled,record+0xA0);
    if(func_L01_0053DC70(&start,&end,2,CACTOR(),0)!=0)
        COPY_QUAD(record+0xA0,&D_00242360);
    CHALF(CACTOR(),0x34)|=2;
    actor=CACTOR();
    CFLOAT(actor,0x70)=CFLOAT(CPTR(actor,0x24),0x24)*1.2000000476837158203125f;
    CWORD(record,0x634)=(u32)CACTOR();
    func_L01_0067DDD0(record,1);
    func_L01_0066E580(record);
    func_L01_00688FE0(record,0,0);
    CBYTE(record,0x2ED5)=255;
    for(i=1;i>=0;i--) CHALF(record,0x26E8+i*2)=0xFFFF;
    if(D_0021E824==0 && D_0021E828==1 && CWORD(record,0x2F10)==(u32)D_0021E828) {
        u8 selected=CBYTE(D_00171B40,0x27);
        if(selected!=0) {
            u8 same=(u8)(selected==CBYTE(D_00171B40,0x28));
            CBYTE(D_00171B40,0x28)=same;
            if(same!=0) CBYTE(D_00171B40,0x28)=(u8)(selected==2 ? 3 : 2);
        }
    }
    if(D_0021E824!=1) {
        u8 selected=CBYTE(D_00171B40,0x27+CWORD(record,0x2F10));
        if(selected!=0) func_L01_00688578(record,selected);
    }
    for(i=8;i>=0;i--) CWORD(record,0x2D30+i*4)=0xFFFFFFFFU;
    CFLOAT(record,0x2E24)=2.125f;
    CFLOAT(record,0x2E28)=1.25f;
    CWORD(record,0x1870)=(u32)func_L01_0058FF38(0xB4,0x12C);
    CWORD(record,0x2E58)=0;
    CFLOAT(record,0x2E48)=10.f;
    CFLOAT(record,0x1874)=0.007000000216066837310791015625f;
    CFLOAT(record,0x1878)=0.300000011920928955078125f;
    CFLOAT(record,0x1480)=1.15999996662139892578125f;
    CFLOAT(record,0x1484)=CFLOAT(record,0xA8);
    CFLOAT(record,0x1488)=0.119999997317790985107421875f;
    CFLOAT(record,0x2E4C)=13.f;
    CWORD(record,0x2E60)=11;
    func_L01_005617A8(record+0x1970,(ConstructorVector *)(record+0xA0),
                    CPTR(record,0x634),CFLOAT(record,0xB8),0.f);
    CFLOAT(record,0x1AFC)=1.14999997615814208984375f;
    if(CBYTE(record,0x266D)!=0) {
        CWORD(record,0x1A58)=(u32)(record+0x18D0);
        CWORD(record,0x1A5C)=(u32)(record+0x1900);
    } else {
        CWORD(record,0x1A58)=(u32)(record+0x20);
        CWORD(record,0x1A5C)=(u32)(record+0x60);
    }
    if(D_0021E824==1 || D_00171DE0[CWORD(record,0x2F10)]==0)
        CHALF(record,0x1A50)|=1;
    CWORD(record,0x1A54)=(u32)func_L01_0068FE70;
    CWORD(record,0x1A9C)=CWORD(record,0x2F10);
    CWORD(record,0x1AA8)=CWORD(record,0x2F10);
    CWORD(record,0x1AA0)=(u32)record;
    if(D_00171DE0[CWORD(record,0x2F10)]==1) {
        u32 negativeSix=0xC0C00000U,zero=0;
        if(D_0021DE10==1) {
            __asm__ volatile("lqc2 $vf1,0(%0)\n\tqmtc2.ni %1,$vf2\n\t"
                "vaddx.x $vf1,$vf0,$vf2x\n\tsqc2 $vf1,0(%0)"
                : : "r"(record+0x1A30),"r"(negativeSix) : "memory");
        } else {
            __asm__ volatile("lqc2 $vf1,0(%0)\n\tqmtc2.ni %1,$vf3\n\t"
                "vaddx.x $vf1,$vf0,$vf3x\n\tsqc2 $vf1,0(%0)"
                : : "r"(record+0x1A30),"r"(negativeSix) : "memory");
        }
        CFLOAT(record,0x1A94)=6.f;
        __asm__ volatile("lqc2 $vf1,0(%0)\n\tqmtc2.ni %1,$vf5\n\t"
            "vaddx.z $vf1,$vf0,$vf5x\n\tsqc2 $vf1,0(%0)"
            : : "r"(record+0x1A30),"r"(zero) : "memory");
        CHALF(record,0x1A50)&=0xFFFE;
    }
    CBYTE(record,0x2E84)=255;
    if(D_0021E878!=0) COPY_QUAD(record+0xD0,&D_00222B60);
    func_L01_006909C8(record);
    CFLOAT(record,0x2FC0)=1.f;
    func_L01_00690B60(record);
    func_L01_00690C88(record);
    COPY_QUAD(record+0x2F80,record+0xB0);
    CHALF(record,0x2E2C)=1;
    CWORD(record,0x2D78)=1;
    CWORD(record,0x2D88)=2;
    CFLOAT(record,0x2D8C)=10.f;CFLOAT(record,0x2D90)=5.5f;
    CWORD(record,0x2D98)=4;CWORD(record,0x2DA8)=0x67;
    CFLOAT(record,0x2DAC)=8.f;CFLOAT(record,0x2DB0)=5.f;
    COPY_QUAD(record+0x2F70,record+0xA0);
    CFLOAT(record,0x2D7C)=10.f;CFLOAT(record,0x2D80)=5.5f;
    CFLOAT(record,0x2D9C)=8.f;CFLOAT(record,0x2DA0)=5.f;
}
#endif
