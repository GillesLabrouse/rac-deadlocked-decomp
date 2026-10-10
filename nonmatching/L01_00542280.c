#include "common.h"

/* Complete CPU supplementary sphere contact00542280..00542808 (0x588).
 * NONMATCHING: register allocation, decomposed VU operations, instruction
 * scheduling and helper calls differ. This is an authored algorithm recovery,
 * not a byte match. The original has no JAL and no DMA transfer. Its four
 * genuine bank-save/restore micro calls remain, with actual VU arithmetic.
 * a0 is a16-byte center; a1=0 publishes the live collision packet. Nonzero
 * a1 is the END of a4096-byte reverse contact buffer, not a boolean flag.
 * f12 is radius.00222BE0 is the relocated static table published by577C70;
 * Dreadzone has12 groups/250 faces/275 vertices, independently read from ISO.
 * All source preloads (including odd vertex pairs and next faces), unsigned
 * halfword expansion, packed clipping, nearest shrink, backward collection,
 * packet W preservation and untouched no-hit fields are retained.
 *
 * Valid PS2 backing, natural alignment and installed E90/E10/F00/F40 are
 * required. There is no native success substitute in this source. The separate
 * native adapter declares its bounded host-IEEE approximation explicitly.
 */
#ifndef DEADLOCKED_NATIVE
typedef unsigned int BodyQuad __attribute__((mode(TI)));
typedef union BodyFloat {u32 bits;f32 value;} BodyFloat;
typedef union BodyVector {
    BodyQuad quad;
    u32 word[4];
    f32 value[4];
} __attribute__((aligned(16))) BodyVector;
extern u8 *D_00222BE0;
extern u8 D_00242340[] __attribute__((aligned(16)));
typedef char BodyVectorWidth[sizeof(BodyVector)==16?1:-1];

static f32 body_value(u32 bits) {BodyFloat v;v.bits=bits;return v.value;}
static u32 body_word(f32 value) {BodyFloat v;v.value=value;return v.bits;}
/* Real signed SUB overflow semantics cannot be expressed by overflowing C. */
static s32 body_checked_sub(s32 a,s32 b) {
    s32 out;__asm__ volatile("sub %0,%1,%2":"=r"(out):"r"(a),"r"(b));return out;
}
#define BODY_VU_SCALAR(name,instruction) \
static f32 name(f32 a,f32 b) { \
    u32 result,first=body_word(a),second=body_word(b); \
    __asm__ volatile("qmtc2.ni %1,$vf1\n\tqmtc2.ni %2,$vf2\n\t" \
                     instruction "\n\tqmfc2.ni %0,$vf3" \
                     :"=r"(result):"r"(first),"r"(second)); \
    return body_value(result); \
}
BODY_VU_SCALAR(body_add,"vadd.x $vf3,$vf1,$vf2")
BODY_VU_SCALAR(body_sub,"vsub.x $vf3,$vf1,$vf2")
BODY_VU_SCALAR(body_mul,"vmul.x $vf3,$vf1,$vf2")
BODY_VU_SCALAR(body_max,"vmax.x $vf3,$vf1,$vf2")
BODY_VU_SCALAR(body_min,"vmini.x $vf3,$vf1,$vf2")
BODY_VU_SCALAR(body_div,"vdiv Q,$vf1x,$vf2x\n\tvwaitq\n\tvaddq.x $vf3,$vf0,Q")
BODY_VU_SCALAR(body_rsqrt,"vrsqrt Q,$vf1x,$vf2x\n\tvwaitq\n\tvaddq.x $vf3,$vf0,Q")
#undef BODY_VU_SCALAR
static s32 body_integer(f32 a) {
    u32 result,first=body_word(a);
    __asm__ volatile("qmtc2.ni %1,$vf1\n\tvftoi0.x $vf2,$vf1\n\tqmfc2.ni %0,$vf2"
                     :"=r"(result):"r"(first));return (s32)result;
}
static void body_difference(const BodyVector *a,const BodyVector *b,BodyVector *out) {
    __asm__ volatile("lqc2 $vf1,0(%0)\n\tlqc2 $vf2,0(%1)\n\t"
                     "vsub.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0(%2)"
                     ::"r"(a),"r"(b),"r"(out):"memory");
}
static void body_cross(const BodyVector *a,const BodyVector *b,BodyVector *out) {
    __asm__ volatile("lqc2 $vf1,0(%0)\n\tlqc2 $vf2,0(%1)\n\tvmove.w $vf3,$vf1\n\t"
                     "vopmula.xyz ACC,$vf1,$vf2\n\tvopmsub.xyz $vf3,$vf2,$vf1\n\tsqc2 $vf3,0(%2)"
                     ::"r"(a),"r"(b),"r"(out):"memory");
}
static f32 body_dot(const BodyVector *a,const BodyVector *b) {
    u32 result;
    __asm__ volatile("lqc2 $vf1,0(%1)\n\tlqc2 $vf2,0(%2)\n\t"
                     "vaddw.xyz $vf4,$vf0,$vf0w\n\tvmul.xyz $vf1,$vf1,$vf2\n\t"
                     "vadday.x ACC,$vf1,$vf1y\n\tvmaddz.x $vf3,$vf4,$vf1z\n\tqmfc2.ni %0,$vf3"
                     :"=r"(result):"r"(a),"r"(b));return body_value(result);
}
/* Original VADDA + VMADD preserve the accumulator operation, rather than
 * replacing it with a separately rounded multiply and add. */
static void body_project(const BodyVector *base,const BodyVector *direction,
                         f32 scale,BodyVector *out) {
    u32 bits=body_word(scale);
    __asm__ volatile("lqc2 $vf1,0(%0)\n\tlqc2 $vf2,0(%1)\n\tqmtc2.ni %3,$vf3\n\t"
                     "vadda.xyz ACC,$vf0,$vf1\n\tvmaddx.xyz $vf1,$vf2,$vf3x\n\tsqc2 $vf1,0(%2)"
                     ::"r"(base),"r"(direction),"r"(out),"r"(bits):"memory");
}
static u32 body_clip(u16 vertex,u16 lower,u16 upper) {
    return ((u16)(vertex-lower)>>15)|(((u16)(upper-vertex)>>15)<<1);
}
static void body_decode(u64 packed,const u16 lower[4],const u16 upper[4],BodyVector *out) {
    s32 i;u32 clip=0;
    for(i=0;i<4;i++) {u16 v=(u16)(packed>>(i*16));out->word[i]=v;clip|=body_clip(v,lower[i],upper[i])<<(i*8);}
    __asm__ volatile("lqc2 $vf1,0(%0)\n\tvitof0.xyz $vf1,$vf1\n\tsqc2 $vf1,0(%0)"
                     ::"r"(out):"memory");out->word[3]=clip;
}
static s32 body_triangle(const BodyVector v[3],const BodyVector *center,
                         f32 *best,BodyVector *position,BodyVector *normal) {
    BodyVector e2,e3,delta,projection,n,negative,side[3],edge,toward,begin,end,product;
    f32 dot,norm2,q,distance,test[3],difference;u32 i;
    body_difference(v+1,v,&e2);body_difference(v+2,v,&e3);
    body_difference(center,v,&delta);body_cross(&e3,&e2,&n);
    dot=body_dot(&delta,&n);norm2=body_dot(&n,&n);q=body_div(dot,norm2);
    negative.quad=n.quad;for(i=0;i<3;i++)negative.value[i]=body_sub(body_value(0),n.value[i]);
    if(body_word(dot)&0x80000000U)return 0;
    product.quad=n.quad;for(i=0;i<3;i++)product.value[i]=body_mul(n.value[i],q);
    body_project(&delta,&negative,q,&projection);distance=body_dot(&product,&product);
    body_cross(&projection,&e2,side);body_cross(&e3,&projection,side+1);
    body_difference(&e3,&e2,&edge);body_difference(&projection,&e2,&toward);
    body_cross(&toward,&edge,side+2);difference=body_sub(*best,distance);
    test[0]=body_dot(side,&n);
    if(body_word(difference)&0x80000000U)return 0;
    test[1]=body_dot(side+1,&n);test[2]=body_dot(side+2,&n);
    if((body_word(test[0])|body_word(test[1])|body_word(test[2]))&0x80000000U) {
        begin.quad=v[0].quad;end.quad=v[0].quad;
        /* Branches542620/542628/542630 select e2→e3, e3→0 or0→e2.
         * The first predicate tested is side0, followed by side1/side2. */
        if(body_word(test[0])&0x80000000U) {begin.quad=v[0].quad;end.quad=v[0].quad;for(i=0;i<3;i++){begin.value[i]=0;end.value[i]=e2.value[i];}}
        else if(body_word(test[1])&0x80000000U) {begin.quad=e3.quad;for(i=0;i<3;i++)end.value[i]=0;}
        else {begin.quad=e2.quad;end.quad=e3.quad;}
        body_difference(&projection,&begin,&toward);body_difference(&end,&begin,&edge);
        q=body_div(body_dot(&toward,&edge),body_dot(&edge,&edge));
        q=body_max(q,body_value(0));q=body_min(q,body_value(0x3F800000));
        body_project(&begin,&edge,q,&projection);
        body_difference(center,v,&toward);body_difference(&projection,&toward,&toward);
        distance=body_dot(&toward,&toward);difference=body_sub(*best,distance);
        if(body_word(difference)&0x80000000U)return 0;
    }
    *best=body_mul(distance,body_value(0x3F7FDF3B));
    for(i=0;i<3;i++) {normal->value[i]=body_add(n.value[i],body_value(0));position->value[i]=body_add(projection.value[i],v[0].value[i]);}
    return 1;
}

s32 func_L01_00542280(BodyVector *center,u8 *collectionEnd,f32 radiusInput) {
    BodyVector scaled,output[5],triangle[3],*vertices=(BodyVector *)0x70002000;
    BodyVector position,normal,lowerVector,upperVector;
    u32 *selected=(u32 *)0x70000000,*selectedCursor=selected,*selectedEnd;
    u8 *record,*payload,*face,*faceEnd,*cursor=collectionEnd,*group;
    u16 lower[4],upper[4];s32 packedCenter[4],delta[4],absolute[4],square[4];
    s32 count,i,j,remaining,have=0,accept;u32 nf,nv,word,nextWord,indices[3],common,vf2w=0;
    u64 pending,first,second;f32 radius,best,distance,q;
    __asm__ volatile("vcallms 0xE90\n\tvcallms 0xE10\n\t"
                     "sqc2 $vf25,0(%0)\n\tsqc2 $vf26,16(%0)\n\tsqc2 $vf27,32(%0)\n\t"
                     "sqc2 $vf28,48(%0)\n\tsqc2 $vf29,64(%0)"::"r"(output):"memory");
    scaled.quad=center->quad;
    if(D_00222BE0==NULL)goto miss;
    count=*(s32 *)D_00222BE0;record=D_00222BE0+16;
    for(i=0;i<3;i++)scaled.value[i]=body_mul(scaled.value[i],body_value(0x42800000));
    radius=body_mul(radiusInput,body_value(0x42800000));scaled.value[3]=body_mul(body_sub(body_value(0),radiusInput),body_value(0x42800000));
    for(i=0;i<4;i++) {
        packedCenter[i]=body_integer(scaled.value[i]);
        lowerVector.value[i]=i==3?body_value(0):body_sub(scaled.value[i],radius);
        upperVector.value[i]=i==3?body_value(0):body_add(scaled.value[i],radius);
        lower[i]=(u16)body_integer(lowerVector.value[i]);upper[i]=(u16)body_integer(upperVector.value[i]);
    }
    pending=*(volatile u64 *)record;
    while(count>0) {
        first=pending;pending=*(volatile u64 *)(record+16);accept=1;
        for(i=0;i<4;i++) {s32 a;delta[i]=(s32)((u32)((first>>(i*16))&65535U)-(u32)packedCenter[i]);a=delta[i];absolute[i]=a==(s32)0x80000000U?0x7FFFFFFF:a<0?-a:a;}
        for(i=0;i<3;i++)if(body_checked_sub(absolute[3],absolute[i])<0)accept=0;
        if(accept) {
            for(i=0;i<4;i++) {s32 h=(s16)(delta[i]>>1);square[i]=h*h;}
            accept=body_checked_sub(body_checked_sub(body_checked_sub(square[3],square[0]),square[1]),square[2])>=0;
            /* Original delay-slot SW executes even when the sphere test fails. */
            *selectedCursor=(u32)record;if(accept)selectedCursor++;
        }
        record+=16;count--;
    }
    selectedEnd=(u32 *)(0x70000000U+(((u32)selectedCursor)&0xFFF));best=body_mul(radius,radius);
    while(selected!=selectedEnd) {
        group=(u8 *)*selected++;payload=*(u8 **)(group+12);nv=*(u16 *)(group+10);remaining=(s32)nv;
        first=*(volatile u64 *)payload;second=*(volatile u64 *)(payload+8);i=0;
        do {
            body_decode(first,lower,upper,vertices+i);body_decode(second,lower,upper,vertices+i+1);
            vf2w=(u32)(second>>48);
            first=*(volatile u64 *)(payload+(i+2)*8);second=*(volatile u64 *)(payload+(i+3)*8);
            i+=2;remaining-=2;
        } while(remaining>0);
        /* Both fields are genuinely reloaded after vertex preparation. */
        nv=*(u16 *)(group+10);payload=*(u8 **)(group+12);nf=*(u8 *)(group+8);
        face=payload+nv*8;faceEnd=face+nf*4;word=*(volatile u32 *)face;
        while(face!=faceEnd) {
            for(i=0;i<3;i++)indices[i]=(word>>(i*8))&255;
            common=vertices[indices[0]].word[3]&vertices[indices[1]].word[3]&vertices[indices[2]].word[3];
            nextWord=*(volatile u32 *)(face+4);face+=4;word=nextWord;
            if((s32)common>0)continue;
            for(i=0;i<3;i++)triangle[i].quad=vertices[indices[i]].quad;
            vf2w=triangle[1].word[3];
            if(!body_triangle(triangle,&scaled,&best,&position,&normal))continue;
            have=1;
            for(i=0;i<3;i++) {output[0].value[i]=position.value[i];output[1].value[i]=normal.value[i];}
            for(i=0;i<3;i++)for(j=0;j<3;j++)output[i+2].value[j]=body_add(triangle[i].value[j],body_value(0));
            if(collectionEnd!=NULL) {
                best=body_mul(radius,radius);
                if(cursor==collectionEnd-4096)goto collectDone;
                distance=body_dot(&normal,&normal);cursor-=64;
                for(i=0;i<3;i++) {for(j=0;j<3;j++)triangle[i].value[j]=body_mul(output[i+2].value[j],body_value(0x3C800000));*(BodyQuad *)(cursor+i*16)=triangle[i].quad;}
                normal.word[3]=body_word(distance);*(BodyQuad *)(cursor+48)=normal.quad;
                *(u32 *)(cursor+12)=0x100C;*(u32 *)(cursor+28)=0;
            }
        }
    }
    if(collectionEnd!=NULL)goto collectDone;
    if(!have)goto miss;
    q=body_rsqrt(radius,best);
    for(i=0;i<5;i++)if(i!=1)for(j=0;j<3;j++)output[i].value[j]=body_mul(output[i].value[j],body_value(0x3C800000));
    for(i=0;i<5;i++)*(BodyQuad *)(D_00242340+(i==0?32:i==1?64:80+(i-2)*16))=output[i].quad;
    *(u32 *)(D_00242340+24)=0;*(u32 *)(D_00242340+28)=0x100C;
    for(i=0;i<3;i++)position.value[i]=body_add(body_mul(body_mul(body_sub(scaled.value[i],position.value[i]),q),body_value(0x3C800000)),output[0].value[i]);
    position.word[3]=vf2w;*(BodyQuad *)(D_00242340+48)=position.quad;
    __asm__ volatile("vcallms 0xF00\n\tvcallms 0xF40":::"memory");return 1;
collectDone:
    __asm__ volatile("vcallms 0xF00\n\tvcallms 0xF40":::"memory");return (s32)((u32)collectionEnd-(u32)cursor)>>6;
miss:
    __asm__ volatile("vcallms 0xF00\n\tvcallms 0xF40":::"memory");return 0;
}
#endif
