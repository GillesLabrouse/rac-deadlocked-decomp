#include "dreadzone_actor_slope.h"
#include "dreadzone_actor_velocity.h"
#include "dreadzone_input.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static int span(const void *p,uintptr_t n){return p && (uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *p,u32 n,const void *q,u32 m) {
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;return a>=b?a-b<m:b-a<n;
}
static void region(const DreadzoneActorInputContext *c,u32 i,DreadzoneActorInputMemory *v) {
    if(i)*v=c->views[i-1];else *v=(DreadzoneActorInputMemory){c->record,c->record_guest,c->record_bytes,1};
}
static int valid(const DreadzoneActorInputContext *c) {
    DreadzoneActorInputMemory a,b;u32 i,j,n;
    if(!span(c,sizeof(*c)) || c->view_count>64)return 0;
    n=c->view_count*(u32)sizeof(*c->views);
    if(n && (!span(c->views,n) || overlap(c,sizeof(*c),c->views,n)))return 0;
    if(c->record_bytes<0x31F0 || (c->record_guest&15))return 0;
    for(i=0;i<=c->view_count;++i) {
        region(c,i,&a);
        if(!a.bytes || !span(a.storage,a.bytes) || a.guest_base>0xFFFFFFFFU-(a.bytes-1) ||
           (a.writable!=0 && a.writable!=1) || overlap(a.storage,a.bytes,c,sizeof(*c)) ||
           (n && overlap(a.storage,a.bytes,c->views,n)))return 0;
        for(j=0;j<i;++j) {
            region(c,j,&b);
            if(overlap(a.storage,a.bytes,b.storage,b.bytes) ||
               (a.guest_base>=b.guest_base?a.guest_base-b.guest_base<b.bytes:
                                          b.guest_base-a.guest_base<a.bytes))return 0;
        }
    }
    return 1;
}
static int normal(f32 value) {
    u32 bits;memcpy(&bits,&value,4);bits&=0x7FFFFFFFU;
    return !bits || (bits>=0x00800000U && bits<0x7F800000U);
}
static int bounded(f32 value,f32 floor) {
    return normal(value) && fabsf(value)<=16.f && (value==0.f || fabsf(value)>=floor);
}
static int angle_tables(const DreadzoneActorInputContext *ctx) {
    static const f32 coefficients[8]={
        0.999999344348907470703125f,-0.33329856395721435546875f,
        0.1994653642177581787109375f,-0.13908533751964569091796875f,
        0.096420042216777801513671875f,-0.05590988695621490478515625f,
        0.0218612290918827056884765625f,-0.0040540578775107860565185546875f
    };
    static const f32 quadrants[16]={
        1.f,0.f,-1.f,1.57079637050628662109375f,
        -1.f,0.f,1.f,-1.57079637050628662109375f,
        -1.f,3.1415927410125732421875f,1.f,1.57079637050628662109375f,
        1.f,-3.1415927410125732421875f,-1.f,-1.57079637050628662109375f
    };
    const u8 *a=dreadzone_actor_input_read(ctx,0x003B9540,sizeof(coefficients));
    const u8 *b=dreadzone_actor_input_read(ctx,0x003B9580,sizeof(quadrants));
    if(!a || !b)return DREADZONE_SLOPE_NEEDS_MEMORY;
    return memcmp(a,coefficients,sizeof(coefficients)) || memcmp(b,quadrants,sizeof(quadrants))?
        DREADZONE_SLOPE_UNSUPPORTED_TABLES:DREADZONE_SLOPE_COMPLETED;
}
int dreadzone_actor_slope_bytes(const DreadzoneActorInputContext *ctx,u8 *record) {
    VelocityVector normal_vector,velocity;const u8 *time_bytes;
    f32 value,planar_length,angle,scale,time,smaller,larger;u32 i;s16 contact;int status;
    volatile f32 negative_x,product_x,product_y,dot,product,rate,offset;
    volatile f32 numerator,denominator,ratio;
    const f32 component_floor=0.0000000000009094947017729282379150390625f;
    const f32 scalar_floor=0.00000095367431640625f;
    memcpy(&contact,record+0x2FE,2);
    if(!contact)return DREADZONE_SLOPE_COMPLETED;
    value=dreadzone_velocity_float(record+0x2CC);
    if(!normal(value))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    if(!(value<0.15f))return DREADZONE_SLOPE_COMPLETED;
    value=dreadzone_velocity_float(record+0x158);
    if(!normal(value))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    if(!(value>0.f) || !record[0x226])return DREADZONE_SLOPE_COMPLETED;
    value=dreadzone_velocity_float(record+0x1D8);
    if(!normal(value))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    if(!(value>0.f))return DREADZONE_SLOPE_COMPLETED;
    memcpy(&normal_vector,record+0x1D0,16);
    /* Terrain packets contain raw cross-product normals, not unit vectors. */
    for(i=0;i<3;++i)if(!normal(normal_vector.v[i]) || fabsf(normal_vector.v[i])>1073741824.f ||
        (normal_vector.v[i]!=0.f && fabsf(normal_vector.v[i])<component_floor))
        return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    planar_length=dreadzone_velocity_length(&normal_vector,2);
    if(!normal(planar_length))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    status=angle_tables(ctx);if(status<0)return status;
    smaller=normal_vector.v[2]<planar_length?normal_vector.v[2]:planar_length;
    larger=normal_vector.v[2]>planar_length?normal_vector.v[2]:planar_length;
    numerator=smaller-larger;denominator=smaller+larger;ratio=numerator/denominator;
    /* The actual polynomial includes ratio^15. Bound every selected power
     * away from subnormal arithmetic without claiming a physical flush model.
     * The zero-ratio path is exact; smaller nonzero ratios are refused. */
    if(!normal(ratio) || (ratio!=0.f && fabsf(ratio)<0.0078125f))
        return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    angle=func_L01_0055D708(normal_vector.v[2],planar_length);
    if(!normal(angle))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    if(!(angle>=0.872664630413055419921875f))return DREADZONE_SLOPE_APPROXIMATE;
    normal_vector.v[2]=0.f;
    if(!dreadzone_velocity_scale(&normal_vector,1.f,3,1))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    memcpy(&velocity,record+0x130,16);
    /* Only X/Y participate until the genuine positive-dot branch is selected. */
    for(i=0;i<2;++i)if(!bounded(velocity.v[i],component_floor))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    negative_x=-velocity.v[0];product_y=velocity.v[1]*normal_vector.v[1];
    product_x=negative_x*normal_vector.v[0];dot=product_x-product_y;
    if(!normal(product_x) || !normal(product_y) || !normal(dot))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    if(!(dot>0.f))return DREADZONE_SLOPE_APPROXIMATE;
    if(!bounded(velocity.v[2],component_floor))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    scale=dreadzone_velocity_float(record+0x25C4);
    if(!bounded(scale,scalar_floor))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    for(i=0;i<3;++i) {
        product=normal_vector.v[i]*dot;
        if(!normal(product))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
        value=velocity.v[i]+product;
        if(!normal(value))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
        velocity.v[i]=value;
    }
    memcpy(record+0x130,&velocity,16);
    time_bytes=dreadzone_actor_input_read(ctx,0x0021DDF8,4);
    if(!time_bytes)return DREADZONE_SLOPE_NEEDS_MEMORY;
    time=dreadzone_velocity_float(time_bytes);
    if(!bounded(time,scalar_floor))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    rate=time*54.f;offset=rate*scale;
    if(!normal(rate) || !normal(offset))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    offset=offset+offset;
    if(!normal(offset))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    value=velocity.v[2]-offset;
    if(!normal(value))return DREADZONE_SLOPE_UNSUPPORTED_FLOAT;
    velocity.v[2]=value;memcpy(record+0x130,&velocity,16);
    return DREADZONE_SLOPE_APPROXIMATE;
}
int dreadzone_actor_slope_0067D4E8(const DreadzoneActorInputContext *ctx) {
    DreadzoneActorInputContext shadow_context;
    DreadzoneActorInputMemory original[65],views[64];
    u8 *copies[65]={0};u32 total=0,i;int status;
    if(!valid(ctx))return DREADZONE_SLOPE_INVALID;
    for(i=0;i<=ctx->view_count;++i) {
        region(ctx,i,original+i);
        if(original[i].bytes>16U*1024U*1024U-total)return DREADZONE_SLOPE_TRANSACTION_MEMORY;
        total+=original[i].bytes;
    }
    for(i=0;i<=ctx->view_count;++i) {
        copies[i]=(u8 *)malloc(original[i].bytes);
        if(!copies[i]){status=DREADZONE_SLOPE_TRANSACTION_MEMORY;goto discard;}
        memcpy(copies[i],original[i].storage,original[i].bytes);
        if(i){views[i-1]=original[i];views[i-1].storage=copies[i];}
    }
    shadow_context=*ctx;shadow_context.record=copies[0];shadow_context.views=views;
    status=dreadzone_actor_slope_bytes(&shadow_context,shadow_context.record);
    if(status>=0)for(i=0;i<=ctx->view_count;++i)if(original[i].writable)
        memcpy(original[i].storage,copies[i],original[i].bytes);
discard:
    for(i=0;i<=ctx->view_count;++i)free(copies[i]);
    return status;
}
