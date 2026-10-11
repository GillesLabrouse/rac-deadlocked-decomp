#include "common.h"
/* Complete segment collision probe0053DC70..0053EC88 (0x1018), NONMATCHING.
 * Actual a0=start16B, a1=end16B, a2=flags, a3=ignored actor, t0=cache record.
 * Complete terrain/grid, model polygons, sphere/capsule primitives and cache
 * effects. E90/E10/F00/F40 and actual RAM-to-SPR DMA are retained. Tree helper
 * 53DBD0 has genuine t8/at/v0/v1 -> at ABI, not a standard C signature.
 * Source decomposition changes registers/scheduling/size; coherent PS2 RAM,
 * installed resident VU banks and DMA hardware are required. No native stub.
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

typedef struct WallWinner {
    f32 position[3],normal[3],vertex[3][3],best;
    u32 metadata,object;
} WallWinner;
static u32 wall_bits(f32 x){return body_word(x);}
static f32 wall_float(u32 x){return body_value(x);}
static f32 wall_add(f32 a,f32 b){return body_add(a,b);}
static f32 wall_sub(f32 a,f32 b){return body_sub(a,b);}
static f32 wall_mul(f32 a,f32 b){return body_mul(a,b);}
static f32 wall_div(f32 a,f32 b){return body_div(a,b);}
static f32 wall_min(f32 a,f32 b){return body_min(a,b);}
static f32 wall_max(f32 a,f32 b){return body_max(a,b);}
static void wall_copy(f32 o[3],const f32 a[3]){u32 i;for(i=0;i<3;i++)o[i]=a[i];}
static f32 wall_dot(const f32 a[3],const f32 b[3]){BodyVector x,y;u32 i;x.word[3]=y.word[3]=0;for(i=0;i<3;i++){x.value[i]=a[i];y.value[i]=b[i];}return body_dot(&x,&y);}
static void wall_difference(const f32 a[3],const f32 b[3],f32 o[3]){BodyVector x,y,z;u32 i;x.word[3]=y.word[3]=0;for(i=0;i<3;i++){x.value[i]=a[i];y.value[i]=b[i];}body_difference(&x,&y,&z);for(i=0;i<3;i++)o[i]=z.value[i];}
static void wall_cross(const f32 a[3],const f32 b[3],f32 o[3]){BodyVector x,y,z;u32 i;x.word[3]=y.word[3]=0;for(i=0;i<3;i++){x.value[i]=a[i];y.value[i]=b[i];}body_cross(&x,&y,&z);for(i=0;i<3;i++)o[i]=z.value[i];}
static void wall_projection(const f32 base[3],const f32 dir[3],f32 q,f32 o[3]){BodyVector a,b,c;u32 i;a.word[3]=b.word[3]=0;for(i=0;i<3;i++){a.value[i]=base[i];b.value[i]=dir[i];}body_project(&a,&b,q,&c);for(i=0;i<3;i++)o[i]=c.value[i];}
static s32 wall_add32(s32 a,s32 b){s32 out;__asm__ volatile("add %0,%1,%2":"=r"(out):"r"(a),"r"(b));return out;}
static u8 *wall_pointer(u8 *a,s32 b){return (u8 *)(u32)wall_add32((s32)(u32)a,b);}
static u32 wall_tree(u8 *root,s32 x,s32 y,s32 z){
    u32 result;
    __asm__ volatile(".set noat\n\t.set noreorder\n\t"
                     "move $24,%1\n\tmove $1,%2\n\tmove $2,%3\n\tmove $3,%4\n\t"
                     "jal func_L01_0053DBD0\n\tnop\n\tmove %0,$1\n\t"
                     ".set reorder\n\t.set at"
                     :"=r"(result):"r"(root),"r"(x),"r"(y),"r"(z)
                     :"$1","$2","$3","$4","$5","$6","$24","$31","memory");
    return result;
}
static void wall_dma(u8 *root,u32 leaf,u32 bank,s32 clear){
    volatile u32 *dma=(volatile u32 *)0x1000D400;
    if(clear)*(volatile u64 *)(0x70000000U+bank)=0;
    dma[8]=leaf&255U;dma[32]=bank;dma[4]=(u32)wall_pointer(root,(s32)(leaf>>8));dma[0]=0x100;
}
static void wall_wait(void){while(*(volatile u32 *)0x1000D400&0x100U){__asm__ volatile("vnop\n\tnop");}}
static void wall_transform(const BodyVector matrix[3],const f32 input[3],const f32 translation[3],s32 includeTranslation,f32 output[3]){
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
static u32 wall_clip(s32 value,s32 lower,s32 upper){return ((u16)((u32)value-(u32)lower)>>15)|(((u16)((u32)upper-(u32)value)>>15)<<1);}
static s32 wall_signed(u32 v,u32 n){u32 sign=1U<<(n-1);return (s32)(v&((1U<<n)-1))-(s32)((v&sign)*2);}

static void wall_triangle(const f32 v[3][3],const f32 start[3],const f32 end[3],const f32 dir[3],const f32 origin[3],u32 flags,u32 surface,u32 object,WallWinner *winner){
    f32 e2[3],e3[3],n[3],a[3],b[3],d[3],intersection[3],side[3],c[3],x[3],y[3],q,da,db,den,nearest;u32 i;
    if((flags&0x90U)&surface)return;
    wall_difference(v[1],v[0],e2);wall_difference(v[2],v[0],e3);
    wall_difference(v[0],start,a);wall_difference(v[0],end,b);wall_cross(e3,e2,n);
    for(i=0;i<3;i++)d[i]=wall_mul(dir[i],n[i]);
    if((flags&0x20U)&&!(((flags>>8)^surface)&15U))return;
    den=wall_add(wall_add(d[0],d[1]),wall_mul(1.f,d[2]));da=wall_dot(a,n);db=wall_dot(b,n);
    wall_difference(start,v[0],x);q=wall_div(da,den);
    if(!(flags&0x1000U)&&((s32)wall_bits(da)>=0||(s32)wall_bits(db)<=0))return;
    if(!((wall_bits(da)^wall_bits(db))&0x80000000U))return;
    wall_projection(x,dir,q,intersection);
    for(i=0;i<3;i++)c[i]=wall_add(v[0][i],origin[i]);
    wall_cross(intersection,e2,x);if((s32)wall_bits(q)<=0)return;side[0]=wall_dot(x,n);
    wall_cross(e3,intersection,x);side[1]=wall_dot(x,n);
    wall_difference(e3,e2,x);wall_difference(intersection,e2,y);wall_cross(y,x,d);side[2]=wall_dot(d,n);
    nearest=wall_sub(winner->best,q);
    if((wall_bits(side[0])|wall_bits(side[1])|wall_bits(side[2]))&0x80000000U)return;
    if((s32)wall_bits(nearest)<=0)return;
    winner->metadata=surface+0x1000U;winner->object=object;winner->best=wall_add(0.f,q);
    for(i=0;i<3;i++){
        winner->position[i]=wall_add(intersection[i],c[i]);winner->normal[i]=wall_add(n[i],0.f);
        winner->vertex[0][i]=wall_add(0.f,c[i]);winner->vertex[1][i]=wall_add(e2[i],c[i]);winner->vertex[2][i]=wall_add(e3[i],c[i]);
    }
}
static void wall_faces(BodyVector *vertices,u8 *face,u8 *limit,const f32 start[3],const f32 end[3],const f32 dir[3],const f32 origin[3],u32 flags,u32 object,WallWinner *winner){
    u32 word=*(volatile u32 *)face,next,index[3],common,i,j;f32 v[3][3];
    while(face!=limit){
        for(i=0;i<3;i++)index[i]=(word>>(i*8))&255U;
        common=vertices[index[0]].word[3]&vertices[index[1]].word[3]&vertices[index[2]].word[3];
        next=*(volatile u32 *)(face+4);face+=4;
        if((s32)common<=0){for(i=0;i<3;i++)for(j=0;j<3;j++)v[i][j]=vertices[index[i]].value[j];wall_triangle((const f32 (*)[3])v,start,end,dir,origin,flags,word>>24,object,winner);}
        word=next;
    }
}
typedef struct WallGridRecord {u32 fraction,cell;} WallGridRecord;
/* Real axis lists and stable X/Y/Z merge. The retail80th-entry path counts a
 * fraction without storing it, preserving the actual scratch contract. */
static s32 wall_grid(const f32 start[3],const f32 end[3],WallGridRecord *records){
    u32 packed=0,total=0,axis;f32 dir[3];s32 begin[3],finish[3],step[3],count[3],cursor[3],boundary,target,budget;
    volatile u32 *lists[3];f32 q,fraction;
    lists[0]=(u32 *)0x70003000;lists[1]=(u32 *)0x70003200;lists[2]=(u32 *)0x70003400;
    for(axis=0;axis<3;axis++){begin[axis]=((u32)body_integer(start[axis]))>>12;finish[axis]=((u32)body_integer(end[axis]))>>12;packed|=(u32)(u8)begin[axis]<<(axis*8);dir[axis]=wall_sub(end[axis],start[axis]);}
    records[0].fraction=0;records[0].cell=packed;
    if(begin[0]==finish[0]&&begin[1]==finish[1]&&begin[2]==finish[2])goto integer_test;
    for(axis=0;axis<3;axis++){
        q=wall_div(1.f,dir[axis]);step[axis]=finish[axis]>=begin[axis]?1:-1;count[axis]=0;cursor[axis]=0;
        boundary=begin[axis];target=finish[axis];if(step[axis]<0){boundary=wall_add32(boundary,1);target=wall_add32(target,1);}
        if(boundary!=target){
            boundary=wall_add32(boundary,step[axis]);budget=80;
            do{
                fraction=wall_mul(wall_sub((f32)(s32)((u32)boundary<<12),start[axis]),q);
                total++;count[axis]++;budget--;
                if(budget<=0)break;
                lists[axis][cursor[axis]++]=(u32)((s32)wall_bits(fraction)<0?0:(s32)wall_bits(fraction)>0x3F800000?0x3F800000:wall_bits(fraction));
                if(boundary==target)break;boundary=wall_add32(boundary,step[axis]);
            }while(1);
        }
        lists[axis][cursor[axis]]=0x50000000U;cursor[axis]=0;
        *(s8 *)(0x70003FFCU+axis)=(s8)step[axis];
    }
    for(axis=1;axis<=total;axis++){
        u32 x=lists[0][cursor[0]],y=lists[1][cursor[1]],z=lists[2][cursor[2]],selected;
        if((s32)y<(s32)x)selected=(s32)z<(s32)y?2:1;else selected=(s32)z<(s32)x?2:0;
        records[axis].fraction=lists[selected][cursor[selected]++];
        packed=(u32)wall_add32((s32)packed,(s32)((u32)step[selected]<<(selected*8)));
        records[axis].cell=packed;
    }
    integer_test:
    for(axis=0;axis<3;axis++)if(body_integer(start[axis])!=body_integer(end[axis]))break;
    if(axis==3)return 0;
    records[total+1].fraction=0x3F800000;records[total+1].cell=0;return (s32)total+2;
}

static f32 wall_sqrt(f32 a){u32 result,first=wall_bits(a);__asm__ volatile("qmtc2.ni %1,$vf1\n\tvsqrt Q,$vf1x\n\tvwaitq\n\tvaddq.x $vf2,$vf0,Q\n\tqmfc2.ni %0,$vf2":"=r"(result):"r"(first));return wall_float(result);}
static void wall_sphere(const f32 point[3],f32 radius2,const f32 start[3],const f32 end[3],u32 actor,u32 metadata,WallWinner *winner){
    f32 dir[3],toward[3],closest[3],delta[3],normal[3],a,b,c,q,project,test,disc;u32 i;
    wall_difference(end,start,dir);wall_difference(point,start,toward);
    a=wall_dot(dir,dir);b=wall_dot(toward,dir);c=wall_dot(toward,toward);
    project=wall_min(wall_max(wall_div(b,a),0.f),1.f);wall_projection(start,dir,project,closest);wall_difference(closest,point,delta);
    test=wall_add(wall_add(wall_sub(wall_mul(delta[0],delta[0]),radius2),wall_mul(1.f,wall_mul(delta[1],delta[1]))),wall_mul(1.f,wall_mul(delta[2],delta[2])));
    if((s32)wall_bits(test)>0)return;
    b=wall_add(b,b);c=wall_sub(c,radius2);disc=wall_mul(b,b);q=wall_mul(wall_mul(4.f,a),c);disc=wall_max(wall_sub(disc,q),0.f);
    q=wall_sqrt(disc);b=wall_sub(0.f,b);a=wall_add(a,a);b=wall_add(b,q);q=wall_div(b,a);
    q=wall_min(wall_max(wall_sub(0.f,q),0.f),1.f);wall_projection(start,dir,q,closest);wall_difference(closest,point,normal);
    if((s32)wall_bits(wall_sub(winner->best,q))<=0)return;
    winner->object=actor;winner->metadata=metadata;winner->best=wall_add(0.f,q);
    for(i=0;i<3;i++){winner->position[i]=wall_add(0.f,closest[i]);winner->normal[i]=wall_add(0.f,normal[i]);}
}
/* Actual line-line capsule reduction53E810..53E9AC, including the parallel
 * endpoint branch and every Q/sqrt before the final sphere test. */
static s32 wall_capsule_point(const f32 a[3],const f32 axis[3],f32 radius2,const f32 start[3],const f32 dir[3],f32 point[3]){
    f32 cross[3],opposite[3],n7[3],n8[3],square[3],toward[3],other[3],closest_axis[3],closest_ray[3],axis2,dir2,den,b,c,q,distance,cos2,advance;u32 i;
    wall_cross(axis,dir,cross);wall_cross(dir,axis,opposite);
    for(i=0;i<3;i++)square[i]=wall_mul(cross[i],cross[i]);
    wall_cross(dir,cross,n7);wall_cross(axis,opposite,n8);
    wall_copy(closest_ray,start);
    if(!((wall_bits(wall_sub(square[0],1.f))&wall_bits(wall_sub(square[1],1.f))&wall_bits(wall_sub(square[2],1.f)))&0x80000000U)){
        wall_difference(start,a,toward);wall_difference(a,start,other);den=wall_add(wall_add(square[0],square[1]),wall_mul(1.f,square[2]));
        b=wall_dot(n7,toward);c=wall_dot(n8,other);q=wall_div(b,den);wall_projection(a,axis,q,closest_axis);q=wall_div(c,den);wall_projection(start,dir,q,closest_ray);
        axis2=wall_dot(axis,axis);dir2=wall_dot(dir,dir);wall_difference(closest_ray,closest_axis,toward);
        distance=wall_add(wall_add(wall_add(wall_sub(0.f,radius2),wall_mul(toward[0],toward[0])),wall_mul(1.f,wall_mul(toward[1],toward[1]))),wall_mul(1.f,wall_mul(toward[2],toward[2])));
        q=body_rsqrt(1.f,axis2);if((s32)wall_bits(distance)>=0)return 0;
        distance=wall_sub(0.f,distance);for(i=0;i<3;i++)n7[i]=wall_mul(axis[i],q);
        q=body_rsqrt(1.f,dir2);for(i=0;i<3;i++)n8[i]=wall_mul(dir[i],q);
        cos2=wall_dot(n7,n8);cos2=wall_mul(cos2,cos2);cos2=wall_sub(1.f,cos2);
        q=wall_div(distance,cos2);q=wall_max(wall_add(0.f,q),0.f);advance=wall_sqrt(q);
        for(i=0;i<3;i++)closest_ray[i]=wall_sub(closest_ray[i],wall_mul(n8[i],advance));
        wall_difference(closest_ray,start,toward);b=wall_dot(toward,dir);
        if((s32)wall_bits(b)<0)wall_copy(closest_ray,start);
    }
    wall_difference(closest_ray,a,toward);b=wall_dot(toward,axis);den=wall_dot(axis,axis);q=wall_min(wall_max(wall_div(b,den),0.f),1.f);
    for(i=0;i<3;i++)point[i]=wall_add(a[i],wall_mul(axis[i],q));return 1;
}
static void wall_dynamic(u32 ignored,u32 flags,u32 generation,const f32 start[3],const f32 end[3],const f32 dir[3],WallGridRecord *records,WallWinner *winner){
    u32 *cells=(u32 *)0x70003800,*cursor=cells,prior=0xFFFFFFFFU,index,i,j;u8 *pool=D_00222D14,*actor,*definition,*list,*limit,*primitive,*model;
    BodyVector position,matrix[3],translation,scale,vertices[2],record;f32 normalized[3],toward[3],closest[3],product[3],point[3],axis[3],relative_start[3],relative_end[3],zero[3];
    f32 t,q,broad,radius2,a[3],b[3],low[3],high[3];u32 coarse,first,count,type,visited,mask,bits,pending[4],next[4];s32 kind,primitiveMask;
    BodyVector *decoded=(BodyVector *)0x70002000;
    zero[0]=zero[1]=zero[2]=0.f;q=wall_div(1.f,wall_dot(dir,dir));
    for(i=0;i<3;i++)normalized[i]=wall_mul(dir[i],q);
    for(index=0;records[index].fraction!=0x3F800000U;index++){
        coarse=((records[index].cell&255U)>>2)|((((records[index].cell>>8)&255U)>>2)<<16);
        if(coarse!=prior){*cursor++=coarse;prior=coarse;}
    }
    *cursor=0xFFFFFFFFU;winner->object=0;
    for(cursor=cells;(s32)*cursor>=0;cursor++){
        coarse=*cursor;definition=wall_pointer(D_0026FF20,(s32)((coarse>>8)+((coarse&65535U)<<2)));
        first=*(u16 *)definition;count=definition[2];list=wall_pointer(D_0026FF20,wall_add32(0x4000,(s32)(first<<5)));limit=wall_pointer(list,(s32)(count<<1));*cursor=(u32)limit;
        for(;;){
            index=*(volatile u16 *)list;actor=wall_pointer(pool,(s32)(index<<8));if(list==limit)break;
            position=*(BodyVector *)actor;list=wall_pointer(list,2);if((u32)actor==ignored)continue;
            wall_difference(position.value,start,toward);radius2=wall_mul(position.value[3],position.value[3]);bits=*(u16 *)(actor+0x34);definition=*(u8 **)(actor+0x98);
            for(i=0;i<3;i++)product[i]=wall_mul(toward[i],normalized[i]);
            if((flags&9U)&&!(bits&0x4000U))continue;
            t=wall_add(wall_add(product[0],product[1]),wall_mul(1.f,product[2]));t=wall_min(wall_max(t,0.f),winner->best);
            wall_projection(start,dir,t,closest);wall_difference(closest,position.value,toward);
            for(i=0;i<3;i++)product[i]=wall_mul(toward[i],toward[i]);
            broad=wall_add(wall_add(wall_sub(product[0],radius2),wall_mul(1.f,product[1])),wall_mul(1.f,product[2]));
            (void)wall_pointer(definition,(flags&4U)?0:2);if(!definition)continue;
            visited=*(u32 *)(actor+0xA0);if((s32)wall_bits(broad)>0)continue;
            *(u32 *)(actor+0xA0)=generation;type=*(volatile u32 *)(definition+12);if(visited==generation)continue;
            model=*(u8 **)(actor+0x58);if(type==((flags&2U)?0U:1U))continue;
            translation=*(BodyVector *)(actor+16);for(i=0;i<3;i++)matrix[i]=*(BodyVector *)(actor+0xC0+i*16);scale=*(BodyVector *)(actor+0x70);
            type=*(volatile u32 *)(definition+12);mask=*(u32 *)(actor+0x9C);for(i=0;i<3;i++)translation.value[i]=wall_mul(translation.value[i],1024.f);
            if(type&&(s32)mask>=0){
                u8 *payload,*face;u32 offset=*(u32 *)(definition+4),bytes=type,facebytes;
                if(flags&0x2000U)continue;
                wall_difference(start,translation.value,relative_start);wall_difference(end,translation.value,relative_end);
                for(i=0;i<3;i++){for(j=0;j<3;j++)matrix[i].value[j]=wall_mul(matrix[i].value[j],scale.value[0]);low[i]=wall_min(relative_start[i],relative_end[i]);high[i]=wall_max(relative_start[i],relative_end[i]);}
                payload=wall_pointer(definition,wall_add32((s32)offset,16));
                for(i=0;i<4;i++)pending[i]=*(volatile u32 *)(payload+i*4);
                index=0;
                do{
                    for(i=0;i<4;i++)next[i]=*(volatile u32 *)(payload+16+i*4);
                    for(j=0;j<2;j++){
                        for(i=0;i<3;i++)a[i]=(f32)(s16)(pending[j*2+(i>>1)]>>((i&1)*16));
                        wall_transform(matrix,a,zero,0,b);decoded[index].word[3]=0;
                        for(i=0;i<3;i++){decoded[index].value[i]=b[i];decoded[index].word[3]|=((wall_bits(wall_sub(b[i],low[i]))>>31)|((wall_bits(wall_sub(high[i],b[i]))>>31)<<1))<<(i*8);}
                        index++;
                    }
                    payload+=16;for(i=0;i<4;i++)pending[i]=next[i];bytes=(u32)wall_add32((s32)bytes,-16);
                }while((s32)bytes>0);
                bytes=*(volatile u32 *)(definition+12);facebytes=*(u32 *)(definition+8);face=wall_pointer(definition,wall_add32(wall_add32((s32)offset,16),(s32)bytes));
                wall_faces(decoded,face,wall_pointer(face,(s32)facebytes),relative_start,relative_end,dir,translation.value,flags,(u32)actor,winner);
                continue;
            }
            count=*(u32 *)(definition+4);primitive=definition+16;if((flags&2U)||!count)continue;
            primitiveMask=0;mask=*(u32 *)(actor+0x9C);
            for(;;){
                u32 skip;if(primitiveMask<0)break;skip=mask&1;mask>>=1;primitiveMask=*(s16 *)(primitive+2);kind=*(s8 *)primitive;
                primitive+=32;if(skip||!(((flags&4U)?1:2)&primitiveMask))continue;
                record=*(BodyVector *)(primitive-32);
                if(kind==1){vertices[0]=*(BodyVector *)(primitive-16);for(i=0;i<3;i++)a[i]=wall_mul(vertices[0].value[i],scale.value[0]);radius2=wall_mul(vertices[0].value[3],scale.value[0]);wall_transform(matrix,a,translation.value,1,point);radius2=wall_mul(radius2,radius2);}
                else if(kind==2){
                    vertices[0]=*(BodyVector *)wall_pointer(model,wall_add32((s32)((u32)*(s32 *)(primitive-28)<<6),48));vertices[1]=*(BodyVector *)(primitive-16);
                    for(i=0;i<3;i++)a[i]=wall_mul(wall_add(vertices[0].value[i],vertices[1].value[i]),scale.value[0]);radius2=wall_mul(record.value[3],scale.value[0]);wall_transform(matrix,a,zero,0,point);for(i=0;i<3;i++)point[i]=wall_add(point[i],translation.value[i]);radius2=wall_mul(radius2,radius2);
                }else{
                    if(kind==3){
                        vertices[0]=*(BodyVector *)(primitive-16);for(i=0;i<3;i++)a[i]=wall_mul(vertices[0].value[i],scale.value[0]);wall_transform(matrix,a,translation.value,1,point);q=wall_mul(record.value[1],scale.value[0]);for(i=0;i<3;i++)axis[i]=wall_mul(matrix[2].value[i],q);radius2=wall_mul(vertices[0].value[3],scale.value[0]);
                    }else{
                        vertices[0]=*(BodyVector *)wall_pointer(model,wall_add32((s32)((u32)(s32)*(s16 *)(primitive-28)<<6),48));vertices[1]=*(BodyVector *)wall_pointer(model,wall_add32((s32)((u32)(s32)*(s16 *)(primitive-26)<<6),48));
                        for(i=0;i<3;i++){a[i]=wall_mul(vertices[0].value[i],scale.value[0]);b[i]=wall_mul(vertices[1].value[i],scale.value[0]);}
                        wall_transform(matrix,a,zero,0,point);wall_transform(matrix,b,zero,0,closest);wall_difference(closest,point,axis);for(i=0;i<3;i++)point[i]=wall_add(point[i],translation.value[i]);radius2=wall_mul(record.value[3],scale.value[0]);
                    }
                    radius2=wall_mul(radius2,radius2);wall_copy(a,point);if(!wall_capsule_point(a,axis,radius2,start,dir,point))continue;
                }
                wall_sphere(point,radius2,start,end,(u32)actor,(u32)(-(s32)(u32)(primitive-32)),winner);
            }
        }
    }
}

static s32 wall_sub32(s32 a,s32 b){s32 out;__asm__ volatile("sub %0,%1,%2":"=r"(out):"r"(a),"r"(b));return out;}
static void wall_cache(u8 *actor,u8 *cache,const BodyVector *position){
    u32 packet=(u32)D_00242340,index=*(volatile u32 *)(packet+20),next,offset,flags,bits,age,oldActor,carry=0;
    s32 slot,ageDifference;u8 *target;BodyQuad first,second;u64 third;u32 fourth;
    if(!actor)return;bits=*(volatile u16 *)(actor+0x34);if(!cache)return;
    next=(u32)wall_add32((s32)index,1)&63U;offset=index<<6;slot=*(volatile s8 *)(actor+0x2C);
    if(!(bits&0x4000U))return;
    if(slot>=0){
        target=wall_pointer((u8 *)0x2423C0,(s32)((u32)slot<<6));
        oldActor=*(volatile u32 *)(target+60);age=*(volatile u32 *)(target+44);
        if(oldActor==(u32)actor){
            ageDifference=wall_sub32((s32)age,*(s32 *)(cache+28));flags=*(volatile u32 *)(target+36);
            if(ageDifference>=0){*(u32 *)(target+36)=*(u32 *)(cache+20)|flags;return;}carry=flags;
        }
    }
    /* Stale actor/age follows the original new-slot branch, not a miss. */
    *(u32 *)(packet+20)=next;*(s8 *)(actor+0x2C)=(s8)index;target=wall_pointer((u8 *)0x2423C0,(s32)offset);
    first=*(BodyQuad *)cache;second=*(BodyQuad *)(cache+16);third=*(u64 *)(cache+32);flags=*(u32 *)(cache+20);fourth=*(u32 *)(cache+40);
    *(BodyQuad *)target=position->quad;*(BodyQuad *)(target+16)=first;*(BodyQuad *)(target+32)=second;*(u64 *)(target+48)=third;*(u32 *)(target+56)=fourth;*(u32 *)(target+36)=flags|carry;*(u8 **)(target+60)=actor;
}

s32 func_L01_0053DC70(BodyVector *startInput,BodyVector *endInput,u32 callerFlags,u8 *ignoredActor,u8 *cache){
    BodyVector inputs[2],output[5],*vertices=(BodyVector *)0x70002000;
    WallGridRecord *records=(WallGridRecord *)0x70003800;WallWinner winner;
    f32 world[2][3],start[3],end[3],dir[3],origin[3],relative_start[3],relative_end[3],first[3],last[3];
    u32 i,j,flags=callerFlags^16U,packet=(u32)D_00242340,root,leaf,bank=0x1000U,nv,nf,nq,pending[2],next[2],index,record;
    s32 generation,count,low[3],high[3],a,b;u8 *block,*payload,*face,*limit,*quadCursor,*quad;
    __asm__ volatile("vcallms 0xE90\n\tvcallms 0xE10\n\t"
                     "sqc2 $vf25,0(%0)\n\tsqc2 $vf26,16(%0)\n\tsqc2 $vf27,32(%0)\n\t"
                     "sqc2 $vf28,48(%0)\n\tsqc2 $vf29,64(%0)"::"r"(output):"memory");
    inputs[0]=*startInput;inputs[1]=*endInput;output[3].word[3]=output[4].word[3]=0;
    generation=*(s32 *)(packet+16);for(i=0;i<3;i++){world[0][i]=inputs[0].value[i];world[1][i]=inputs[1].value[i];start[i]=wall_mul(world[0][i],1024.f);end[i]=wall_mul(world[1][i],1024.f);}
    if(generation<0)goto miss;generation=wall_add32(generation,1);*(s32 *)(packet+16)=generation;
    for(i=0;i<3;i++)if(wall_bits(wall_sub(wall_min(world[0][i],world[1][i]),0.f))&0x80000000U)goto miss;
    root=*(u32 *)packet;for(i=0;i<3;i++)if(!(wall_bits(wall_sub(wall_max(world[0][i],world[1][i]),1024.f))&0x80000000U))goto miss;
    for(i=0;i<3;i++)dir[i]=wall_sub(end[i],start[i]);
    records[0].fraction=0;records[0].cell=((u32)body_integer(start[0])>>12)|(((u32)body_integer(start[1])>>12)<<8)|(((u32)body_integer(start[2])>>12)<<16);
    if(!(flags&1U)){
        leaf=wall_tree((u8 *)root,(s32)(records[0].cell&255U),(s32)((records[0].cell>>8)&255U),(s32)(records[0].cell>>16));
        *(volatile u64 *)0x70000000=0;if(leaf)wall_dma((u8 *)root,leaf,0,0);
    }
    count=wall_grid(start,end,records);if(!count){wall_wait();goto miss;}
    winner.metadata=0;winner.object=0;winner.best=1.f;
    for(i=0;i<3;i++)for(j=0;j<3;j++)winner.vertex[i][j]=i==0?output[2].value[j]:wall_float((u32)body_integer(i==1?start[j]:end[j]));
    if(!(flags&1U))for(record=0;records[record].fraction!=0x3F800000U;record++){
        if(winner.metadata&&(s32)wall_bits(winner.best)<=(s32)records[record].fraction)break;
        bank^=0x1000U;wall_wait();
        if(records[record+1].fraction!=0x3F800000U){
            u32 cell=records[record+1].cell;leaf=wall_tree((u8 *)root,(s32)(cell&255U),(s32)((cell>>8)&255U),(s32)(cell>>16));
            *(volatile u64 *)(0x70000000U+(bank^0x1000U))=0;if(leaf)wall_dma((u8 *)root,leaf,bank^0x1000U,0);
        }
        block=(u8 *)(0x70000000U+bank);nv=block[2];
        for(i=0;i<3;i++){first[i]=wall_add(wall_mul(dir[i],wall_float(records[record].fraction)),start[i]);last[i]=wall_add(wall_mul(dir[i],wall_float(records[record+1].fraction)),start[i]);}
        if(!nv)continue;
        for(i=0;i<3;i++){
            origin[i]=(f32)((s32)((records[record].cell>>(i*8))&255U)*4096+2048);
            a=wall_add32(body_integer(first[i]),-(s32)origin[i]);b=wall_add32(body_integer(last[i]),-(s32)origin[i]);
            low[i]=(s16)((u32)(a<b?a:b)-1U);high[i]=(s16)((u32)(a>b?a:b)+1U);
            relative_start[i]=wall_sub(start[i],origin[i]);relative_end[i]=wall_sub(end[i],origin[i]);
        }
        payload=block+4;pending[0]=*(volatile u32 *)payload;pending[1]=*(volatile u32 *)(payload+4);index=0;
        do{
            next[0]=*(volatile u32 *)(payload+8);next[1]=*(volatile u32 *)(payload+12);
            for(j=0;j<2;j++){
                s32 decoded[3];decoded[0]=wall_signed(pending[j],10)*64;decoded[1]=wall_signed(pending[j]>>10,10)*64;decoded[2]=wall_signed(pending[j]>>20,12)*16;
                vertices[index].word[3]=0;for(i=0;i<3;i++){vertices[index].value[i]=(f32)decoded[i];vertices[index].word[3]|=wall_clip(decoded[i],low[i],high[i])<<(i*8);}index++;
            }
            payload+=8;pending[0]=next[0];pending[1]=next[1];
        }while((s32)index<(s32)nv);
        nv=block[2];nf=block[0];face=block+4+nv*4;limit=face+nf*4;
        wall_faces(vertices,face,limit,relative_start,relative_end,dir,origin,flags,root,&winner);
        nq=block[3];if(nq){quadCursor=limit;quad=face;for(i=0;i<nq;i++){u8 third=quad[2],fourth=*quadCursor++;quad[1]=third;quad[2]=fourth;quad+=4;}wall_faces(vertices,face,face+nq*4,relative_start,relative_end,dir,origin,flags,root,&winner);}
    }
    wall_wait();wall_dynamic((u32)ignoredActor,flags,(u32)generation,start,end,dir,records,&winner);
    wall_wait();if(!winner.metadata)goto miss;
    for(i=0;i<3;i++){
        output[0].value[i]=wall_mul(winner.position[i],1.f/1024.f);output[1].value[i]=winner.normal[i];
        for(j=0;j<3;j++)output[j+2].value[i]=wall_mul(winner.vertex[j][i],1.f/1024.f);
    }
    for(i=0;i<5;i++)*(BodyQuad *)(packet+(i==0?32:i==1?64:80+(i-2)*16))=output[i].quad;
    *(u32 *)(packet+24)=winner.object;*(u32 *)(packet+28)=winner.metadata;
    wall_cache((u8 *)winner.object,cache,output);
    __asm__ volatile("vcallms 0xF00\n\tvcallms 0xF40":::"memory");return 1;
miss:
    __asm__ volatile("vcallms 0xF00\n\tvcallms 0xF40":::"memory");return 0;
}
#endif
