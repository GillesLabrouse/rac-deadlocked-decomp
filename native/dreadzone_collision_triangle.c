#include "dreadzone_collision_triangle.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
static int valid(float value) {return isfinite(value)&&(value==0.0f || fabsf(value)>=FLT_MIN);}
static u32 bits(float value) {u32 word;memcpy(&word,&value,4);return word;}
static int add(float a,float b,float *out) {volatile float v=a+b;*out=v;return valid(v);}
static int sub(float a,float b,float *out) {volatile float v=a-b;*out=v;return valid(v);}
static int mul(float a,float b,float *out) {volatile float v=a*b;*out=v;return valid(v);}
static int divide(float a,float b,float *out,int saturate,DreadzoneCollisionTriangleQStatus *status) {
    volatile float value;
    status->division_executed=1;
    if(b==0.0f) {
        u32 word;
        if(!saturate)return 0;
        word=0x7F7FFFFFU|((bits(a)^bits(b))&0x80000000U);
        memcpy(out,&word,4);
        status->q_bits=word;status->exception_bits=a==0.0f?0x10U:0x20U;
        return 1;
    }
    value=a/b;*out=value;status->q_bits=bits(value);return valid(value);
}
static int vector_sub(const float a[3],const float b[3],float out[3]) {
    u32 i;for(i=0;i<3;i++)if(!sub(a[i],b[i],out+i))return 0;return 1;
}
static int cross(const float a[3],const float b[3],float out[3]) {
    u32 i;float acc,product;
    for(i=0;i<3;i++) {
        if(!mul(a[(i+1)%3],b[(i+2)%3],&acc) ||
           !mul(b[(i+1)%3],a[(i+2)%3],&product) || !sub(acc,product,out+i))return 0;
    }
    return 1;
}
static int vector_mul(const float a[3],const float b[3],float out[3]) {
    u32 i;for(i=0;i<3;i++)if(!mul(a[i],b[i],out+i))return 0;return 1;
}
static int sum(const float v[3],float *out) {
    float acc,product;
    return add(v[0],v[1],&acc)&&mul(1.0f,v[2],&product)&&add(acc,product,out);
}
#define REQUIRE(x) do {if(!(x))return DREADZONE_TRIANGLE_UNSUPPORTED;}while(0)
static int triangle_calculate(const DreadzoneCollisionTriangleRequest *request,
                              DreadzoneCollisionTriangleContact *contact,int saturate,
                              DreadzoneCollisionTriangleQStatus *status) {
    DreadzoneCollisionTriangleRequest r;DreadzoneCollisionTriangleContact result;
    float edge2[3],edge3[3],normal[3],plane_start[3],plane_end[3],start_delta[3];
    float intersection[3],edge9[3],edge10[3],side[3][3],direction_product[3],start_product[3],end_product[3];
    float denominator,start_dot,end_dot,q,test[3],nearest;
    float product,acc;u32 i,j,common,front;
    if(request==NULL || contact==NULL || (uintptr_t)request>UINTPTR_MAX-sizeof(r) ||
       (uintptr_t)contact>UINTPTR_MAX-sizeof(result))return DREADZONE_TRIANGLE_INVALID;
    memset(status,0,sizeof(*status));
    memcpy(&r,request,sizeof(r));
    if(!valid(r.best_fraction)||r.best_fraction<0 || r.best_fraction>1 || r.surface>255)
        return DREADZONE_TRIANGLE_UNSUPPORTED;
    for(i=0;i<3;i++) {
        if(r.clip_flags[i]&~0x00030303U)return DREADZONE_TRIANGLE_UNSUPPORTED;
        REQUIRE(valid(r.cell_origin[i])&&valid(r.start_relative[i])&&valid(r.end_relative[i])&&valid(r.direction[i]));
        for(j=0;j<3;j++)REQUIRE(valid(r.vertex[j][i]));
    }
    common=r.clip_flags[0]&r.clip_flags[1]&r.clip_flags[2];
    if((s32)common>0)return DREADZONE_TRIANGLE_MISS;
    if((r.effective_flags&0x90U)&r.surface)return DREADZONE_TRIANGLE_MISS;
    front=((r.effective_flags&0x20U)==0)|(((r.effective_flags>>8)^r.surface)&15U);
    REQUIRE(vector_sub(r.vertex[1],r.vertex[0],edge2));
    REQUIRE(vector_sub(r.vertex[2],r.vertex[0],edge3));
    REQUIRE(vector_sub(r.vertex[0],r.start_relative,plane_start));
    REQUIRE(vector_sub(r.vertex[0],r.end_relative,plane_end));
    REQUIRE(cross(edge3,edge2,normal));
    /* The direction multiply occupies the front-filter branch delay slot. */
    REQUIRE(vector_mul(r.direction,normal,direction_product));
    if(!front)return DREADZONE_TRIANGLE_MISS;
    REQUIRE(vector_mul(plane_start,normal,start_product));
    REQUIRE(vector_mul(plane_end,normal,end_product));
    REQUIRE(vector_sub(r.start_relative,r.vertex[0],start_delta));
    REQUIRE(sum(direction_product,&denominator));
    REQUIRE(sum(start_product,&start_dot));
    REQUIRE(sum(end_product,&end_dot));
    REQUIRE(divide(start_dot,denominator,&q,saturate,status));
    for(i=0;i<3;i++)REQUIRE(mul(start_delta[i],1.0f,start_delta+i));
    if(!(r.effective_flags&0x1000U)) {
        if(!(bits(start_dot)&0x80000000U) || (s32)bits(end_dot)<=0)return DREADZONE_TRIANGLE_MISS;
    }
    if(!((bits(start_dot)^bits(end_dot))&0x80000000U))return DREADZONE_TRIANGLE_MISS;
    for(i=0;i<3;i++) {
        acc=start_delta[i];
        REQUIRE(mul(r.direction[i],q,&product));
        REQUIRE(add(acc,product,intersection+i));
        REQUIRE(add(r.vertex[0][i],r.cell_origin[i],result.vertices_scaled[0]+i));
    }
    REQUIRE(vector_sub(edge3,edge2,edge9));
    REQUIRE(vector_sub(intersection,edge2,edge10));
    /* First OPMSUB is the positive-Q branch delay slot. */
    REQUIRE(cross(intersection,edge2,side[0]));
    if((s32)bits(q)<=0)return DREADZONE_TRIANGLE_MISS;
    /* All three edge results and the nearest difference are computed before
     * any edge rejection in the actual instruction stream. */
    REQUIRE(cross(edge3,intersection,side[1]));
    REQUIRE(cross(edge10,edge9,side[2]));
    for(i=0;i<3;i++)REQUIRE(vector_mul(side[i],normal,side[i]));
    REQUIRE(sub(r.best_fraction,q,&nearest));
    for(i=0;i<3;i++)REQUIRE(sum(side[i],test+i));
    for(i=0;i<3;i++)if(bits(test[i])&0x80000000U)return DREADZONE_TRIANGLE_MISS;
    if((s32)bits(nearest)<=0)return DREADZONE_TRIANGLE_MISS;
    /* Exact stores selected by the original success path, XYZ only. */
    result.hit_metadata=r.surface+0x1000U;
    result.source_guest=r.source_guest;
    result.fraction=q;
    for(i=0;i<3;i++) {
        REQUIRE(add(intersection[i],result.vertices_scaled[0][i],result.position_scaled+i));
        REQUIRE(add(normal[i],0.0f,result.normal+i));
        REQUIRE(add(edge2[i],result.vertices_scaled[0][i],result.vertices_scaled[1]+i));
        REQUIRE(add(edge3[i],result.vertices_scaled[0][i],result.vertices_scaled[2]+i));
        REQUIRE(add(0.0f,result.vertices_scaled[0][i],result.vertices_scaled[0]+i));
    }
    memcpy(contact,&result,sizeof(result));return DREADZONE_TRIANGLE_APPROXIMATE_CONTACT;
}
int dreadzone_collision_triangle(const DreadzoneCollisionTriangleRequest *request,
                                DreadzoneCollisionTriangleContact *contact) {
    DreadzoneCollisionTriangleQStatus status;
    return triangle_calculate(request,contact,0,&status);
}
int dreadzone_collision_triangle_with_q(const DreadzoneCollisionTriangleRequest *request,
                                       DreadzoneCollisionTriangleContact *contact,
                                       DreadzoneCollisionTriangleQStatus *q_status) {
    DreadzoneCollisionTriangleQStatus status;int result;
    uintptr_t a=(uintptr_t)contact,b=(uintptr_t)q_status;
    if(contact==NULL || q_status==NULL || a>UINTPTR_MAX-sizeof(*contact) ||
       b>UINTPTR_MAX-sizeof(*q_status) ||
       (a<b+sizeof(*q_status) && b<a+sizeof(*contact)))return DREADZONE_TRIANGLE_INVALID;
    result=triangle_calculate(request,contact,1,&status);
    if(result>=0)memcpy(q_status,&status,sizeof(status));
    return result;
}
