#include "dreadzone_wall_helpers.h"
#include "dreadzone_input.h"
#include <math.h>
#include <string.h>

static int span(const void *p,u32 n){return p && (uintptr_t)p<=UINTPTR_MAX-n;}
static int valid(f32 x){u32 b;memcpy(&b,&x,4);b&=0x7FFFFFFFU;return !b || (b>=0x00800000U && b<0x7F800000U);}
typedef struct Numeric {int good;} Numeric;
static f32 check(Numeric *n,f32 x){if(!valid(x))n->good=0;return x;}
static f32 mul(Numeric *n,f32 a,f32 b){volatile f32 x=a*b;return check(n,x);}
static f32 add(Numeric *n,f32 a,f32 b){volatile f32 x=a+b;return check(n,x);}
static f32 sub(Numeric *n,f32 a,f32 b){volatile f32 x=a-b;return check(n,x);}

/* Preflight the actual polynomial's ordered intermediates. The returned angle
 * is always computed by the genuine recovered callee, never this preflight.
 */
static void angle_domain(Numeric *n,f32 x,f32 y){
    static const f32 c[8]={0.999999344348907470703125f,-0.33329856395721435546875f,
        0.1994653642177581787109375f,-0.13908533751964569091796875f,
        0.096420042216777801513671875f,-0.05590988695621490478515625f,
        0.0218612290918827056884765625f,-0.0040540578775107860565185546875f};
    f32 a=fabsf(x),b=fabsf(y),small,large,ratio,square,fourth,eighth,odd[4],high[4],sum;
    u32 difference;int i;f32 d=sub(n,a,b);
    memcpy(&difference,&d,4);
    small=(difference>>31)?a:b;large=(difference>>31)?b:a;
    if(large<=0.f)return;
    {f32 numerator=sub(n,small,large),denominator=add(n,small,large);volatile f32 q;
        if(!n->good || denominator==0.f){n->good=0;return;}
        q=numerator/denominator;ratio=check(n,q);}
    square=mul(n,ratio,ratio);fourth=mul(n,square,square);eighth=mul(n,fourth,fourth);
    odd[0]=ratio;odd[1]=mul(n,ratio,square);odd[2]=mul(n,ratio,fourth);odd[3]=mul(n,odd[2],square);
    for(i=0;i<4;++i){high[i]=mul(n,mul(n,odd[i],eighth),c[i+4]);odd[i]=mul(n,odd[i],c[i]);}
    sum=add(n,odd[0],odd[1]);sum=add(n,sum,odd[2]);sum=add(n,sum,odd[3]);
    for(i=0;i<4;++i)sum=add(n,sum,high[i]);
    add(n,0.785398185253143310546875f,sum);
}

int dreadzone_wall_surface_00542FF8(const u8 *packet,u32 bytes,s32 *out){
    s32 metadata,value;
    if(bytes<0x20 || !span(packet,0x20) || !span(out,4))return DREADZONE_WALL_HELPER_INVALID;
    memcpy(&metadata,packet+0x1C,4);value=metadata<0?-1:metadata&15;if(value==15)value=-1;
    memcpy(out,&value,4);return 1;
}

int dreadzone_wall_inclination_0068AFF8(const u8 *record,u32 bytes,
    const u8 *normal,u32 normal_bytes,f32 *out){
    u8 mode;f32 vector[4],matrix[4][4],transformed[4],squares[4],radial,result;
    Numeric n={1};u32 i,k;
    if(bytes<0x265A || !span(record,0x265A) || !span(out,4))return DREADZONE_WALL_HELPER_INVALID;
    mode=record[0x2659];
    if(mode>=3){result=0.f;memcpy(out,&result,4);return 1;}
    if(normal_bytes<16 || !span(normal,16))return DREADZONE_WALL_HELPER_INVALID;
    memcpy(vector,normal,16);
    for(i=0;i<4;++i)check(&n,vector[i]);
    if(mode){
        memcpy(matrix,record+0x60,64);
        for(k=0;k<4;++k)for(i=0;i<4;++i)check(&n,matrix[k][i]);
        if(!n.good)return DREADZONE_WALL_HELPER_UNSUPPORTED;
        for(i=0;i<4;++i){
            f32 acc=mul(&n,matrix[0][i],vector[0]);
            acc=add(&n,acc,mul(&n,matrix[1][i],vector[1]));
            acc=add(&n,acc,mul(&n,matrix[2][i],vector[2]));
            transformed[i]=add(&n,acc,mul(&n,matrix[3][i],vector[3]));
        }
    }else memcpy(transformed,vector,16);
    if(!n.good)return DREADZONE_WALL_HELPER_UNSUPPORTED;
    for(i=0;i<4;++i)squares[i]=mul(&n,transformed[i],transformed[i]);
    squares[0]=add(&n,squares[0],squares[1]);
    if(!n.good)return DREADZONE_WALL_HELPER_UNSUPPORTED;
    {volatile f32 q=sqrtf(squares[0]);radial=check(&n,q);}
    radial=add(&n,0.f,radial); /* Actual VADDQ.X from immutableVF0.X. */
    angle_domain(&n,transformed[2],radial);
    if(!n.good)return DREADZONE_WALL_HELPER_UNSUPPORTED;
    result=func_L01_0055D708(transformed[2],radial);check(&n,result);
    if(!n.good)return DREADZONE_WALL_HELPER_UNSUPPORTED;
    memcpy(out,&result,4);return 1;
}
