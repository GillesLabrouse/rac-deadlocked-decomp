#include "dreadzone_actor_velocity.h"
#include "dreadzone_actor_direction.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Velocity requires IEEE binary32 host floats
#endif
typedef char velocity_float_size[sizeof(f32)==4?1:-1];
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
f32 dreadzone_velocity_float(const void *p) {f32 f;memcpy(&f,p,4);return f;}
void dreadzone_velocity_store(void *p,f32 f) {memcpy(p,&f,4);}
static int normal(f32 f) {
    u32 bits;memcpy(&bits,&f,4);bits&=0x7FFFFFFFU;
    return !bits || (bits>=0x00800000U && bits<0x7F800000U);
}
static int bounded(f32 f,f32 maximum) {
    return normal(f) && fabsf(f)<=maximum && (f==0.f || fabsf(f)>=0.00000095367431640625f);
}
f32 dreadzone_velocity_min(f32 a,f32 b) {return a<b || (a==b && signbit(a))?a:b;}
f32 dreadzone_velocity_max(f32 a,f32 b) {return a>b || (a==b && !signbit(a))?a:b;}
static f32 squared(const VelocityVector *v,u32 dimensions) {
    volatile f32 x=v->v[0]*v->v[0],y=v->v[1]*v->v[1],sum=x+y;
    if(!normal(x) || !normal(y) || !normal(sum) ||
       (v->v[0]!=0.f && x==0.f) || (v->v[1]!=0.f && y==0.f)) return NAN;
    if(dimensions==3) {
        volatile f32 z=v->v[2]*v->v[2];
        if(!normal(z) || (v->v[2]!=0.f && z==0.f)) return NAN;
        sum=sum+z;if(!normal(sum)) return NAN;
    }
    return sum;
}
f32 dreadzone_velocity_length(const VelocityVector *v,u32 dimensions) {
    f32 sum=squared(v,dimensions);return sqrtf(sum);
}
int dreadzone_velocity_scale(VelocityVector *v,f32 requested,u32 dimensions,int multiply_zero) {
    f32 sum=squared(v,dimensions),factor;u32 i;
    if(!isfinite(sum)) return 0;
    if(sum==0.f) {
        if(!multiply_zero) {v->v[0]=0.f;v->v[1]=0.f;return 1;}
        if(signbit(requested)) return 0;
        factor=FLT_MAX;
    } else {
        volatile f32 quotient=requested/sqrtf(sum);factor=quotient;
        if(!normal(factor)) return 0;
    }
    for(i=0;i<dimensions;++i) {
        volatile f32 result=v->v[i]*factor;
        if(!normal(result)) return 0;
        v->v[i]=result;
    }
    return 1;
}
static int vector_valid(const u8 *p,u32 dimensions) {
    u32 i;for(i=0;i<dimensions;++i) if(!bounded(dreadzone_velocity_float(p+i*4),16.f)) return 0;
    return 1;
}
int dreadzone_velocity_0068ABA8(const u8 *record,u8 *output,const u8 *input,f32 height) {
    int status=dreadzone_direction_projection_bytes((u8 *)record,output,input,height);
    return status<0?DREADZONE_VELOCITY_UNSUPPORTED_MODE:DREADZONE_VELOCITY_APPROXIMATE;
}
static int run(const DreadzoneActorInputContext *ctx,u32 operation,u32 output,u32 input,f32 a,f32 b) {
    DreadzoneActorInputContext tx;DreadzoneActorInputMemory original[65],copies[64];
    u8 *buffers[65]={0};const u8 *source;u8 *destination;u32 i,total=0;int status;
    if(!valid(ctx)) return DREADZONE_VELOCITY_INVALID;
    if(operation==0 && (!bounded(a,16.f) || !bounded(b,16.f) || a<0.f || b<0.f))
        return DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
    if(operation==1 && !bounded(a,100000.f)) return DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
    if(operation==3 && (!bounded(a,16.f) || a<0.f)) return DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
    if((operation==4 || operation==5) && !bounded(a,16.f)) return DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
    for(i=0;i<=ctx->view_count;++i) {
        region(ctx,i,original+i);
        if(original[i].bytes>16U*1024U*1024U-total) return DREADZONE_VELOCITY_TRANSACTION_MEMORY;
        total+=original[i].bytes;
    }
    for(i=0;i<=ctx->view_count;++i) {
        buffers[i]=(u8 *)malloc(original[i].bytes);
        if(!buffers[i]) {status=DREADZONE_VELOCITY_TRANSACTION_MEMORY;goto discard;}
        memcpy(buffers[i],original[i].storage,original[i].bytes);
        if(i) {copies[i-1]=original[i];copies[i-1].storage=buffers[i];}
    }
    tx=*ctx;tx.record=buffers[0];tx.views=copies;
    if(tx.record[0x2659]) {status=DREADZONE_VELOCITY_UNSUPPORTED_MODE;goto discard;}
    status=DREADZONE_VELOCITY_APPROXIMATE;
    if(operation==0) {
        if(!bounded(dreadzone_velocity_float(tx.record+0x1780),16.f) ||
           !bounded(dreadzone_velocity_float(tx.record+0x1784),16.f)) {
            status=DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;goto discard;
        }
        func_L01_0067D350(tx.record,a,b);
        if(!normal(dreadzone_velocity_float(tx.record+0x1784))) status=DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
    } else if(operation==1) {
        f32 heading=a<=3.1415927410125732421875f?a:dreadzone_velocity_float(tx.record+0xB8);
        if(!bounded(heading,3.1415927410125732421875f) ||
           !bounded(dreadzone_velocity_float(tx.record+0x2D4),3.1415927410125732421875f) ||
           !bounded(dreadzone_velocity_float(tx.record+0x1784),16.f)) {
            status=DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;goto discard;
        }
        status=dreadzone_velocity_0067D3A0(tx.record,a);
        if(status>=0) for(i=0;i<3;++i) if(!normal(dreadzone_velocity_float(tx.record+0x130+i*4)))
            status=DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
    } else if(operation==5) {
        if(!vector_valid(tx.record+0x1A0,2) || !vector_valid(tx.record+0x130,3)) {
            status=DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;goto discard;
        }
        status=dreadzone_velocity_006834C8(tx.record,a);
    } else {
        if((input&15) || ((operation==3 || operation==4) && (output&15))) {status=DREADZONE_VELOCITY_NEEDS_MEMORY;goto discard;}
        source=dreadzone_actor_input_read(&tx,input,16);
        destination=operation==2?tx.record+0x130:dreadzone_actor_input_write(&tx,output,16);
        if(!source || !destination) {status=DREADZONE_VELOCITY_NEEDS_MEMORY;goto discard;}
        if((operation!=4 && !vector_valid(source,operation==2?3:2)) ||
           (operation==2 && !bounded(dreadzone_velocity_float(tx.record+0x1784),16.f))) {
            status=DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;goto discard;
        }
        if(operation==2) status=dreadzone_velocity_0067D480(tx.record,source);
        else if(operation==3) status=dreadzone_velocity_0068AC58(tx.record,destination,source,a);
        else status=dreadzone_velocity_0068ABA8(tx.record,destination,source,a);
    }
    if(status>=0) for(i=0;i<=ctx->view_count;++i) if(original[i].writable) memcpy(original[i].storage,buffers[i],original[i].bytes);
discard:
    for(i=0;i<=ctx->view_count;++i) free(buffers[i]);return status;
}
int dreadzone_actor_speed_0067D350(const DreadzoneActorInputContext *c,f32 a,f32 b) {return run(c,0,0,0,a,b);}
int dreadzone_actor_velocity_0067D3A0(const DreadzoneActorInputContext *c,f32 a) {return run(c,1,0,0,a,0.f);}
int dreadzone_actor_normalized_velocity_0067D480(const DreadzoneActorInputContext *c,u32 input) {return run(c,2,0,input,0.f,0.f);}
int dreadzone_actor_planar_projection_0068AC58(const DreadzoneActorInputContext *c,u32 output,u32 input,f32 a) {return run(c,3,output,input,a,0.f);}
int dreadzone_actor_vertical_projection_0068ABA8(const DreadzoneActorInputContext *c,u32 output,u32 input,f32 a) {return run(c,4,output,input,a,0.f);}
int dreadzone_actor_accumulated_motion_006834C8(const DreadzoneActorInputContext *c,f32 a) {return run(c,5,0,0,a,0.f);}
