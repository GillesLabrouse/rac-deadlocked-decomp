#include "common.h"
/* Complete capsule probe0053FAA8..00540A30 (0xF88), NONMATCHING.
 * Authored recovery: actual terrain selection, RAM-to-SPR DMA, packed vertices,
 * quad mutation, dynamic actor filters, model polygons and four primitive kinds.
 * a0=center16B, a1=flags, a2=ignored actor; independent f12=radius/f13=height.
 * The genuine53DBD0 call uses t8/at/v0/v1 and returns at; its targeted wrapper
 * retains this interface rather than inventing a standard C ABI. E90/E10/F00/
 * F40 remain real resident VU micro calls. No native success replacement.
 * Local helper decomposition changes scheduling/registers/size. Valid PS2 RAM,
 * original installed VU banks, coherent model/collision data and DMA hardware
 * are required; the separate native adapter explicitly limits dynamic geometry.
 */
#ifndef DEADLOCKED_NATIVE
typedef unsigned int BodyQuad __attribute__((mode(TI)));
typedef union BodyFloat {u32 bits;f32 value;} BodyFloat;
typedef union BodyVector {
    BodyQuad quad;
    u32 word[4];
    f32 value[4];
} __attribute__((aligned(16))) BodyVector;
extern u8 D_0026FF20[];
extern u8 *D_00222D14;
extern u8 D_00242340[] __attribute__((aligned(16)));
typedef char BodyVectorWidth[sizeof(BodyVector)==16?1:-1];

static f32 body_value(u32 bits) {BodyFloat v;v.bits=bits;return v.value;}
static u32 body_word(f32 value) {BodyFloat v;v.value=value;return v.bits;}
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
typedef struct CapsuleWinner {
    f32 position[3],normal[3],vertex[3][3],best;
    u32 metadata,vf2_w,vf3_w,object;
    s32 have;
} CapsuleWinner;
static u32 caps_bits(f32 x){return body_word(x);}
static f32 caps_float(u32 x){return body_value(x);}
static f32 caps_add(f32 a,f32 b){return body_add(a,b);}
static f32 caps_sub(f32 a,f32 b){return body_sub(a,b);}
static f32 caps_mul(f32 a,f32 b){return body_mul(a,b);}
static f32 caps_div(f32 a,f32 b){return body_div(a,b);}
static f32 caps_min(f32 a,f32 b){return body_min(a,b);}
static f32 caps_max(f32 a,f32 b){return body_max(a,b);}
static void caps_copy(f32 out[3],const f32 in[3]){s32 i;for(i=0;i<3;i++)out[i]=in[i];}
static f32 caps_dot(const f32 a[3],const f32 b[3]){
    BodyVector x,y;s32 i;x.word[3]=y.word[3]=0;
    for(i=0;i<3;i++){x.value[i]=a[i];y.value[i]=b[i];}return body_dot(&x,&y);
}
static void caps_difference(const f32 a[3],const f32 b[3],f32 out[3]){
    BodyVector x,y,z;s32 i;x.word[3]=y.word[3]=0;
    for(i=0;i<3;i++){x.value[i]=a[i];y.value[i]=b[i];}body_difference(&x,&y,&z);
    for(i=0;i<3;i++)out[i]=z.value[i];
}
static void caps_cross(const f32 a[3],const f32 b[3],f32 out[3]){
    BodyVector x,y,z;s32 i;x.word[3]=y.word[3]=0;
    for(i=0;i<3;i++){x.value[i]=a[i];y.value[i]=b[i];}body_cross(&x,&y,&z);
    for(i=0;i<3;i++)out[i]=z.value[i];
}
static void caps_projection(const f32 base[3],const f32 edge[3],f32 q,f32 out[3]){
    BodyVector x,y,z;s32 i;x.word[3]=y.word[3]=0;
    for(i=0;i<3;i++){x.value[i]=base[i];y.value[i]=edge[i];}body_project(&x,&y,q,&z);
    for(i=0;i<3;i++)out[i]=z.value[i];
}
static void caps_sides(const f32 point[3],const f32 e2[3],const f32 e3[3],const f32 n[3],f32 out[3]) {
    f32 a[3],b[3],c[3];
    caps_cross(point,e2,c);out[0]=caps_dot(c,n);
    caps_cross(e3,point,c);out[1]=caps_dot(c,n);
    caps_difference(e3,e2,a);caps_difference(point,e2,b);caps_cross(b,a,c);out[2]=caps_dot(c,n);
}
static void caps_edge(const f32 point[3],const f32 e2[3],const f32 e3[3],const f32 side[3],f32 out[3]) {
    f32 begin[3],end[3],edge[3],toward[3],q;u32 i;
    for(i=0;i<3;i++) {
        if(caps_bits(side[0])&0x80000000U){begin[i]=0.f;end[i]=caps_add(e2[i],0.f);}
        else if(caps_bits(side[1])&0x80000000U){begin[i]=caps_add(e3[i],0.f);end[i]=0.f;}
        else{begin[i]=caps_add(e2[i],0.f);end[i]=caps_add(e3[i],0.f);}
    }
    caps_difference(point,begin,toward);caps_difference(end,begin,edge);
    q=caps_div(caps_dot(toward,edge),caps_dot(edge,edge));q=caps_min(caps_max(q,0.f),1.f);
    caps_projection(begin,edge,q,out);
}
/* Complete original plane/vertical-segment/edge kernel53FF48..5402EC. */
static int caps_triangle(const f32 vertices[3][3],const f32 center[3],const f32 origin[3],f32 height,u32 flags,u32 surface,CapsuleWinner *winner) {
    f32 e2[3],e3[3],n[3],bottom[3],top[3],reference[3],projection[3],normalpart[3],side[3],d0,d1,q,distance,difference,norm2;
    u32 i,raw;
    if((flags&0x90U)&surface)return 0;
    if((flags&0x20U)&&((flags>>8)^surface)%16U==0)return 0;
    caps_difference(vertices[1],vertices[0],e2);caps_difference(vertices[2],vertices[0],e3);
    caps_difference(center,vertices[0],bottom);caps_copy(top,bottom);top[2]=caps_add(top[2],height);
    caps_cross(e3,e2,n);d0=caps_dot(bottom,n);d1=caps_dot(top,n);
    if(!(flags&0x1000U)&&((caps_bits(d0)&caps_bits(d1))&0x80000000U))return 0;
    if((caps_bits(n[2])&0x7FFFFFFFU)<0x3F800000U) {
        reference[0]=bottom[0];reference[1]=bottom[1];
        reference[2]=caps_add(caps_max(caps_min(caps_sub(0.f,bottom[2]),height),0.f),bottom[2]);
    } else {
        q=caps_div(d0,caps_mul(height,n[2]));
        raw=caps_bits(bottom[2])|caps_bits(caps_sub(bottom[2],e2[2]))|caps_bits(caps_sub(bottom[2],e3[2]));
        if(!(raw&0x80000000U))caps_copy(reference,bottom);
        else {
            raw=caps_bits(top[2])&caps_bits(caps_sub(top[2],e2[2]))&caps_bits(caps_sub(top[2],e3[2]));
            if(raw&0x80000000U)caps_copy(reference,top);
            else {
                caps_copy(projection,bottom);projection[2]=caps_sub(bottom[2],caps_mul(height,q));
                caps_sides(projection,e2,e3,n,side);
                if((caps_bits(side[0])|caps_bits(side[1])|caps_bits(side[2]))&0x80000000U)caps_edge(projection,e2,e3,side,projection);
                reference[0]=bottom[0];reference[1]=bottom[1];
                reference[2]=caps_add(caps_max(caps_min(caps_sub(projection[2],bottom[2]),height),0.f),bottom[2]);
            }
        }
    }
    norm2=caps_dot(n,n);q=caps_div(caps_dot(reference,n),norm2);
    for(i=0;i<3;i++)normalpart[i]=caps_mul(n[i],q);
    caps_difference(reference,normalpart,projection);distance=caps_dot(normalpart,normalpart);
    difference=caps_sub(winner->best,distance);if(caps_bits(difference)&0x80000000U)return 0;
    caps_sides(projection,e2,e3,n,side);
    if((caps_bits(side[0])|caps_bits(side[1])|caps_bits(side[2]))&0x80000000U) {
        caps_edge(projection,e2,e3,side,projection);caps_difference(projection,reference,normalpart);
        distance=caps_dot(normalpart,normalpart);difference=caps_sub(winner->best,distance);
        if(caps_bits(difference)&0x80000000U)return 0;
    }
    winner->best=caps_mul(distance,caps_float(0x3F7FDF3BU));winner->metadata=surface+0x1000U;winner->have=1;
    for(i=0;i<3;i++) {
        f32 first=caps_add(vertices[0][i],origin[i]);
        winner->position[i]=caps_add(projection[i],first);winner->normal[i]=caps_add(n[i],0.f);
        winner->vertex[0][i]=caps_add(0.f,first);
        winner->vertex[1][i]=caps_add(e2[i],first);winner->vertex[2][i]=caps_add(e3[i],first);
    }
    return 1;
}
static s32 caps_add32(s32 a,s32 b){s32 out;__asm__ volatile("add %0,%1,%2":"=r"(out):"r"(a),"r"(b));return out;}
static u8 *caps_pointer(u8 *a,s32 b){return (u8 *)(u32)caps_add32((s32)(u32)a,b);}
static u32 caps_tree(u8 *root,s32 x,s32 y,s32 z){
    u32 result;
    __asm__ volatile(".set noat\n\t.set noreorder\n\t"
                     "move $24,%1\n\tmove $1,%2\n\tmove $2,%3\n\tmove $3,%4\n\t"
                     "jal func_L01_0053DBD0\n\tnop\n\tmove %0,$1\n\t"
                     ".set reorder\n\t.set at"
                     :"=r"(result):"r"(root),"r"(x),"r"(y),"r"(z)
                     :"$1","$2","$3","$4","$5","$6","$24","$31","memory");
    return result;
}
static void caps_dma(u8 *root,u32 leaf,u32 bank,s32 clear){
    volatile u32 *dma=(volatile u32 *)0x1000D400;
    if(clear)*(volatile u64 *)(0x70000000U+bank)=0;
    dma[8]=leaf&255U;dma[32]=bank;dma[4]=(u32)caps_pointer(root,(s32)(leaf>>8));dma[0]=0x100;
}
static void caps_wait(void){while(*(volatile u32 *)0x1000D400&0x100U){__asm__ volatile("vnop\n\tnop");}}
static void caps_transform(const BodyVector matrix[3],const f32 input[3],const f32 translation[3],s32 includeTranslation,f32 output[3]){
    BodyVector in,t,out;u32 i;in.word[3]=t.word[3]=0;
    for(i=0;i<3;i++){in.value[i]=input[i];t.value[i]=translation[i];}
    if(!includeTranslation){
    __asm__ volatile("lqc2 $vf1,0(%0)\n\tlqc2 $vf2,0(%1)\n\tlqc2 $vf3,16(%1)\n\tlqc2 $vf4,32(%1)\n\t"
                     "vmulax.xyz ACC,$vf2,$vf1x\n\tvmadday.xyz ACC,$vf3,$vf1y\n\tvmaddz.xyz $vf1,$vf4,$vf1z\n\tsqc2 $vf1,0(%2)"
                     ::"r"(&in),"r"(matrix),"r"(&out):"memory");
    }else{
        /* Original translation is an accumulator term before the final MADD. */
        __asm__ volatile("lqc2 $vf1,0(%0)\n\tlqc2 $vf2,0(%1)\n\tlqc2 $vf3,16(%1)\n\tlqc2 $vf4,32(%1)\n\tlqc2 $vf5,0(%3)\n\t"
                         "vmulax.xyz ACC,$vf2,$vf1x\n\tvmadday.xyz ACC,$vf3,$vf1y\n\tvmaddaz.xyz ACC,$vf4,$vf1z\n\t"
                         "vmaddw.xyz $vf1,$vf5,$vf0w\n\tsqc2 $vf1,0(%2)"
                         ::"r"(&in),"r"(matrix),"r"(&out),"r"(&t):"memory");
    }
    for(i=0;i<3;i++)output[i]=out.value[i];
}
static u32 caps_clip(s32 value,s32 lower,s32 upper){return ((u16)((u32)value-(u32)lower)>>15)|(((u16)((u32)upper-(u32)value)>>15)<<1);}
static s32 caps_signed(u32 v,u32 n){u32 sign=1U<<(n-1);return (s32)(v&((1U<<n)-1))-(s32)((v&sign)*2);}
static void caps_faces(BodyVector *vertices,u8 *face,u8 *faceEnd,const f32 center[3],const f32 origin[3],f32 height,u32 flags,u32 object,CapsuleWinner *winner){
    u32 word=*(volatile u32 *)face,next,index[3],common,i,j;f32 triangle[3][3];
    while(face!=faceEnd){
        for(i=0;i<3;i++)index[i]=(word>>(i*8))&255U;
        common=vertices[index[0]].word[3]&vertices[index[1]].word[3]&vertices[index[2]].word[3];
        next=*(volatile u32 *)(face+4);face+=4;
        if((s32)common<=0){
            winner->vf2_w=vertices[index[1]].word[3];winner->vf3_w=vertices[index[2]].word[3];
            for(i=0;i<3;i++)for(j=0;j<3;j++)triangle[i][j]=vertices[index[i]].value[j];
            if(caps_triangle((const f32 (*)[3])triangle,center,origin,height,flags,word>>24,winner))winner->object=object;
        }
        word=next;
    }
}
/* Primitive endpoint transforms and the original horizontal segment choice. */
static void caps_primitive(const BodyVector matrix[3],const f32 translation[3],f32 scale,u8 *model,u8 *record,s32 kind,const f32 center[3],f32 point[3],f32 *radius,u32 *vf2w,u32 *vf3w){
    BodyVector first,second,definition;f32 a[3],b[3],edge[3],toward[3],n,d,q;u32 i;
    if(kind==1){
        first=*(BodyVector *)(record+16);
        for(i=0;i<3;i++)a[i]=caps_mul(first.value[i],scale);
        *radius=caps_mul(first.value[3],scale);caps_transform(matrix,a,translation,1,point);return;
    }
    if(kind==2){
        first=*(BodyVector *)caps_pointer(model,caps_add32((s32)((u32)*(s32 *)(record+4)<<6),48));
        definition=*(BodyVector *)record;second=*(BodyVector *)(record+16);*vf3w=second.word[3];
        for(i=0;i<3;i++)a[i]=caps_mul(caps_add(first.value[i],second.value[i]),scale);
        *radius=caps_mul(definition.value[3],scale);*vf2w=definition.word[3];caps_transform(matrix,a,translation,1,point);return;
    }
    definition=*(BodyVector *)record;
    if(kind==3){
        first=*(BodyVector *)(record+16);*radius=caps_mul(first.value[3],scale);*vf2w=definition.word[3];*vf3w=caps_bits(*radius);
        for(i=0;i<3;i++)a[i]=caps_mul(first.value[i],scale);
        caps_transform(matrix,a,translation,1,point);
        d=caps_mul(definition.value[1],scale);
        for(i=0;i<3;i++)edge[i]=caps_mul(matrix[2].value[i],d);
    }else{
        first=*(BodyVector *)caps_pointer(model,caps_add32((s32)((u32)(s32)*(s16 *)(record+4)<<6),48));
        second=*(BodyVector *)caps_pointer(model,caps_add32((s32)((u32)(s32)*(s16 *)(record+6)<<6),48));
        for(i=0;i<3;i++){a[i]=caps_mul(first.value[i],scale);b[i]=caps_mul(second.value[i],scale);}
        *radius=caps_mul(definition.value[3],scale);*vf3w=caps_bits(*radius);caps_transform(matrix,a,translation,1,point);caps_transform(matrix,b,translation,1,b);
        *vf2w=second.word[3];caps_difference(b,point,edge);
    }
    caps_difference(center,point,toward);
    n=caps_add(caps_mul(toward[0],edge[0]),caps_mul(toward[1],edge[1]));
    d=caps_add(caps_mul(edge[0],edge[0]),caps_mul(edge[1],edge[1]));q=caps_div(n,d);
    if(caps_bits(caps_sub(1.f,caps_float(caps_bits(d)&0x7FFFFFFFU)))&0x80000000U){
        q=caps_min(caps_max(q,0.f),1.f);caps_projection(point,edge,q,point);
    }else{
        /* 540760..774 deliberately leaves XY at the first endpoint. */
        for(i=0;i<3;i++)b[i]=caps_add(edge[i],point[i]);
        point[2]=caps_min(b[2],caps_max(point[2],center[2]));
    }
}
static s32 caps_dynamic(u32 ignored,u32 flags,u32 generation,const f32 center[3],const f32 low[3],const f32 high[3],f32 radius,f32 height,CapsuleWinner *winner,BodyVector *fastPoint,BodyVector *fastClosest){
    u32 *list=(u32 *)0x70003800,*cursor=list;u8 *pool=D_00222D14;
    s32 x,y,x0=(u32)body_integer(low[0])>>14,x1=(u32)body_integer(high[0])>>14;
    s32 y0=(u32)body_integer(low[1])>>14,y1=(u32)body_integer(high[1])>>14;
    u8 *cell,*start,*end,*actor,*definition,*model,*record,*payload;u32 index,type,cache,phase,i,j;
    BodyVector bounds,translationVector,matrix[3],scaleVector,*vertices=(BodyVector *)0x70002000,source;
    f32 total,delta[2],zlow,zhigh,broad,translation[3],relative[3],lower[3],upper[3],scale;
    f32 a[3],point[3],closest[3],toward[3],contact[3],normal[3],pradius,distance,q,combined,travel;
    s32 maskPrevious,mask,kind;u32 primitiveMask,object,remaining,nv,nf;u64 pending[2],next[2];
    winner->object=0;
    for(y=y0;y<=y1;y++)for(x=x0;x<=x1;x++){
        cell=caps_pointer(D_0026FF20,caps_add32(x<<2,y<<8));
        start=caps_pointer(D_0026FF20,caps_add32(0x4000,(s32)*(u16 *)cell<<5));
        *cursor++=(u32)start;*cursor++=(u32)caps_pointer(start,(s32)*(u8 *)(cell+2)<<1);
    }
    *(u64 *)cursor=0;cursor=list;
    while(*cursor){
        start=(u8 *)cursor[0];end=(u8 *)cursor[1];cursor+=2;
        for(;;){
            index=*(volatile u16 *)start;actor=caps_pointer(pool,(s32)(index<<8));
            if(start==end)break;bounds=*(BodyVector *)actor;start+=2;if((u32)actor==ignored)continue;
            total=caps_add(radius,bounds.value[3]);
            for(i=0;i<2;i++){delta[i]=caps_sub(bounds.value[i],center[i]);delta[i]=caps_mul(delta[i],delta[i]);}
            definition=*(u8 **)(actor+0x98);
            zlow=caps_sub(caps_sub(caps_sub(bounds.value[2],total),center[2]),height);
            zhigh=caps_sub(caps_add(bounds.value[2],total),center[2]);
            broad=caps_add(caps_sub(delta[0],caps_mul(total,total)),caps_mul(1.f,delta[1]));
            (void)caps_pointer(definition,(flags&4U)?0:2);
            if(*(u32 *)(actor+0xA0)==generation||definition==NULL||
               (s32)caps_bits(broad)>0||(s32)caps_bits(zhigh)<=0||(s32)caps_bits(zlow)>=0)continue;
            type=*(u32 *)(definition+12);*(u32 *)(actor+0xA0)=generation;model=*(u8 **)(actor+0x58);
            if(type==((flags&2U)?0U:1U))continue;
            translationVector=*(BodyVector *)(actor+16);
            for(i=0;i<3;i++)matrix[i]=*(BodyVector *)(actor+0xC0+i*16);
            scaleVector=*(BodyVector *)(actor+0x70);type=*(u32 *)(definition+12);cache=*(u32 *)(actor+0x9C);
            for(i=0;i<3;i++)translation[i]=caps_mul(translationVector.value[i],1024.f);
            scale=scaleVector.value[0];object=(u32)actor;
            if(type&&(s32)cache>=0){
                if(flags&0x2000U)continue;
                for(i=0;i<3;i++){relative[i]=caps_sub(center[i],translation[i]);lower[i]=caps_sub(relative[i],radius);upper[i]=caps_add(relative[i],radius);}
                upper[2]=caps_add(upper[2],height);
                for(i=0;i<3;i++)for(j=0;j<3;j++)matrix[i].value[j]=caps_mul(matrix[i].value[j],scale);
                payload=caps_pointer(definition,caps_add32(*(s32 *)(definition+4),16));remaining=*(u32 *)(definition+12);nv=0;
                pending[0]=*(volatile u64 *)payload;pending[1]=*(volatile u64 *)(payload+8);
                /* Original signed-halfword source pairs include one next pair preload. */
                do{
                    next[0]=*(volatile u64 *)(payload+16);next[1]=*(volatile u64 *)(payload+24);
                    for(phase=0;phase<2;phase++){
                        for(i=0;i<3;i++)a[i]=(f32)(s16)(pending[phase]>>(i*16));
                        caps_transform(matrix,a,translation,0,point);source.word[3]=0;
                        for(i=0;i<3;i++)source.value[i]=point[i];
                        source.word[3]=0;
                        for(i=0;i<3;i++)source.word[3]|=((caps_bits(caps_sub(point[i],lower[i]))>>31)|((caps_bits(caps_sub(upper[i],point[i]))>>30)&2U))<<(i*8);
                        vertices[nv++]=source;
                    }
                    payload+=16;remaining=(u32)caps_add32((s32)remaining,-16);pending[0]=next[0];pending[1]=next[1];
                }while((s32)remaining>0);
                payload=caps_pointer(definition,caps_add32(caps_add32(*(s32 *)(definition+4),16),*(s32 *)(definition+12)));
                nf=*(u32 *)(definition+8);caps_faces(vertices,payload,payload+nf,relative,translation,height,flags,object,winner);
                continue;
            }
            scaleVector=*(BodyVector *)(actor+0x70);scale=scaleVector.value[0];
            remaining=*(u32 *)(definition+4);if(flags&2U||!remaining)continue;
            record=definition+16;cache=*(u32 *)(actor+0x9C);primitiveMask=(flags&4U)?1U:2U;maskPrevious=0;
            while(maskPrevious>=0){
                u32 disabled=cache&1U;cache>>=1;mask=*(s16 *)(record+2);kind=*(s8 *)record;
                record=caps_pointer(record,32);maskPrevious=mask;if(disabled||!(primitiveMask&(u32)mask))continue;
                caps_primitive(matrix,translation,scale,model,record-32,kind,center,point,&pradius,&winner->vf2_w,&winner->vf3_w);
                closest[0]=center[0];closest[1]=center[1];closest[2]=caps_add(center[2],caps_max(caps_min(caps_sub(point[2],center[2]),height),0.f));
                caps_difference(point,closest,toward);distance=caps_dot(toward,toward);combined=caps_add(radius,pradius);
                /* SQRT runs even when the combined-radius test rejects. */
                {u32 input=caps_bits(distance),out;__asm__ volatile("qmtc2.ni %1,$vf1\n\tvsqrt Q,$vf1x\n\tvwaitq\n\tvaddq.x $vf2,$vf0,Q\n\tqmfc2.ni %0,$vf2":"=r"(out):"r"(input));travel=caps_float(out);}
                if(caps_bits(caps_sub(caps_mul(combined,combined),distance))&0x80000000U)continue;
                q=caps_div(1.f,travel);travel=caps_mul(travel,1.f);travel=caps_min(travel,pradius);
                caps_difference(closest,point,normal);
                for(i=0;i<3;i++){normal[i]=caps_mul(caps_mul(normal[i],q),travel);contact[i]=caps_add(point[i],normal[i]);}
                caps_difference(contact,closest,toward);distance=caps_dot(toward,toward);
                if(caps_bits(caps_sub(winner->best,distance))&0x80000000U)continue;
                winner->object=object;winner->metadata=(u32)(-(s32)(u32)(record-32));winner->best=caps_mul(distance,1.f);winner->have=1;
                if((s32)caps_bits(distance)<(s32)0x3E000000U){
                    for(i=0;i<3;i++){fastPoint->value[i]=point[i];fastClosest->value[i]=closest[i];}
                    fastPoint->value[3]=pradius;return 1;
                }
                caps_copy(winner->position,contact);caps_copy(winner->normal,normal);
            }
        }
    }
    return 0;
}
s32 func_L01_0053FAA8(BodyVector *centerInput,u32 callerFlags,u8 *ignoredActor,f32 radiusInput,f32 heightInput){
    BodyVector centerWorld,output[5],*selected=(BodyVector *)0x70003800,*selectedCursor=selected;
    BodyVector *record,*vertices=(BodyVector *)0x70002000,firstFast,closestFast;
    CapsuleWinner winner;f32 world[3],center[3],lowWorld[3],highWorld[3],low[3],high[3],radius,height;
    f32 current[3],initial[3],square[3],relative[3],origin[3],distance,radius2,q,dz,adjusted[3],position[3];
    s32 begin[3],end[3],x,y,z,lo[3],hi[3],generation,fast;
    u32 flags=callerFlags^0x10U,root,leaf,bank=0x1000U,nv,nf,nq,pending[2],next[2],i,j,index;
    u8 *block,*payload,*face,*endFace,*quadCursor,*quadRecord;u32 packet=(u32)D_00242340;
    __asm__ volatile("vcallms 0xE90\n\tvcallms 0xE10\n\t"
                     "sqc2 $vf25,0(%0)\n\tsqc2 $vf26,16(%0)\n\tsqc2 $vf27,32(%0)\n\t"
                     "sqc2 $vf28,48(%0)\n\tsqc2 $vf29,64(%0)"::"r"(output):"memory");
    centerWorld=*centerInput;generation=*(s32 *)(packet+16);
    for(i=0;i<3;i++){
        world[i]=centerWorld.value[i];lowWorld[i]=caps_sub(world[i],radiusInput);highWorld[i]=caps_add(world[i],radiusInput);
        center[i]=caps_mul(world[i],1024.f);
    }
    highWorld[2]=caps_add(highWorld[2],heightInput);radius=caps_mul(radiusInput,1024.f);height=caps_mul(heightInput,1024.f);
    if(generation<0)goto miss;generation=caps_add32(generation,1);*(s32 *)(packet+16)=generation;
    for(i=0;i<3;i++)for(j=0;j<3;j++)winner.vertex[i][j]=output[i+2].value[j];
    winner.have=0;winner.metadata=0;winner.object=0;winner.vf2_w=0;winner.vf3_w=0;radius2=caps_mul(radius,radius);
    winner.best=caps_mul(radius2,caps_float(0x3F7FDF3B));
    for(i=0;i<3;i++)if(caps_bits(caps_sub(lowWorld[i],0.f))&0x80000000U)goto miss;
    root=*(u32 *)packet;if((s32)caps_bits(radiusInput)<=0)goto miss;
    for(i=0;i<3;i++)if(!(caps_bits(caps_sub(highWorld[i],1024.f))&0x80000000U))goto miss;
    if((s32)caps_bits(heightInput)<=0)goto miss;
    for(i=0;i<3;i++){low[i]=caps_mul(lowWorld[i],1024.f);high[i]=caps_mul(highWorld[i],1024.f);}
    if(!(flags&1U)){
        for(i=0;i<3;i++){begin[i]=(u32)body_integer(lowWorld[i])>>2;end[i]=(u32)body_integer(highWorld[i])>>2;initial[i]=(f32)((u32)begin[i]<<12);}
        *(volatile u64 *)0x70000000=0;current[2]=initial[2];
        for(z=begin[2];;z=caps_add32(z,1),current[2]=caps_add(current[2],4096.f)){
            current[1]=initial[1];for(y=begin[1];;y=caps_add32(y,1),current[1]=caps_add(current[1],4096.f)){
                current[0]=initial[0];for(x=begin[0];;x=caps_add32(x,1),current[0]=caps_add(current[0],4096.f)){
                    for(i=0;i<3;i++){
                        f32 closest=caps_min(caps_max(current[i],center[i]),caps_add(current[i],4096.f));
                        f32 segment=i==2?caps_max(center[i],caps_min(caps_add(center[i],height),closest)):caps_add(center[i],0.f);
                        f32 delta=caps_sub(closest,segment);square[i]=caps_mul(delta,delta);
                    }
                    distance=caps_add(caps_add(caps_add(caps_sub(0.f,radius2),square[0]),caps_mul(1.f,square[1])),caps_mul(1.f,square[2]));
                    if(caps_bits(distance)&0x80000000U){
                        for(i=0;i<3;i++)selectedCursor->value[i]=caps_add(current[i],2048.f);
                        leaf=caps_tree((u8 *)root,x,y,z);selectedCursor->word[3]=leaf;
                        if(leaf){if(selectedCursor==selected)caps_dma((u8 *)root,leaf,0,0);selectedCursor++;}
                    }
                    if(x==end[0])break;
                }
                if(y==end[1])break;
            }
            if(z==end[2])break;
        }
        selectedCursor->word[3]=0;record=selected;
        while(record->word[3]){
            leaf=record[1].word[3];for(i=0;i<3;i++)origin[i]=record->value[i];record++;
            bank^=0x1000U;caps_wait();if(leaf)caps_dma((u8 *)root,leaf,bank^0x1000U,1);
            block=(u8 *)(0x70000000U+bank);nv=*(u8 *)(block+2);
            for(i=0;i<3;i++)relative[i]=caps_sub(center[i],origin[i]);
            if(!nv)continue;
            for(i=0;i<3;i++){lo[i]=body_integer(caps_sub(low[i],origin[i]));hi[i]=body_integer(caps_sub(high[i],origin[i]));}
            payload=block+4;pending[0]=*(volatile u32 *)payload;pending[1]=*(volatile u32 *)(payload+4);index=0;
            do{
                next[0]=*(volatile u32 *)(payload+8);next[1]=*(volatile u32 *)(payload+12);
                for(j=0;j<2;j++){
                    s32 decoded[3];decoded[0]=caps_signed(pending[j],10)*64;decoded[1]=caps_signed(pending[j]>>10,10)*64;decoded[2]=caps_signed(pending[j]>>20,12)*16;
                    vertices[index].word[3]=0;
                    for(i=0;i<3;i++){vertices[index].value[i]=(f32)decoded[i];vertices[index].word[3]|=caps_clip(decoded[i],lo[i],hi[i])<<(i*8);}
                    index++;
                }
                payload+=8;pending[0]=next[0];pending[1]=next[1];
            }while((s32)index<(s32)nv);
            winner.vf2_w=0;
            nv=*(u8 *)(block+2);nf=*(u8 *)block;face=block+4+nv*4;endFace=face+nf*4;
            caps_faces(vertices,face,endFace,relative,origin,height,flags,root,&winner);
            nq=*(u8 *)(block+3);if(nq){
                quadCursor=endFace;quadRecord=face;
                for(i=0;i<nq;i++){u8 third=*(u8 *)(quadRecord+2),fourth=*(u8 *)quadCursor++;quadRecord[1]=third;quadRecord[2]=fourth;quadRecord+=4;}
                caps_faces(vertices,face,face+nq*4,relative,origin,height,flags,root,&winner);
            }
        }
    }
    fast=caps_dynamic((u32)ignoredActor,flags,(u32)generation,center,low,high,radius,height,&winner,&firstFast,&closestFast);
    if(fast){
        f32 delta[3],length;BodyVector normal,contact,corrected;
        caps_difference(closestFast.value,firstFast.value,delta);length=caps_dot(delta,delta);q=body_rsqrt(1.f,length);
        normal.word[3]=body_word(caps_mul(caps_add(firstFast.value[3],radius),1.f/1024.f));
        contact.word[3]=winner.vf3_w;corrected.word[3]=0;
        /* VF0's actual constant quad is (0,0,0,1), not an all-zero integer. */
        {BodyVector zero;zero.word[0]=zero.word[1]=zero.word[2]=0;zero.word[3]=0x3F800000U;*(BodyQuad *)(packet+80)=zero.quad;*(BodyQuad *)(packet+96)=zero.quad;*(BodyQuad *)(packet+112)=zero.quad;}
        *(u32 *)(packet+24)=winner.object;*(u32 *)(packet+28)=winner.metadata;
        for(i=0;i<3;i++){
            f32 point=caps_mul(firstFast.value[i],1.f/1024.f);normal.value[i]=caps_mul(delta[i],q);
            contact.value[i]=caps_add(caps_mul(normal.value[i],caps_mul(firstFast.value[3],1.f/1024.f)),point);
            corrected.value[i]=caps_add(caps_mul(normal.value[i],normal.value[3]),point);
        }
        *(BodyQuad *)(packet+64)=normal.quad;*(BodyQuad *)(packet+32)=contact.quad;*(BodyQuad *)(packet+48)=corrected.quad;
        goto hitDone;
    }
    if(!winner.metadata)goto miss;q=body_rsqrt(radius,winner.best);
    for(i=0;i<3;i++){
        output[0].value[i]=caps_mul(winner.position[i],1.f/1024.f);output[1].value[i]=winner.normal[i];
        for(j=0;j<3;j++)output[j+2].value[i]=caps_mul(winner.vertex[j][i],1.f/1024.f);
        position[i]=output[0].value[i];
    }
    for(i=0;i<5;i++)*(BodyQuad *)(packet+(i==0?32:i==1?64:80+(i-2)*16))=output[i].quad;
    *(u32 *)(packet+24)=winner.object;*(u32 *)(packet+28)=winner.metadata;
    if((s32)caps_bits(winner.best)<(s32)0x3E000000U){
        caps_copy(adjusted,position);
        if(*(s32 *)(packet+72)<0)adjusted[2]=caps_sub(caps_sub(adjusted[2],caps_mul(radius,1.f/1024.f)),caps_mul(height,1.f/1024.f));
        else adjusted[2]=caps_add(adjusted[2],caps_mul(radius,1.f/1024.f));
    }else{
        dz=caps_max(caps_min(caps_sub(winner.position[2],center[2]),height),0.f);
        for(i=0;i<3;i++){f32 delta=caps_sub(center[i],winner.position[i]);if(i==2)delta=caps_add(delta,dz);delta=caps_mul(delta,q);if(i==2)delta=caps_sub(delta,dz);adjusted[i]=caps_add(caps_mul(delta,1.f/1024.f),position[i]);}
    }
    {BodyVector out;for(i=0;i<3;i++)out.value[i]=adjusted[i];out.word[3]=winner.vf2_w;*(BodyQuad *)(packet+48)=out.quad;}
hitDone:
    __asm__ volatile("vcallms 0xF00\n\tvcallms 0xF40":::"memory");return 1;
miss:
    __asm__ volatile("vcallms 0xF00\n\tvcallms 0xF40":::"memory");return 0;
}
#endif
