/* cflags: -ffast-math -mno-split-addresses */
/* asflags: --float-immediates */
/* Complete original 0067EF00..0067F518 (1560 bytes), NONMATCHING.
 * All true capsule, sphere, supplementary-static and recovery dependencies
 * remain. Mode-zero capsule selection is based on real basis/direction state.
 * The real returned -1 is distinct from an adapter refusing an unavailable
 * dependency. VF25..29 W persist across publication-mode capsule/static calls.
 * Static collection mode is excluded here: the genuine caller passes a1=0. */
#include "common.h"
#ifndef DEADLOCKED_NATIVE
typedef unsigned int CapsuleResolveQuad __attribute__((mode(TI)));
typedef struct CapsuleResolveVector {f32 x,y,z,w;}
    __attribute__((aligned(16))) CapsuleResolveVector;
extern volatile s32 D_0021DE10,D_0021EBD8,D_0021DDB4,D_0021E878;
extern volatile f32 D_0021DDE8,D_00220744;
extern u8 D_00242340[];
void func_L01_0068BB60(u8 *);
void func_L01_0068A638(u8 *,CapsuleResolveVector *,const CapsuleResolveVector *,f32);
s32 func_L01_0053EC88(CapsuleResolveVector *,s32,u8 *,f32);
s32 func_L01_0053FAA8(CapsuleResolveVector *,s32,u8 *,f32,f32);
s32 func_L01_00542280(CapsuleResolveVector *,u8 *,f32);
#define CR_WORD(o) (*(volatile s32 *)(record+(o)))
#define CR_HALF(o) (*(volatile s16 *)(record+(o)))
#define CR_BYTE(o) (*(volatile u8 *)(record+(o)))
#define CR_FLOAT(o) (*(volatile f32 *)(record+(o)))
#define CR_QUAD(p) (*(volatile CapsuleResolveQuad *)(p))
static f32 capsule_resolve_dot(const void *a,const void *b) {
    union {u32 bits;f32 value;}scalar;
    __asm__ volatile("lqc2 $vf3,0(%1)\n\tlqc2 $vf1,0(%2)\n\t"
        "vmaxw.xyzw $vf2,$vf0,$vf0w\n\tvmul.xyzw $vf1,$vf1,$vf3\n\t"
        "vadday.x ACC,$vf1,$vf1y\n\tvmaddz.x $vf2,$vf2,$vf1z\n\t"
        "qmfc2.ni %0,$vf2" : "=r"(scalar.bits) : "r"(a),"r"(b) : "memory");
    return scalar.value;
}
static void capsule_resolve_transform(const void *matrix,const CapsuleResolveVector *input,CapsuleResolveVector *out) {
    __asm__ volatile("lqc2 $vf4,0(%1)\n\tlqc2 $vf1,0(%2)\n\t"
        "vmulax.xyzw ACC,$vf1,$vf4x\n\tlqc2 $vf2,16(%2)\n\t"
        "vmadday.xyzw ACC,$vf2,$vf4y\n\tlqc2 $vf3,32(%2)\n\t"
        "vmaddaz.xyzw ACC,$vf3,$vf4z\n\tlqc2 $vf1,48(%2)\n\t"
        "vmaddw.xyzw $vf1,$vf1,$vf4w\n\tsqc2 $vf1,0(%0)"
        : : "r"(out),"r"(input),"r"(matrix) : "memory");
}
static f32 capsule_resolve_displacement(const CapsuleResolveVector *a,const CapsuleResolveVector *b,
                                         CapsuleResolveVector *difference) {
    union {u32 bits;f32 value;}scalar;
    __asm__ volatile("lqc2 $vf2,0(%1)\n\tlqc2 $vf1,0(%2)\n\t"
        "vsub.xyz $vf3,$vf2,$vf1\n\tsqc2 $vf3,0(%3)\n\t"
        "vmul.xyzw $vf1,$vf3,$vf3\n\tvmaxw.xyzw $vf2,$vf0,$vf0w\n\t"
        "vadday.x ACC,$vf1,$vf1y\n\tvmaddz.x $vf1,$vf2,$vf1z\n\t"
        "nop\n\tnop\n\tvsqrt Q,$vf1x\n\tvwaitq\n\t"
        "vaddq.x $vf1,$vf0,Q\n\tqmfc2.ni %0,$vf1"
        : "=r"(scalar.bits) : "r"(a),"r"(b),"r"(difference) : "memory");
    return scalar.value;
}
s32 func_L01_0067EF00(u8 *record,s32 attempt) {
    CapsuleResolveVector saved,offset={0.f,0.f,0.f,0.f},difference,limited;
    s32 special,previous,mode,state,flags,limit,contacts=0,hit;
    u8 *body,*related;
    s32 priorRelated;
    u64 observed;
    union {f32 value;u32 bits;}radiusTransfer;
    f32 distance,radius;
    if(D_0021DE10==31 && D_0021EBD8==0 && D_0021DDB4==2)return 1;
    if(CR_HALF(0x3A0)!=0)return 1;
    CR_QUAD(&saved)=CR_QUAD(record+0xA0);
    if(CR_HALF(0x2E18)!=0) {func_L01_0068BB60(record);CR_HALF(0x2E18)=0;}
    special=capsule_resolve_dot(record+0x270,record+0x40)>-0.920000016689300537109375f;
    previous=CR_WORD(0x25D4);
    if(previous==15) {
        offset.z=0.800000011920928955078125f;
        body=(u8 *)CR_WORD(0x2EF0);
        capsule_resolve_transform(body+0xC0,&offset,&offset);
    }else if(previous!=17) {
        mode=CR_BYTE(0x2659);
        if(mode==0 && CR_HALF(0x3D6)==0 && !special)offset.z=CR_FLOAT(0x1F4);
        else if(mode==1 || CR_HALF(0x3D6)!=0) {
            offset.z=0.60000002384185791015625f;
            capsule_resolve_transform(record+0x20,&offset,&offset);
        }else if(D_0021E878!=0)func_L01_0068A638(record,&offset,&offset,0.769999980926513671875f);
        else {
            offset.z=CR_FLOAT(0x1F4);offset.w=D_00220744;
            capsule_resolve_transform(record+0x20,&offset,&offset);
        }
    }
    __asm__ volatile("lqc2 $vf1,0xA0(%0)\n\tlqc2 $vf2,0(%1)\n\t"
        "vadd.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0xA0(%0)"
        : : "r"(record),"r"(&offset) : "memory");
    state=CR_WORD(0x25CC);flags=0x24;
    if(state==126 || (state==153 && CR_WORD(0x25D8)==126))flags=0xD24;
    if(state==104 || state==122)flags=0x324;
    limit=CR_BYTE(0x266D)!=0?8:1;
    for(;;) {
        previous=CR_WORD(0x25D4);
        if(previous==17)hit=func_L01_0053EC88((CapsuleResolveVector *)(record+0xA0),flags,(u8 *)CR_WORD(0x2EF0),0.60000002384185791015625f);
        else if(previous==15)hit=func_L01_0053EC88((CapsuleResolveVector *)(record+0xA0),flags,(u8 *)CR_WORD(0x2EF0),
            (D_0021DDE8*0.449999988079071044921875f)*CR_FLOAT(0x25C4));
        else if(CR_BYTE(0x2659)!=0 || special) {
            hit=func_L01_0053EC88((CapsuleResolveVector *)(record+0xA0),flags,(u8 *)CR_WORD(0x2EF0),CR_FLOAT(0x204));
            if(CR_HALF(0x39E)==0)hit|=func_L01_00542280((CapsuleResolveVector *)(record+0xA0),NULL,CR_FLOAT(0x204));
        }else {
            f32 height=CR_FLOAT(0x1F0)-CR_FLOAT(0x1F4);
            f32 minimum=0.0500000007450580596923828125f;
            __asm__ volatile("max.s %0,%1,%2":"=f"(height):"f"(height),"f"(minimum));
            related=NULL;priorRelated=0;
            if(CR_WORD(0x2314)==2) {
                u8 *item=(u8 *)CR_WORD(0x22F0);
                if(item!=NULL && *(volatile s16 *)(item+0xBC)==0x2094)related=(u8 *)**(volatile u32 **)(item+0xAC);
            }
            if(related!=NULL) {priorRelated=*(volatile s32 *)(related+0x98)!=0;*(volatile s32 *)(related+0x98)=0;}
            hit=func_L01_0053FAA8((CapsuleResolveVector *)(record+0xA0),flags,(u8 *)CR_WORD(0x2EF0),CR_FLOAT(0x204),height);
            if(CR_HALF(0x39E)==0)hit|=func_L01_00542280((CapsuleResolveVector *)(record+0xA0),NULL,CR_FLOAT(0x204));
            if(related!=NULL && priorRelated)*(volatile u32 *)(related+0x98)=*(volatile u32 *)(*(volatile u32 *)(related+0x24)+0x10);
        }
        if(!hit)break;
        CR_QUAD(record+0xA0)=CR_QUAD(D_00242340+0x30);
        CR_QUAD(record+0x1D0)=CR_QUAD(D_00242340+0x40);
        CR_BYTE(0x226)=1;
        CR_QUAD(record+0x1E0)=CR_QUAD(D_00242340+0x20);
        CR_WORD(0x20C)=*(volatile s32 *)(D_00242340+0x18);
        if(++contacts>=limit)break;
    }
    __asm__ volatile("lqc2 $vf2,0xA0(%0)\n\tlqc2 $vf1,0(%1)\n\t"
        "vsub.xyz $vf2,$vf2,$vf1\n\tsqc2 $vf2,0xA0(%0)"
        : : "r"(record),"r"(&offset) : "memory");
    distance=capsule_resolve_displacement((CapsuleResolveVector *)(record+0xA0),&saved,&difference);
    radius=CR_FLOAT(0x204);
    if(!(radius+radius<distance))return 1;
    if(attempt!=15)return -1;
    radiusTransfer.value=radius;
    __asm__ volatile("lqc2 $vf3,0(%1)\n\tqmtc2.ni %2,$vf1\n\t"
        "vmaxx.xyz $vf3,$vf3,$vf1x\n\tqmtc2.ni %3,$vf1\n\t"
        "vminix.xyz $vf3,$vf3,$vf1x\n\tvmul.xyzw $vf1,$vf3,$vf3\n\t"
        "vmaxw.xyzw $vf5,$vf0,$vf0w\n\tvadday.x ACC,$vf1,$vf1y\n\t"
        "vmaddz.x $vf1,$vf5,$vf1z\n\tqmtc2.ni %4,$vf2\n\t"
        "nop\n\tnop\n\tvrsqrt Q,$vf2x,$vf1x\n\t"
        "sqc2 $vf3,0(%5)\n\tqmfc2.ni %0,$vf1"
        : "=r"(observed) : "r"(&difference),"r"(0xC4000000U),
          "r"(0x44000000U),"r"(radiusTransfer.bits),"r"(&limited) : "memory");
    if(observed!=0) {
        __asm__ volatile("vwaitq\n\tlqc2 $vf1,0(%0)\n\tvmulq.xyz $vf1,$vf1,Q\n\t"
            "sqc2 $vf1,0(%0)" : : "r"(&limited) : "memory");
    }else limited.x=limited.y=limited.z=0.f;
    __asm__ volatile("lqc2 $vf2,0(%1)\n\tlqc2 $vf1,0(%2)\n\t"
        "vadd.xyz $vf2,$vf2,$vf1\n\tsqc2 $vf2,0xA0(%0)"
        : : "r"(record),"r"(&saved),"r"(&limited) : "memory");
    return -1;
}
#undef CR_WORD
#undef CR_HALF
#undef CR_BYTE
#undef CR_FLOAT
#undef CR_QUAD
#endif
