#include "dreadzone_actor_walk_parameters.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Walk parameter arithmetic requires binary32 floats
#endif
static int span(const void *p,uintptr_t bytes) {
    return p && (uintptr_t)p<=UINTPTR_MAX-bytes;
}
static int overlap(const void *p,u32 n,const void *q,u32 m) {
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;
    return a>=b?a-b<m:b-a<n;
}
static void region(const DreadzoneActorInputContext *c,u32 i,DreadzoneActorInputMemory *v) {
    if(i)*v=c->views[i-1];else*v=(DreadzoneActorInputMemory){c->record,c->record_guest,c->record_bytes,1};
}
static int valid(const DreadzoneActorInputContext *c) {
    DreadzoneActorInputMemory a,b;u32 i,j,n;
    if(!span(c,sizeof(*c))||c->view_count>64)return 0;
    n=c->view_count*(u32)sizeof(*c->views);
    if(n && (!span(c->views,n)||overlap(c,sizeof(*c),c->views,n)))return 0;
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
s32 dreadzone_walk_parameters_word(const u8 *p) {
    s32 result;memcpy(&result,p,4);return result;
}
static int normal(f32 value) {
    u32 bits;memcpy(&bits,&value,4);bits&=0x7FFFFFFFU;
    return !bits || (bits>=0x00800000U && bits<0x7F800000U);
}
static f32 checked(DreadzoneWalkParametersRun *run,const u8 *p,f32 floor,f32 maximum,int nonnegative) {
    f32 value;memcpy(&value,p,4);
    if(!normal(value) || fabsf(value)>maximum || (value!=0.f && fabsf(value)<floor) ||
       (nonnegative && value<0.f)) {
        if(run->status>=0)run->status=DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;
        return 0.f;
    }
    return value;
}
f32 dreadzone_walk_parameters_float(DreadzoneWalkParametersRun *run,u32 offset) {
    f32 floor=0.00000095367431640625f,maximum=16.f;int nonnegative=1;
    switch(offset) {
        case 0x17A8:case 0xB8:case 0x5F8:
            floor=0.0000000000009094947017729282379150390625f;
            maximum=3.1415927410125732421875f;nonnegative=0;break;
        case 0x17A4:
            floor=0.0000000000009094947017729282379150390625f;
            maximum=6.283185482025146484375f;nonnegative=0;break;
        case 0x2E08:maximum=1.f;break;
        case 0x880:floor=0.f;nonnegative=0;break;
        case 0x25C4:case 0x1B4:break;
        default:run->status=DREADZONE_DIRECTION_UNSUPPORTED_MODE;return 0.f;
    }
    return checked(run,run->context->record+offset,floor,maximum,nonnegative);
}
f32 dreadzone_walk_parameters_global(DreadzoneWalkParametersRun *run,u32 address) {
    const u8 *p=dreadzone_actor_input_read(run->context,address,4);
    if(!p) {if(run->status>=0)run->status=DREADZONE_DIRECTION_NEEDS_MEMORY;return 0.f;}
    return checked(run,p,0.00000095367431640625f,16.f,1);
}
void dreadzone_walk_parameters_store(DreadzoneWalkParametersRun *run,u32 offset,f32 value) {
    if(!normal(value) && run->status>=0)run->status=DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;
    memcpy(run->context->record+offset,&value,4);
}
f32 dreadzone_walk_parameters_divide(DreadzoneWalkParametersRun *run,f32 numerator,f32 denominator) {
    volatile f32 value;
    if(denominator==0.f) {
        if(run->status>=0)run->status=DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;
        return 0.f;
    }
    value=numerator/denominator;
    if(!normal(value) && run->status>=0)run->status=DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;
    return value;
}
s32 dreadzone_walk_parameters_history(DreadzoneWalkParametersRun *run,u32 input) {
    s32 result=0;int status=dreadzone_direction_history(run->context,0x28,2,0,input,&result);
    if(status<0 && run->status>=0)run->status=status;
    return result;
}
int dreadzone_actor_walk_parameters_00681B30(const DreadzoneActorInputContext *ctx) {
    DreadzoneActorInputContext tx;DreadzoneActorInputMemory original[65],views[64];
    DreadzoneWalkParametersRun run;u8 *buffers[65]={0};u32 i,count,total=0;int status;
    if(!valid(ctx))return DREADZONE_DIRECTION_INVALID;
    if(dreadzone_walk_parameters_word(ctx->record+0x25CC)!=2 || ctx->record[0x2659])
        return DREADZONE_DIRECTION_UNSUPPORTED_MODE;
    count=ctx->view_count;
    for(i=0;i<=count;++i) {
        region(ctx,i,original+i);
        if(original[i].bytes>16U*1024U*1024U-total)return DREADZONE_DIRECTION_TRANSACTION_MEMORY;
        total+=original[i].bytes;
    }
    for(i=0;i<=count;++i) {
        buffers[i]=(u8 *)malloc(original[i].bytes);
        if(!buffers[i]) {status=DREADZONE_DIRECTION_TRANSACTION_MEMORY;goto discard;}
        memcpy(buffers[i],original[i].storage,original[i].bytes);
        if(i){views[i-1]=original[i];views[i-1].storage=buffers[i];}
    }
    tx=*ctx;tx.record=buffers[0];tx.views=views;run.context=&tx;run.status=DREADZONE_DIRECTION_APPROXIMATE;
    status=dreadzone_walk_parameters_execute(&run);
    if(status>=0)for(i=0;i<=count;++i)if(original[i].writable)
        memcpy(original[i].storage,buffers[i],original[i].bytes);
discard:
    for(i=0;i<=count;++i)free(buffers[i]);return status;
}
