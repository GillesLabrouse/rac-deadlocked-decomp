#include "dreadzone_actor_speed_target.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Speed target requires IEEE binary32 host floats
#endif
typedef char target_float_size[sizeof(f32)==4?1:-1];
static int span(const void *p,uintptr_t n) {return p && (uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *p,u32 n,const void *q,u32 m) {
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;return a>=b?a-b<m:b-a<n;
}
static void region(const DreadzoneActorInputContext *c,u32 i,DreadzoneActorInputMemory *v) {
    if(i) *v=c->views[i-1];else *v=(DreadzoneActorInputMemory){c->record,c->record_guest,c->record_bytes,1};
}
static int valid(const DreadzoneActorInputContext *c) {
    DreadzoneActorInputMemory a,b;u32 i,j,n;
    if(!span(c,sizeof(*c)) || c->view_count>64) return 0;
    n=c->view_count*(u32)sizeof(*c->views);
    if(n && (!span(c->views,n) || overlap(c,sizeof(*c),c->views,n))) return 0;
    if(c->record_bytes<0x31F0 || (c->record_guest&15)) return 0;
    for(i=0;i<=c->view_count;++i) {
        region(c,i,&a);
        if(!a.bytes || !span(a.storage,a.bytes) || a.guest_base>0xFFFFFFFFU-(a.bytes-1) ||
           (a.writable!=0 && a.writable!=1) || overlap(a.storage,a.bytes,c,sizeof(*c)) ||
           (n && overlap(a.storage,a.bytes,c->views,n))) return 0;
        for(j=0;j<i;++j) {
            region(c,j,&b);
            if(overlap(a.storage,a.bytes,b.storage,b.bytes) ||
               (a.guest_base>=b.guest_base?a.guest_base-b.guest_base<b.bytes:
                                           b.guest_base-a.guest_base<a.bytes)) return 0;
        }
    }
    return 1;
}
static int normal(f32 f) {
    u32 bits;memcpy(&bits,&f,4);bits&=0x7FFFFFFFU;
    return !bits || (bits>=0x00800000U && bits<0x7F800000U);
}
int dreadzone_speed_target_scalar(f32 value,f32 *out) {
    if(!dreadzone_direction_valid_float(value,16.f)) return DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;
    *out=value;return DREADZONE_DIRECTION_APPROXIMATE;
}
int dreadzone_speed_target_read(const DreadzoneActorInputContext *ctx,u32 address,f32 *out) {
    const u8 *p=dreadzone_actor_input_read(ctx,address,4);
    if(!p) return DREADZONE_DIRECTION_NEEDS_MEMORY;
    return dreadzone_speed_target_scalar(dreadzone_direction_float(p),out);
}
int dreadzone_speed_target_product(f32 a,f32 b,f32 *out) {
    volatile f32 result=a*b;
    if(!normal(result)) return DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;
    *out=result;return DREADZONE_DIRECTION_APPROXIMATE;
}
static int run(const DreadzoneActorInputContext *ctx,int turning) {
    DreadzoneActorInputContext tx;DreadzoneActorInputMemory original[65],copies[64];
    u8 *buffers[65]={0};u32 i,total=0;int status;
    if(!valid(ctx)) return DREADZONE_DIRECTION_INVALID;
    for(i=0;i<=ctx->view_count;++i) {
        region(ctx,i,original+i);
        if(original[i].bytes>16U*1024U*1024U-total) return DREADZONE_DIRECTION_TRANSACTION_MEMORY;
        total+=original[i].bytes;
    }
    for(i=0;i<=ctx->view_count;++i) {
        buffers[i]=(u8 *)malloc(original[i].bytes);
        if(!buffers[i]) {status=DREADZONE_DIRECTION_TRANSACTION_MEMORY;goto discard;}
        memcpy(buffers[i],original[i].storage,original[i].bytes);
        if(i) {copies[i-1]=original[i];copies[i-1].storage=buffers[i];}
    }
    tx=*ctx;tx.record=buffers[0];tx.views=copies;
    status=turning?func_L01_0067D100(&tx):func_L01_0067CAF8(&tx);
    if(status>=0) for(i=0;i<=ctx->view_count;++i) if(original[i].writable) memcpy(original[i].storage,buffers[i],original[i].bytes);
 discard:
    for(i=0;i<=ctx->view_count;++i) free(buffers[i]);return status;
}
int dreadzone_actor_speed_target_0067CAF8(const DreadzoneActorInputContext *ctx) {return run(ctx,0);}
int dreadzone_actor_state_turn_0067D100(const DreadzoneActorInputContext *ctx) {return run(ctx,1);}
