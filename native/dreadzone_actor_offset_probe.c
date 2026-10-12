#include "dreadzone_actor_offset_probe.h"
#include "dreadzone_actor_ground_step.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Offset probe requires IEEE binary32
#endif
typedef char offset_probe_float_size[sizeof(f32)==4?1:-1];
typedef struct OffsetWork {
    DreadzoneOffsetProbeRequest request;
    DreadzoneOffsetProbeInfo info;
    u8 *copy;
    int failed;
} OffsetWork;
static int span(const void *p,uintptr_t bytes) {
    return p && (uintptr_t)p<=UINTPTR_MAX-bytes;
}
static int overlap(const void *p,u32 bytes,const void *q,u32 other) {
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;
    return a>=b?a-b<other:b-a<bytes;
}
static u8 *memory(OffsetWork *w,u32 address,u32 bytes,u32 align) {
    if(w->failed)return NULL;
    if((address&(align-1U)) || address<w->request.guest_base ||
       bytes>w->request.storage_bytes ||
       address-w->request.guest_base>w->request.storage_bytes-bytes) {
        w->failed=DREADZONE_OFFSET_INVALID;return NULL;
    }
    return w->copy+(address-w->request.guest_base);
}
static u32 raw(f32 value) {u32 b;memcpy(&b,&value,4);return b;}
static f32 constant(u32 bits) {f32 v;memcpy(&v,&bits,4);return v;}
static f32 checked(OffsetWork *w,f32 v) {
    u32 m=raw(v)&0x7FFFFFFFU;
    if(m && (m<0x00800000U || m>=0x7F800000U)) {
        if(!w->failed)w->failed=DREADZONE_OFFSET_UNSUPPORTED_FLOAT;
        return 0.f;
    }
    return v;
}
static f32 load(OffsetWork *w,u32 address) {
    u32 b=0;u8 *p=memory(w,address,4,4);if(p)memcpy(&b,p,4);
    return checked(w,constant(b));
}
static void store(OffsetWork *w,u32 address,f32 value) {
    u8 *p=memory(w,address,4,4);if(p)memcpy(p,&value,4);
}
static f32 add(OffsetWork *w,f32 a,f32 b) {
    volatile f32 v; a=checked(w,a);b=checked(w,b);v=a+b;return checked(w,v);
}
static f32 subtract(OffsetWork *w,f32 a,f32 b) {
    volatile f32 v; a=checked(w,a);b=checked(w,b);v=a-b;return checked(w,v);
}
static f32 multiply(OffsetWork *w,f32 a,f32 b) {
    volatile f32 v;a=checked(w,a);b=checked(w,b);v=a*b;
    if(a!=0.f && b!=0.f && v==0.f && !w->failed)
        w->failed=DREADZONE_OFFSET_UNSUPPORTED_FLOAT;
    return checked(w,v);
}
static void recompute(OffsetWork *w) {
    DreadzoneGroundRequest g={0};DreadzoneGroundInfo gi;
    DreadzoneWallRequest wall={0};
    u32 quads[2][4]={{0}},b=w->request.record_guest,i;
    f32 distance=add(w,w->request.limit,1.f);
    f32 radius=load(w,b+0x204U),start=subtract(w,radius,constant(0x3CA3D70AU));
    int status,hit;
    store(w,b+0x2F90U,w->request.local_z);
    store(w,b+0x2F94U,distance);
    if(w->failed)return;
    g.storage=w->copy;g.guest_base=w->request.guest_base;
    g.storage_bytes=w->request.storage_bytes;g.record_guest=b;
    g.operation=DREADZONE_GROUND_SIDE_OFFSET;g.integer0=1;
    g.scalar[1]=0.f;g.scalar[2]=w->request.local_z;
    memcpy(g.incoming_vf25_29_w,w->info.outgoing_vf25_29_w,20);
    for(i=0;i<2;++i) {
        quads[i][3]=w->request.incoming_local_w[i];
        g.scalar[0]=i?distance:start;
        status=dreadzone_side_offset_captured_quad(&g,quads[i],quads[i],&gi);
        if(status<0){w->failed=status;return;}
    }
    wall.storage=w->copy;wall.guest_base=w->request.guest_base;
    wall.storage_bytes=w->request.storage_bytes;
    wall.packet_guest=0x242340U;wall.grid_guest=0x26FF20U;
    wall.actor_pool_pointer_guest=0x222D14U;
    wall.actor_pool_count=w->request.actor_pool_count;
    wall.grid_block_count=w->request.grid_block_count;wall.caller_flags=2;
    memcpy(wall.incoming_vf25_27_w,w->info.outgoing_vf25_29_w,12);
    hit=dreadzone_wall_contact_vectors(&wall,quads[0],quads[1],&w->info.wall);
    if(hit<0){w->failed=hit;return;}
    w->info.wall_calls=1;w->info.cache_updates=1;
    w->info.outgoing_vf25_29_w[3]=0;w->info.outgoing_vf25_29_w[4]=0;
    if(hit) {
        f32 squared[4],sum,result;u8 *p=memory(w,0x242360U,16,16);
        u32 contact[4];if(!p)return;memcpy(contact,p,16);
        for(i=0;i<4;++i) {
            f32 v=constant(quads[0][i]);
            if(i<3)v=subtract(w,v,constant(contact[i]));
            squared[i]=multiply(w,v,v);
        }
        sum=add(w,add(w,squared[0],squared[1]),squared[2]);
        result=checked(w,sqrtf(fabsf(sum)));
        store(w,b+0x2F98U,add(w,0.f,result));
    } else store(w,b+0x2F98U,constant(0x44FA0000U));
}
int dreadzone_actor_offset_probe_006708B0(const DreadzoneOffsetProbeRequest *request,
    DreadzoneOffsetProbeInfo *info) {
    OffsetWork w;u32 end,b;f32 delta,previous,answer;int status;
    if(!span(request,sizeof(*request)) || !span(info,sizeof(*info)) ||
       overlap(request,sizeof(*request),info,sizeof(*info)))return DREADZONE_OFFSET_INVALID;
    memset(&w,0,sizeof(w));memcpy(&w.request,request,sizeof(w.request));
    if(!w.request.storage_bytes || w.request.storage_bytes>64U*1024U*1024U ||
       !span(w.request.storage,w.request.storage_bytes) ||
       w.request.guest_base>=0x80000000U ||
       w.request.storage_bytes>0x80000000U-w.request.guest_base ||
       (w.request.record_guest&15U) ||
       overlap(info,sizeof(*info),w.request.storage,w.request.storage_bytes))
        return DREADZONE_OFFSET_INVALID;
    end=w.request.guest_base+w.request.storage_bytes;b=w.request.record_guest;
    if(b<w.request.guest_base || b>end || end-b<0x31F0U ||
       (w.request.guest_base<0x70004000U && end>0x70000000U))return DREADZONE_OFFSET_INVALID;
    checked(&w,w.request.local_z);checked(&w,w.request.limit);
    if(fabsf(w.request.local_z)>16.f || fabsf(w.request.limit)>16.f)
        w.failed=DREADZONE_OFFSET_UNSUPPORTED_FLOAT;
    if(w.failed)return w.failed;
    w.copy=(u8 *)malloc(w.request.storage_bytes);
    if(!w.copy)return DREADZONE_OFFSET_TRANSACTION_MEMORY;
    memcpy(w.copy,w.request.storage,w.request.storage_bytes);
    memcpy(w.info.outgoing_vf25_29_w,w.request.incoming_vf25_29_w,20);
    delta=fabsf(subtract(&w,w.request.local_z,load(&w,b+0x2F90U)));
    if(!w.failed && constant(0x3E083127U)<delta)recompute(&w);
    else if(!w.failed) {
        previous=load(&w,b+0x2F94U);
        if(!w.failed && previous<w.request.limit)recompute(&w);
    }
    answer=load(&w,b+0x2F98U);
    if(!w.failed && answer<w.request.limit) {
        w.info.original_integer=1;
        if(w.request.output_guest)store(&w,w.request.output_guest,answer);
    }
    status=w.failed?w.failed:DREADZONE_OFFSET_APPROXIMATE;
    if(status>0) {
        memcpy(w.request.storage,w.copy,w.request.storage_bytes);
        memcpy(info,&w.info,sizeof(*info));
    }
    free(w.copy);return status;
}
