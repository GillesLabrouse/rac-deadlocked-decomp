#include "dreadzone_actor_position_wrapper.h"
#include "dreadzone_wall_helpers.h"
#include "dreadzone_object_predicate.h"
#include <float.h>
#include <fenv.h>
#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Position wrapper requires binary32 floats
#endif
typedef char position_wrapper_float_size[sizeof(float)==4 ? 1 : -1];
#include <math.h>
#include <stdlib.h>
#include <string.h>
typedef struct WrapperWork {
    DreadzonePositionWrapperRequest request;
    DreadzonePositionWrapperInfo info;
    u8 *copy;
    int failed;
} WrapperWork;
static int span(const void *p,uintptr_t bytes) {return p && (uintptr_t)p<=UINTPTR_MAX-bytes;}
static int overlap(const void *p,u32 bytes,const void *q,u32 other) {
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;return a>=b?a-b<other:b-a<bytes;
}
static u8 *memory(WrapperWork *w,u32 address,u32 bytes,u32 alignment) {
    if(w->failed)return NULL;
    if((address&(alignment-1U)) || address<w->request.guest_base ||
       bytes>w->request.storage_bytes || address-w->request.guest_base>w->request.storage_bytes-bytes) {
        w->failed=-1;return NULL;
    }
    return w->copy+address-w->request.guest_base;
}
static u32 word(WrapperWork *w,u32 a) {
    u32 v=0;u8 *p=memory(w,a,4,4);if(p)memcpy(&v,p,4);return v;
}
static u8 byte(WrapperWork *w,u32 a) {u8 *p=memory(w,a,1,1);return p?*p:0;}
static void store_word(WrapperWork *w,u32 a,u32 v) {u8 *p=memory(w,a,4,4);if(p)memcpy(p,&v,4);}
static void store_byte(WrapperWork *w,u32 a,u8 v) {u8 *p=memory(w,a,1,1);if(p)*p=v;}
static s32 wrapped_sum(u32 a,u32 b) {
    u32 raw=a+b;s32 result;memcpy(&result,&raw,4);return result;
}
static void angle_hit(WrapperWork *w) {
    static const f32 coefficients[8]={
        .999999344348907470703125f,-.33329856395721435546875f,
        .1994653642177581787109375f,-.13908533751964569091796875f,
        .096420042216777801513671875f,-.05590988695621490478515625f,
        .0218612290918827056884765625f,-.0040540578775107860565185546875f
    };
    static const f32 quadrants[16]={
        1.f,0.f,-1.f,1.57079637050628662109375f,
        -1.f,0.f,1.f,-1.57079637050628662109375f,
        -1.f,3.1415927410125732421875f,1.f,1.57079637050628662109375f,
        1.f,-3.1415927410125732421875f,-1.f,-1.57079637050628662109375f
    };
    u32 b=w->request.record_guest,raw[4],object,angle_bits;
    u8 *record=memory(w,b,0x31F0,16),*normal=memory(w,0x242380,16,16),*table;
    f32 angle;int status;DreadzoneObjectPredicateRequest predicate={0};
    DreadzoneObjectPredicateInfo pi;
    if(w->failed)return;
    if(record[0x2659]!=0){w->failed=-2;return;}
    memcpy(raw,normal,16);
    if((raw[0]&0x7FFFFFFFU)||(raw[1]&0x7FFFFFFFU)||(raw[2]&0x7FFFFFFFU)) {
        table=memory(w,0x3B9540,sizeof(coefficients),16);
        if(!table)return;
        if(memcmp(table,coefficients,sizeof(coefficients))){w->failed=-2;return;}
        table=memory(w,0x3B9580,sizeof(quadrants),16);
        if(!table)return;
        if(memcmp(table,quadrants,sizeof(quadrants))){w->failed=-2;return;}
    }
    /* Reuse the validated raw mode-zero kernel: all four squares, XY SQRT,
     * original Z and recovered55D708; no transform or manufactured normal. */
    status=dreadzone_wall_inclination_0068AFF8(record,0x31F0,normal,16,&angle);
    if(status<0){w->failed=status==-1?-1:-3;return;}
    memcpy(&angle_bits,&angle,4);store_word(w,b+0x220,angle_bits);
    w->info.angle_calls=1;
    object=word(w,0x242358);
    if(!w->failed && object) {
        store_byte(w,b+0x225,1);
        if(w->failed)return;
        predicate.storage=w->copy;predicate.guest_base=w->request.guest_base;
        predicate.storage_bytes=w->request.storage_bytes;predicate.packet_guest=word(w,0x242358);
        status=dreadzone_object_predicate_00596E48(&predicate,&pi);
        if(status<0){w->failed=status;return;}
        w->info.object_calls=1;
        if(pi.original_return)store_byte(w,b+0x224,1);
    }
}
static u32 raw_float(f32 x) {u32 b;memcpy(&b,&x,4);return b;}
static f32 decode(u32 b) {f32 x;memcpy(&x,&b,4);return x;}
static f32 checked_float(WrapperWork *w,f32 x) {
    u32 m=raw_float(x)&0x7FFFFFFFU;
    if(m && (m<0x00800000U || m>=0x7F800000U)) {
        if(!w->failed)w->failed=-3;return 0.f;
    }
    return x;
}
static f32 rounded_add(WrapperWork *w,f32 a,f32 b) {
    volatile f32 x; a=checked_float(w,a);b=checked_float(w,b);x=a+b;return checked_float(w,x);
}
static f32 rounded_sub(WrapperWork *w,f32 a,f32 b) {
    volatile f32 x; a=checked_float(w,a);b=checked_float(w,b);x=a-b;return checked_float(w,x);
}
static f32 rounded_mul(WrapperWork *w,f32 a,f32 b) {
    volatile f32 x; a=checked_float(w,a);b=checked_float(w,b);x=a*b;
    if(a!=0.f && b!=0.f && x==0.f && !w->failed)w->failed=-3;
    return checked_float(w,x);
}
static void active_probe(WrapperWork *w) {
    DreadzoneWallRequest wall={0};u32 quads[2][4],matrix[4][4],position[4],i,j,k,b=w->request.record_guest;
    f32 local[2][4]={{1.1f,0.f,1.f,0.f},{1.1f,0.f,-20.f,0.f}},squares[4],distance;
    u8 *p;int hit;
    for(j=0;j<4;++j){p=memory(w,b+0x20U+16*j,16,16);if(!p)return;memcpy(matrix[j],p,16);}
    p=memory(w,b+0xA0U,16,16);if(!p)return;memcpy(position,p,16);
    for(k=0;k<2;++k)for(i=0;i<4;++i) {
        f32 acc=rounded_mul(w,decode(matrix[0][i]),local[k][0]);
        for(j=1;j<4;++j)acc=rounded_add(w,acc,rounded_mul(w,decode(matrix[j][i]),local[k][j]));
        if(i<3)acc=rounded_add(w,acc,decode(position[i]));
        quads[k][i]=raw_float(acc);
    }
    /* Actual684C7C MAXX.Z uses VF3.X=0.5, not zero. */
    if(!w->failed && decode(quads[1][2])<.5f)quads[1][2]=0x3F000000U;
    wall.storage=w->copy;wall.guest_base=w->request.guest_base;wall.storage_bytes=w->request.storage_bytes;
    wall.packet_guest=0x242340U;wall.grid_guest=0x26FF20U;wall.actor_pool_pointer_guest=0x222D14U;
    wall.actor_pool_count=w->request.actor_pool_count;wall.grid_block_count=w->request.grid_block_count;
    wall.ignored_actor_guest=word(w,b+0x2EF0U);wall.caller_flags=2;
    memcpy(wall.incoming_vf25_27_w,w->info.outgoing_vf25_29_w,12);
    if(w->failed)return;
    hit=dreadzone_wall_contact_vectors(&wall,quads[0],quads[1],&w->info.active_wall);
    if(hit<0){w->failed=hit;return;}
    w->info.active_wall_calls=1;w->info.outgoing_vf25_29_w[3]=0;w->info.outgoing_vf25_29_w[4]=0;
    if(!hit){w->failed=-2;return;} /* True miss reaches the unrecovered sphere loop. */
    p=memory(w,0x242360U,16,16);if(!p)return;memcpy(position,p,16);
    for(i=0;i<4;++i) {
        f32 delta=decode(quads[0][i]);
        if(i<3)delta=rounded_sub(w,delta,decode(position[i]));
        squares[i]=rounded_mul(w,delta,delta);
    }
    distance=rounded_add(w,rounded_add(w,squares[0],squares[1]),squares[2]);
    distance=checked_float(w,sqrtf(fabsf(distance)));distance=rounded_add(w,0.f,distance);
    if(!w->failed && distance>3.f)w->failed=-2;
}

static void body(WrapperWork *w) {
    DreadzonePositionStepRequest position={0};DreadzoneOffsetProbeRequest offset={0};
    u32 b=w->request.record_guest,frame,phase,next_frame,active;int status;
    if(byte(w,b+0x2672U)){w->info.early_exit=1;return;}
    if(w->failed)return;
    position.storage=w->copy;position.guest_base=w->request.guest_base;
    position.storage_bytes=w->request.storage_bytes;position.record_guest=b;
    position.actor_pool_count=w->request.actor_pool_count;
    position.grid_block_count=w->request.grid_block_count;
    memcpy(position.incoming_vf25_29_w,w->info.outgoing_vf25_29_w,20);
    memcpy(position.incoming_ground_side_w,w->request.incoming_ground_side_w,8);
    status=dreadzone_actor_position_state_0067F5E8(&position,w->request.incoming_high64,&w->info.position);
    if(status<0){w->failed=status;return;}
    w->info.position_calls=1;
    memcpy(w->info.outgoing_vf25_29_w,w->info.position.outgoing_vf25_29_w,20);
    frame=word(w,0x21DDB8U);phase=word(w,b+0x2F10U);
    next_frame=word(w,0x21DDB8U); /* Real684AD0 delay executes for either gate. */
    if(!w->failed && wrapped_sum(frame,phase)%3==0) {
        store_byte(w,b+0x224U,0);store_byte(w,b+0x225U,0);
        store_word(w,b+0x218U,0x40800000U); /* Real JAL delay store before entry. */
        if(w->failed)return;
        offset.storage=w->copy;offset.guest_base=w->request.guest_base;
        offset.storage_bytes=w->request.storage_bytes;offset.record_guest=b;
        offset.output_guest=b+0x218U;offset.local_z=.7f;offset.limit=4.f;
        offset.actor_pool_count=w->request.actor_pool_count;
        offset.grid_block_count=w->request.grid_block_count;
        memcpy(offset.incoming_vf25_29_w,w->info.outgoing_vf25_29_w,20);
        memcpy(offset.incoming_local_w,w->request.incoming_offset_local_w,8);
        status=dreadzone_actor_offset_probe_006708B0(&offset,&w->info.offset);
        if(status<0){w->failed=status;return;}
        w->info.offset_calls=1;
        memcpy(w->info.outgoing_vf25_29_w,w->info.offset.outgoing_vf25_29_w,20);
        if(w->info.offset.original_integer)angle_hit(w);
        if(w->failed)return;
        next_frame=word(w,0x21DDB8U); /* Actual684B80 or684B64 after the selected returning branch. */
    }
    phase=word(w,b+0x2F10U);
    if(!w->failed && wrapped_sum(next_frame,phase)%5==0) {
        active=word(w,b+0x2F0U);
        store_word(w,b+0x230U,0);store_word(w,b+0x228U,0);
        w->info.mod5_resets=1;
        if(!w->failed && active)active_probe(w);
    }
}
static int numeric_host(void) {
    volatile f32 minimum=0x1p-126f,half_minimum=minimum*.5f;
    f32 probe=half_minimum;u32 raw;memcpy(&raw,&probe,4);
    return fegetround()==FE_TONEAREST && raw==0x00400000U;
}
int dreadzone_actor_position_wrapper_00684A60(const DreadzonePositionWrapperRequest *request,
    DreadzonePositionWrapperInfo *info) {
    WrapperWork w;u32 end,b;int status;
    if(!span(request,sizeof(*request)) || !span(info,sizeof(*info)) ||
       overlap(request,sizeof(*request),info,sizeof(*info)))return -1;
    memset(&w,0,sizeof(w));memcpy(&w.request,request,sizeof(w.request));
    if(!w.request.storage_bytes || w.request.storage_bytes>16U*1024U*1024U ||
       !span(w.request.storage,w.request.storage_bytes) ||
       w.request.guest_base>=0x80000000U || w.request.storage_bytes>0x80000000U-w.request.guest_base ||
       (w.request.record_guest&15U) || overlap(info,sizeof(*info),w.request.storage,w.request.storage_bytes))return -1;
    end=w.request.guest_base+w.request.storage_bytes;b=w.request.record_guest;
    if(b<w.request.guest_base || b>end || end-b<0x31F0U ||
       (w.request.guest_base<0x70004000U && end>0x70000000U))return -1;
    if(!numeric_host())return -3;
    w.copy=(u8 *)malloc(w.request.storage_bytes);if(!w.copy)return -4;
    memcpy(w.copy,w.request.storage,w.request.storage_bytes);
    memcpy(w.info.outgoing_vf25_29_w,w.request.incoming_vf25_29_w,20);
    body(&w);status=w.failed?w.failed:1;
    if(status>0){memcpy(w.request.storage,w.copy,w.request.storage_bytes);memcpy(info,&w.info,sizeof(*info));}
    free(w.copy);return status;
}
