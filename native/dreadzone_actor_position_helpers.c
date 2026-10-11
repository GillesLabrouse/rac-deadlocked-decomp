#include "dreadzone_actor_position_helpers.h"
#include "dreadzone_angle_pair.h"
#include "dreadzone_actor_velocity.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Position helpers require binary32 floats
#endif
typedef struct PositionRun { const DreadzoneActorInputContext *ctx;int status; } PositionRun;
static int span(const void *p,uintptr_t n) {return p && (uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *p,uintptr_t n,const void *q,uintptr_t m) {
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;return a>=b?a-b<m:b-a<n;
}
static void region(const DreadzoneActorInputContext *c,u32 i,DreadzoneActorInputMemory *v) {
    if(i)*v=c->views[i-1];else*v=(DreadzoneActorInputMemory){c->record,c->record_guest,c->record_bytes,1};
}
static int valid(const DreadzoneActorInputContext *c,const DreadzonePositionHelperRequest *q,f32 *out) {
    DreadzoneActorInputMemory a,b;u32 n,i,j;
    if(!span(c,sizeof(*c)) || !span(q,sizeof(*q)) || c->view_count>64 ||
       (out && (!span(out,4) || (uintptr_t)out%4 || overlap(out,4,c,sizeof(*c)) || overlap(out,4,q,sizeof(*q)))))return 0;
    n=c->view_count*(u32)sizeof(*c->views);
    if(n && (!span(c->views,n) || overlap(c,sizeof(*c),c->views,n) ||
       overlap(q,sizeof(*q),c->views,n) || (out && overlap(out,4,c->views,n))))return 0;
    if(overlap(c,sizeof(*c),q,sizeof(*q)) || c->record_bytes<0x31F0 || (c->record_guest&15))return 0;
    for(i=0;i<=c->view_count;++i) {
        region(c,i,&a);
        if(!a.bytes || !span(a.storage,a.bytes) || a.guest_base>0xFFFFFFFFU-(a.bytes-1) ||
           (a.writable!=0 && a.writable!=1) || overlap(a.storage,a.bytes,c,sizeof(*c)) ||
           overlap(a.storage,a.bytes,q,sizeof(*q)) || (out && overlap(a.storage,a.bytes,out,4)) ||
           (n && overlap(a.storage,a.bytes,c->views,n)))return 0;
        for(j=0;j<i;++j) {
            region(c,j,&b);
            if(overlap(a.storage,a.bytes,b.storage,b.bytes) ||
              (a.guest_base>=b.guest_base?a.guest_base-b.guest_base<b.bytes:b.guest_base-a.guest_base<a.bytes))return 0;
        }
    }
    return 1;
}
static void fail(PositionRun *r,int status) {if(r->status>=0)r->status=status;}
static u8 *mapped(PositionRun *r,u32 address,u32 n,u32 alignment,int write) {
    DreadzoneActorInputMemory v;u32 i;
    if(!n || (address&(alignment-1)) || address>0xFFFFFFFFU-(n-1)) {
        fail(r,DREADZONE_DIRECTION_INVALID);return NULL;
    }
    for(i=0;i<=r->ctx->view_count;++i) {
        region(r->ctx,i,&v);
        if(address>=v.guest_base && n<=v.bytes && address-v.guest_base<=v.bytes-n) {
            if(write && !v.writable) {fail(r,DREADZONE_DIRECTION_NEEDS_MEMORY);return NULL;}
            return v.storage+(address-v.guest_base);
        }
    }
    fail(r,DREADZONE_DIRECTION_NEEDS_MEMORY);return NULL;
}
static u32 word(PositionRun *r,u32 address) {
    u32 value=0;u8 *p=mapped(r,address,4,4,0);if(p)memcpy(&value,p,4);return value;
}
static int normal(f32 value) {
    u32 bits;memcpy(&bits,&value,4);bits&=0x7FFFFFFFU;
    return !bits || (bits>=0x00800000U && bits<0x7F800000U);
}
static f32 input(PositionRun *r,f32 value,f32 floor,f32 maximum,int nonnegative) {
    if(!normal(value) || fabsf(value)>maximum || (value!=0.f && fabsf(value)<floor) || (nonnegative && value<0.f)) {
        fail(r,DREADZONE_DIRECTION_UNSUPPORTED_FLOAT);return 0.f;
    }
    return value;
}
static f32 number(PositionRun *r,u32 address,f32 maximum) {
    f32 value=0.f;u8 *p=mapped(r,address,4,4,0);if(p)memcpy(&value,p,4);
    return input(r,value,0x1p-40f,maximum,0);
}
static f32 factor(PositionRun *r,u32 address) {
    f32 value=0.f;u8 *p=mapped(r,address,4,4,0);if(p)memcpy(&value,p,4);
    return input(r,value,0x1p-20f,16.f,1);
}
static f32 result(PositionRun *r,f32 value) {
    if(!normal(value))fail(r,DREADZONE_DIRECTION_UNSUPPORTED_FLOAT);return value;
}
static f32 add(PositionRun *r,f32 a,f32 b) {volatile f32 v=a+b;return result(r,v);}
static f32 sub(PositionRun *r,f32 a,f32 b) {volatile f32 v=a-b;return result(r,v);}
static f32 mul(PositionRun *r,f32 a,f32 b) {volatile f32 v=a*b;return result(r,v);}
static f32 root(PositionRun *r,f32 a) {volatile f32 v=sqrtf(a);return result(r,v);}
static void store(PositionRun *r,u32 address,f32 value) {
    u8 *p=mapped(r,address,4,4,1);result(r,value);if(p)memcpy(p,&value,4);
}
static void store_word(PositionRun *r,u32 address,u32 value) {
    u8 *p=mapped(r,address,4,4,1);if(p)memcpy(p,&value,4);
}
static void vector_read(PositionRun *r,u32 address,u32 raw[4]) {
    u8 *p=mapped(r,address,16,16,0);if(p)memcpy(raw,p,16);else memset(raw,0,16);
}
static void vector_store(PositionRun *r,u32 address,const u32 raw[4]) {
    u8 *p=mapped(r,address,16,16,1);if(p)memcpy(p,raw,16);
}
static f32 raw_float(PositionRun *r,u32 bits,f32 maximum) {
    f32 f;memcpy(&f,&bits,4);return input(r,f,0x1p-40f,maximum,0);
}
static u32 raw_bits(PositionRun *r,f32 f) {u32 bits;result(r,f);memcpy(&bits,&f,4);return bits;}
static f32 xy_distance(PositionRun *r,u32 left,u32 right) {
    u32 a[4],b[4];f32 x,y;
    vector_read(r,left,a);vector_read(r,right,b);
    x=sub(r,raw_float(r,a[0],8192.f),raw_float(r,b[0],8192.f));
    y=sub(r,raw_float(r,a[1],8192.f),raw_float(r,b[1],8192.f));
    return root(r,add(r,mul(r,x,x),mul(r,y,y)));
}
static f32 scalar_progress(PositionRun *r,u32 value,u32 rate,f32 target,f32 force,f32 damping,f32 maximum) {
    u8 *p=mapped(r,rate,4,4,1);f32 current,difference;
    f32 prior=number(r,value,8192.f);number(r,rate,16.f);
    if(!p)return 0.f;
    dreadzone_direction_progress_bytes(p,sub(r,target,prior),force,damping,maximum);
    number(r,rate,16.f);
    current=add(r,number(r,value,8192.f),number(r,rate,16.f));
    difference=sub(r,target,current);store(r,value,current);
    if(fabsf(difference)<mul(r,maximum,0.01f)) {
        store(r,value,target);store_word(r,rate,0);return number(r,rate,16.f);
    }
    return difference;
}
static f32 vector_progress(PositionRun *r,u32 value,u32 target,u32 rate,f32 force,f32 damping,f32 maximum) {
    u32 a[4],b[4];f32 delta[3],square,distance,current,ratio;u8 *p;int i;
    vector_read(r,value,a);vector_read(r,target,b);
    for(i=0;i<3;++i)delta[i]=sub(r,raw_float(r,b[i],8192.f),raw_float(r,a[i],8192.f));
    square=add(r,add(r,mul(r,delta[0],delta[0]),mul(r,delta[1],delta[1])),mul(r,delta[2],delta[2]));
    distance=root(r,square);p=mapped(r,rate,4,4,1);number(r,rate,16.f);if(!p)return 0.f;
    dreadzone_direction_progress_bytes(p,distance,force,damping,maximum);
    current=number(r,rate,16.f);
    square=add(r,add(r,mul(r,delta[0],delta[0]),mul(r,delta[1],delta[1])),mul(r,delta[2],delta[2]));
    if(square!=0.f){volatile f32 q=current/root(r,square);ratio=result(r,q);}else ratio=0.f;
    vector_read(r,value,a);
    for(i=0;i<3;++i) {
        f32 step=square!=0.f?mul(r,delta[i],ratio):0.f;
        a[i]=raw_bits(r,add(r,raw_float(r,a[i],8192.f),step));
    }
    vector_store(r,value,a);current=sub(r,distance,number(r,rate,16.f));
    if(fabsf(current)<mul(r,maximum,0.01f)) {
        vector_read(r,target,b);vector_store(r,value,b);store_word(r,rate,0);return number(r,rate,16.f);
    }
    return current;
}
static void offset(PositionRun *r,u32 value,u32 source,f32 amount) {
    u32 raw[4],body;f32 pair[2],angle;vector_read(r,source,raw);vector_store(r,value,raw);
    body=word(r,r->ctx->record_guest+0x2EF0);if(body>0xFFFFFFFFU-0xFB) {fail(r,DREADZONE_DIRECTION_INVALID);return;}
    angle=number(r,body+0xF8,3.1415927410125732421875f);
    if(dreadzone_angle_pair(angle,pair)<0) {fail(r,DREADZONE_DIRECTION_UNSUPPORTED_FLOAT);return;}
    raw[0]=raw_bits(r,add(r,raw_float(r,raw[0],8192.f),mul(r,pair[1],amount)));
    raw[1]=raw_bits(r,add(r,raw_float(r,raw[1],8192.f),mul(r,pair[0],amount)));
    vector_store(r,value,raw);
}
static f32 heading(PositionRun *r,u32 address) {
    u32 raw[4],body;f32 angle,pair[2],x,y,z;
    vector_read(r,address,raw);body=word(r,r->ctx->record_guest+0x2EF0);
    if(body>0xFFFFFFFFU-0xFB) {fail(r,DREADZONE_DIRECTION_INVALID);return 0.f;}
    angle=number(r,body+0xF8,3.1415927410125732421875f);
    if(dreadzone_angle_pair(angle,pair)<0) {fail(r,DREADZONE_DIRECTION_UNSUPPORTED_FLOAT);return 0.f;}
    pair[0]=add(r,0.f,mul(r,pair[0],1.f));pair[1]=add(r,0.f,mul(r,pair[1],1.f));
    x=mul(r,raw_float(r,raw[0],16.f),pair[1]);y=mul(r,raw_float(r,raw[1],16.f),pair[0]);
    z=mul(r,raw_float(r,raw[2],16.f),0.f);return add(r,add(r,x,y),z);
}
static void settle(PositionRun *r) {
    u32 b=r->ctx->record_guest;u8 *record=r->ctx->record;f32 az,height;
    s16 airborne;memcpy(&airborne,record+0x2FE,2);
    if(airborne==0) {
        az=number(r,b+0xA8,8192.f);height=number(r,b+0x2C8,8192.f);
        if(az<height) {
            if(fabsf(sub(r,az,height))<add(r,fabsf(number(r,b+0x138,16.f)),0.01f) &&
               (record[0x226]==0 || xy_distance(r,b+0x1E0,b+0xA0)>mul(r,number(r,b+0x204,16.f),0.5f))) {
                store(r,b+0xA8,number(r,b+0x2C8,8192.f));goto history;
            }
            if(number(r,b+0x168,16.f)<0.f) {
                store(r,b+0xA8,sub(r,number(r,b+0xA8,8192.f),number(r,b+0x168,16.f)));
                store(r,b+0xA8,dreadzone_direction_min(number(r,b+0xA8,8192.f),number(r,b+0x2C8,8192.f)));
            }
            {
                f32 timing=factor(r,0x21DDEC),force=mul(r,timing,0.05f),damping=mul(r,timing,0.17f);
                f32 maximum=mul(r,factor(r,0x21DDF4),5.f),scale=factor(r,b+0x25C4);
                scalar_progress(r,b+0xA8,b+0x2F4,number(r,b+0x2C8,8192.f),
                    mul(r,force,scale),mul(r,damping,scale),mul(r,maximum,scale));
            }
            goto history;
        }
    }
    store_word(r,b+0x2F4,0);
history:
    if((word(r,b+0x25D4)!=0 && word(r,b+0x25CC)!=2) || number(r,b+0x1B4,16.f)!=0.f)goto clear;
    if(number(r,b+0x304,8192.f)<number(r,b+0xA8,8192.f)) {store(r,b+0x304,number(r,b+0xA8,8192.f));return;}
    if(number(r,b+0xA8,8192.f)<number(r,b+0x308,8192.f) || number(r,b+0x308,8192.f)==0.f) {
        store(r,b+0x308,number(r,b+0xA8,8192.f));return;
    }
    if(sub(r,number(r,b+0x304,8192.f),number(r,b+0x308,8192.f))<0.03f) {
        store_word(r,b+0x300,1);store(r,b+0xA8,number(r,b+0x304,8192.f));return;
    }
clear:
    store_word(r,b+0x308,0);store_word(r,b+0x300,0);store_word(r,b+0x304,0);
}
static void radius(PositionRun *r,s32 initial) {
    u32 b=r->ctx->record_guest,metadata=word(r,b+0x2EEC);s32 previous=(s32)word(r,b+0x25D4),state;
    u8 *record=r->ctx->record;s16 limit;
    if(metadata>0xFFFFFFFFU-0x1F) {fail(r,DREADZONE_DIRECTION_INVALID);return;}
    store(r,b+0x1F8,number(r,metadata+0x14,16.f));store(r,b+0x1FC,number(r,metadata+0x18,16.f));
    store(r,b+0x200,number(r,metadata+0x10,16.f));
    if(previous==4) {
        memcpy(&limit,record+0x1D1C,2);
        if(limit<(s32)word(r,b+0x370) && record[0x1D0E]==0)store(r,b+0x1FC,number(r,b+0x1D28,16.f));
    }else {
        state=(s32)word(r,b+0x25CC);
        if(state==6)store(r,b+0x1FC,number(r,metadata+0x1C,16.f));
        else if(state==4 || record[0x2ECD]!=0)store(r,b+0x1F8,sub(r,0.8f,number(r,metadata+0x10,16.f)));
        else if(previous==17 || previous==18) {
            store(r,b+0x1F8,sub(r,number(r,metadata+0x14,16.f),0.8f));
            store(r,b+0x1FC,sub(r,number(r,metadata+0x18,16.f),0.7f));
        }else if(state==126)store(r,b+0x200,0.8f);
    }
    if(initial!=0) {
        f32 cap=number(r,b+0x200,16.f);store(r,b+0x1F0,number(r,b+0x1F8,16.f));
        store(r,b+0x1F4,number(r,b+0x1FC,16.f));store(r,b+0x240,mul(r,cap,cap));store(r,b+0x204,cap);
    }
    metadata=word(r,b+0x2EEC);
    if(metadata>0xFFFFFFFFU-0x1B) {fail(r,DREADZONE_DIRECTION_INVALID);return;}
    if(record[0x226]!=0 && word(r,b+0x25DC)!=18 && word(r,b+0x25E8)!=18 &&
       !(number(r,b+0x1F4,16.f)<sub(r,number(r,metadata+0x18,16.f),0.17f)) &&
       word(r,b+0x25D4)!=17 && record[0x434]==0 &&
       !(xy_distance(r,b+0xA0,b+0x1E0)>mul(r,number(r,b+0x204,16.f),0.5f)))return;
    {
        u8 *p;f32 step,timing,force,damping,maximum,scale;
        step=mul(r,mul(r,factor(r,0x21DDE8),0.02f),factor(r,b+0x25C4));
        p=mapped(r,b+0x1F0,4,4,1);number(r,b+0x1F0,16.f);
        if(p)dreadzone_velocity_progress_bytes(p,number(r,b+0x1F8,16.f),step);
        number(r,b+0x1F0,16.f);
        step=mul(r,mul(r,factor(r,0x21DDE8),0.02f),factor(r,b+0x25C4));
        p=mapped(r,b+0x1F4,4,4,1);number(r,b+0x1F4,16.f);
        if(p)dreadzone_velocity_progress_bytes(p,number(r,b+0x1FC,16.f),step);
        number(r,b+0x1F4,16.f);
        timing=factor(r,0x21DDEC);force=mul(r,timing,0.02f);damping=mul(r,timing,0.3f);
        maximum=mul(r,factor(r,0x21DDF4),4.f);scale=factor(r,b+0x25C4);
        scalar_progress(r,b+0x204,b+0x208,number(r,b+0x200,16.f),mul(r,force,scale),mul(r,damping,scale),mul(r,maximum,scale));
        store(r,b+0x240,mul(r,number(r,b+0x204,16.f),number(r,b+0x204,16.f)));
    }
}
int dreadzone_actor_position_helper(const DreadzoneActorInputContext *ctx,
        const DreadzonePositionHelperRequest *request,f32 *returned_scalar) {
    DreadzoneActorInputContext tx;DreadzonePositionHelperRequest q;
    DreadzoneActorInputMemory originals[65],views[64];u8 *buffers[65]={0};
    PositionRun run;u32 count,i,total=0,raw[4];int status,returns=0;f32 value=0.f;
    if(!valid(ctx,request,returned_scalar))return DREADZONE_DIRECTION_INVALID;
    q=*request;if(q.operation>DREADZONE_POSITION_RADIUS || ctx->record[0x2659]!=0)return DREADZONE_DIRECTION_UNSUPPORTED_MODE;
    count=ctx->view_count;
    for(i=0;i<=count;++i) {
        region(ctx,i,originals+i);if(originals[i].bytes>16U*1024U*1024U-total)return DREADZONE_DIRECTION_TRANSACTION_MEMORY;
        total+=originals[i].bytes;
    }
    for(i=0;i<=count;++i) {
        buffers[i]=(u8 *)malloc(originals[i].bytes);
        if(!buffers[i]){status=DREADZONE_DIRECTION_TRANSACTION_MEMORY;goto discard;}
        memcpy(buffers[i],originals[i].storage,originals[i].bytes);
        if(i){views[i-1]=originals[i];views[i-1].storage=buffers[i];}
    }
    tx=*ctx;tx.record=buffers[0];tx.views=views;run.ctx=&tx;run.status=DREADZONE_DIRECTION_APPROXIMATE;
    switch(q.operation) {
    case DREADZONE_POSITION_PROJECT:
        vector_read(&run,q.target,raw);raw[2]=0;vector_store(&run,q.value,raw);break;
    case DREADZONE_POSITION_OFFSET:
        offset(&run,q.value,q.target,input(&run,q.scalar[0],0x1p-40f,16.f,0));break;
    case DREADZONE_POSITION_HEADING:value=heading(&run,q.target);returns=1;break;
    case DREADZONE_POSITION_SETTLE:settle(&run);break;
    case DREADZONE_POSITION_SCALAR:
        value=scalar_progress(&run,q.value,q.rate,input(&run,q.scalar[0],0x1p-40f,8192.f,0),
            input(&run,q.scalar[1],0x1p-20f,16.f,1),input(&run,q.scalar[2],0x1p-20f,16.f,1),input(&run,q.scalar[3],0x1p-20f,16.f,1));returns=1;break;
    case DREADZONE_POSITION_VECTOR:
        value=vector_progress(&run,q.value,q.target,q.rate,input(&run,q.scalar[0],0x1p-20f,16.f,1),
            input(&run,q.scalar[1],0x1p-20f,16.f,1),input(&run,q.scalar[2],0x1p-20f,16.f,1));returns=1;break;
    default:radius(&run,q.initial);break;
    }
    status=run.status;
    if(status>=0) {
        for(i=0;i<=count;++i)if(originals[i].writable)memcpy(originals[i].storage,buffers[i],originals[i].bytes);
        if(returns && returned_scalar)memcpy(returned_scalar,&value,4);
    }
discard:
    for(i=0;i<=count;++i)free(buffers[i]);return status;
}
