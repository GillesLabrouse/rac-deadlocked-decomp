#include "dreadzone_actor_direction.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
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
f32 dreadzone_direction_float(const u8 *p) {f32 f;memcpy(&f,p,4);return f;}
void dreadzone_direction_store_float(u8 *p,f32 f) {memcpy(p,&f,4);}
static int normal(f32 f) {
    u32 bits;memcpy(&bits,&f,4);bits&=0x7FFFFFFFU;
    return !bits || (bits>=0x00800000U && bits<0x7F800000U);
}
static int bounded(f32 f,f32 maximum) {
    return normal(f) && fabsf(f)<=maximum && (f==0.f || fabsf(f)>=0.00000095367431640625f);
}
static int angle_bounded(f32 f,f32 maximum) {
    return normal(f) && fabsf(f)<=maximum && (f==0.f || fabsf(f)>=0.0000000000009094947017729282379150390625f);
}
int dreadzone_direction_valid_float(f32 f,f32 maximum) {return bounded(f,maximum);}
f32 dreadzone_direction_min(f32 a,f32 b) {
    return a<b || (a==b && signbit(a))?a:b;
}
f32 dreadzone_direction_max(f32 a,f32 b) {
    return a>b || (a==b && !signbit(a))?a:b;
}
void dreadzone_direction_matrix(const u8 *matrix,const DirectionVector *in,DirectionVector *out) {
    f32 columns[12];u32 lane,i;volatile f32 x,y,z,xy,sum;
    memcpy(columns,matrix,sizeof(columns));
    for(i=0;i<12;++i) if(!bounded(columns[i],16.f)) {
        for(lane=0;lane<4;++lane) out->v[lane]=NAN;return;
    }
    for(lane=0;lane<4;++lane) {
        x=columns[lane]*in->v[0];y=columns[4+lane]*in->v[1];z=columns[8+lane]*in->v[2];
        xy=x+y;sum=xy+z;sum=sum+(lane==3?1.f:0.f)*in->v[3];out->v[lane]=sum;
        if(!normal(x) || !normal(y) || !normal(z) || !normal(xy) || !normal(sum)) out->v[lane]=NAN;
    }
}
f32 dreadzone_direction_length_xy(const DirectionVector *v) {
    volatile f32 x=v->v[0]*v->v[0],y=v->v[1]*v->v[1],sum=x+y;
    if(!normal(x) || !normal(y) || !normal(sum) ||
       (v->v[0]!=0.f && x==0.f) || (v->v[1]!=0.f && y==0.f)) return NAN;
    return sqrtf(sum);
}
void dreadzone_direction_limit_xy(DirectionVector *v) {
    f32 length=dreadzone_direction_length_xy(v),factor;u32 i;
    if(!isfinite(length)) {v->v[0]=NAN;return;}
    if(length>1.f) {factor=1.f/length;for(i=0;i<3;++i) v->v[i]*=factor;}
}
void dreadzone_direction_normalize(DirectionVector *v) {
    volatile f32 x=v->v[0]*v->v[0],y=v->v[1]*v->v[1],z=v->v[2]*v->v[2],xy=x+y,sum=xy+z;
    f32 factor;u32 i;
    if(!normal(x) || !normal(y) || !normal(z) || !normal(xy) || !normal(sum) ||
       (v->v[0]!=0.f && x==0.f) || (v->v[1]!=0.f && y==0.f) || (v->v[2]!=0.f && z==0.f)) {
        v->v[0]=NAN;return;
    }
    if(sum==0.f) {v->v[0]=v->v[1]=v->v[2]=0.f;return;}
    factor=1.f/sqrtf(sum);for(i=0;i<3;++i) v->v[i]*=factor;
}
static int run(const DreadzoneActorInputContext *ctx,int turning,s32 mode,f32 a,f32 b,f32 c) {
    DreadzoneActorInputContext tx;DreadzoneActorInputMemory original[65],copies[64];
    u8 *buffers[65]={0};const u8 *flag;u32 i,total=0;int status;
    if(!valid(ctx)) return DREADZONE_DIRECTION_INVALID;
    if((turning && (mode< -1 || mode>2 || !bounded(a,1.f) || !bounded(b,1.f) ||
                   !bounded(c,1.f) || a<0.f || b<0.f || c<0.f)) ||
       (!turning && (!bounded(a,16.f) || a<0.f))) return DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;
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
    flag=dreadzone_actor_input_read(&tx,0x0021E878,4);
    if(!flag) {status=DREADZONE_DIRECTION_NEEDS_MEMORY;goto discard;}
    if(dreadzone_actor_input_word(flag) || tx.record[0x2659]) {status=DREADZONE_DIRECTION_UNSUPPORTED_MODE;goto discard;}
    if(turning) {
        if(!angle_bounded(dreadzone_direction_float(tx.record+0xB8),3.1415927410125732421875f) ||
           !angle_bounded(dreadzone_direction_float(tx.record+0x17A0),3.1415927410125732421875f) ||
           !bounded(dreadzone_direction_float(tx.record+0x17A4),6.283185482025146484375f)) {
            status=DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;goto discard;
        }
        status=func_L01_0067CFD0(&tx,tx.record,mode,a,b,c);
        if(status>=0 && (!normal(dreadzone_direction_float(tx.record+0xB8)) ||
                         !normal(dreadzone_direction_float(tx.record+0x17A4)) ||
                         !normal(dreadzone_direction_float(tx.record+0x17A8)))) status=DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;
    } else {
        f32 x=dreadzone_direction_float(tx.record+0x120),y=dreadzone_direction_float(tx.record+0x124);
        if(!bounded(x,1.f) || !bounded(y,1.f) ||
           !angle_bounded(dreadzone_direction_float(tx.record+(x==0.f && y==0.f?0xB8:0x2CE8)),3.1415927410125732421875f)) {
            status=DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;goto discard;
        }
        status=func_L01_0067C5F8(&tx,tx.record,mode,a);
        if(status>=0) {
            if(!normal(dreadzone_direction_float(tx.record+0x17A0)) ||
               !normal(dreadzone_direction_float(tx.record+0x1780))) status=DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;
            for(i=0;i<4;++i) if(!normal(dreadzone_direction_float(tx.record+0x1790+i*4))) status=DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;
        }
    }
    if(status>=0) for(i=0;i<=ctx->view_count;++i) if(original[i].writable) memcpy(original[i].storage,buffers[i],original[i].bytes);
discard:
    for(i=0;i<=ctx->view_count;++i) free(buffers[i]);return status;
}
int dreadzone_actor_direction_0067C5F8(const DreadzoneActorInputContext *c,s32 mode,f32 scale) {return run(c,0,mode,scale,0.f,0.f);}
int dreadzone_actor_turn_0067CFD0(const DreadzoneActorInputContext *c,s32 mode,f32 force,f32 damping,f32 maximum) {return run(c,1,mode,force,damping,maximum);}
