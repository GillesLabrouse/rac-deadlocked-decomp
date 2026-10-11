#include "dreadzone_actor_wall_response.h"
#include "dreadzone_wall_helpers.h"
#include "dreadzone_actor_velocity.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Wall response requires IEEE binary32 host floats
#endif
typedef char wall_response_float_size[sizeof(f32)==4?1:-1];
typedef struct ResponseWork {
    DreadzoneActorWallResponseRequest request;
    DreadzoneActorWallResponseInfo info;
    u8 *copy;
    int failed;
} ResponseWork;
typedef struct ResponseVector { f32 lane[4]; } ResponseVector;

static int host_span(const void *p,uintptr_t bytes) {
    return p && (uintptr_t)p<=UINTPTR_MAX-bytes;
}
static int overlaps(const void *p,u32 bytes,const void *q,u32 other) {
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;
    return a>=b?a-b<other:b-a<bytes;
}
static u8 *memory(ResponseWork *w,u32 address,u32 bytes,u32 alignment) {
    DreadzoneWallRequest *r=&w->request.wall;
    if(w->failed)return NULL;
    if((address&(alignment-1U)) || address<r->guest_base ||
        bytes>r->storage_bytes || address-r->guest_base>r->storage_bytes-bytes) {
        w->failed=DREADZONE_ACTOR_WALL_INVALID;return NULL;
    }
    return w->copy+(address-r->guest_base);
}
static u32 word(ResponseWork *w,u32 address) {
    u32 value=0;u8 *p=memory(w,address,4,4);if(p)memcpy(&value,p,4);return value;
}
static s32 signed_word(ResponseWork *w,u32 address) {
    u32 raw=word(w,address);s32 value;memcpy(&value,&raw,4);return value;
}
static u8 byte(ResponseWork *w,u32 address) {
    u8 *p=memory(w,address,1,1);return p?*p:0;
}
static void store_word(ResponseWork *w,u32 address,u32 value) {
    u8 *p=memory(w,address,4,4);if(p)memcpy(p,&value,4);
}
static u32 raw(f32 value) {u32 bits;memcpy(&bits,&value,4);return bits;}
static int normal(f32 value) {
    u32 magnitude=raw(value)&0x7FFFFFFFU;
    return !magnitude || (magnitude>=0x00800000U && magnitude<0x7F800000U);
}
static f32 checked(ResponseWork *w,f32 value) {
    if(!normal(value)&&!w->failed)w->failed=DREADZONE_ACTOR_WALL_UNSUPPORTED_FLOAT;
    return value;
}
static f32 load(ResponseWork *w,u32 address) {
    u32 bits=word(w,address);f32 value;memcpy(&value,&bits,4);return checked(w,value);
}
static f32 add(ResponseWork *w,f32 a,f32 b) {
    volatile f32 result;
    checked(w,a);checked(w,b);result=a+b;return checked(w,result);
}
static f32 mul(ResponseWork *w,f32 a,f32 b) {
    volatile f32 result;
    checked(w,a);checked(w,b);result=a*b;
    if(a!=0.f&&b!=0.f&&result==0.f&&!w->failed)w->failed=DREADZONE_ACTOR_WALL_UNSUPPORTED_FLOAT;
    return checked(w,result);
}
static ResponseVector vector(ResponseWork *w,u32 address) {
    ResponseVector result={{0.f,0.f,0.f,0.f}};u8 *p=memory(w,address,16,16);
    if(p)memcpy(&result,p,16);return result;
}
static void store_vector(ResponseWork *w,u32 address,const ResponseVector *value) {
    u8 *p=memory(w,address,16,16);if(p)memcpy(p,value,16);
}
static f32 squares(ResponseWork *w,const ResponseVector *v,ResponseVector *out) {
    u32 i;for(i=0;i<4;++i)out->lane[i]=mul(w,v->lane[i],v->lane[i]);
    return add(w,out->lane[0],out->lane[1]);
}
static f32 reciprocal_sqrt(ResponseWork *w,f32 numerator,f32 denominator) {
    volatile f32 root,result;
    checked(w,numerator);checked(w,denominator);
    if(denominator<0.f)w->info.approximate_q_exception_bits|=0x10U;
    root=sqrtf(fabsf(denominator));checked(w,root);
    if(root==0.f) {
        u32 sign=(raw(numerator)^raw(root))&0x80000000U;
        u32 bits=0x7F7FFFFFU|sign;f32 saturated;
        w->info.approximate_q_exception_bits|=numerator==0.f?0x10U:0x20U;
        memcpy(&saturated,&bits,4);return saturated;
    }
    result=numerator/root;return checked(w,result);
}
static ResponseVector normalize(ResponseWork *w,ResponseVector value,f32 requested,
                                int predicate_y_is_one,int zero_w_is_one) {
    ResponseVector square;f32 sum=squares(w,&value,&square),norm,q;u32 i;
    norm=add(w,sum,mul(w,1.f,square.lane[2]));
    q=reciprocal_sqrt(w,requested,norm); /* Executes before the real zero test. */
    if(raw(norm)||(predicate_y_is_one?0x3F800000U:raw(square.lane[1]))) {
        for(i=0;i<3;++i)value.lane[i]=mul(w,value.lane[i],q);
    } else {
        /* VSUB immutableVF0.xyz, preserving the old vector's W. */
        for(i=0;i<3;++i)value.lane[i]=0.f;
        /* Final67DB54 operates on VF1=ones, rather than the source vector. */
        if(zero_w_is_one)value.lane[3]=1.f;
    }
    return value;
}
static int projection(ResponseWork *w,u32 record,u8 *out,const u8 *input) {
    u8 *p=memory(w,record,0x31F0,16);int result;
    if(!p)return 0;
    if(byte(w,record+0x2659)!=0){w->failed=DREADZONE_ACTOR_WALL_UNSUPPORTED_PATH;return 0;}
    result=dreadzone_velocity_0068ABA8(p,out,input,0.f);
    if(result<0){w->failed=DREADZONE_ACTOR_WALL_UNSUPPORTED_PATH;return 0;}
    ++w->info.projection_calls;return 1;
}
static void atan_tables(ResponseWork *w) {
    static const f32 coefficient[8]={0.999999344348907470703125f,-0.33329856395721435546875f,
        0.1994653642177581787109375f,-0.13908533751964569091796875f,
        0.096420042216777801513671875f,-0.05590988695621490478515625f,
        0.0218612290918827056884765625f,-0.0040540578775107860565185546875f};
    static const f32 quadrant[16]={1.f,0.f,-1.f,1.57079637050628662109375f,
        -1.f,0.f,1.f,-1.57079637050628662109375f,
        -1.f,3.1415927410125732421875f,1.f,1.57079637050628662109375f,
        1.f,-3.1415927410125732421875f,-1.f,-1.57079637050628662109375f};
    u8 *p=memory(w,0x3B9540,sizeof(coefficient),16);
    if(p&&memcmp(p,coefficient,sizeof(coefficient)))w->failed=DREADZONE_ACTOR_WALL_UNSUPPORTED_PATH;
    p=memory(w,0x3B9580,sizeof(quadrant),16);
    if(p&&memcmp(p,quadrant,sizeof(quadrant)))w->failed=DREADZONE_ACTOR_WALL_UNSUPPORTED_PATH;
}
static int probe(ResponseWork *w,const ResponseVector *start,const ResponseVector *end) {
    DreadzoneWallRequest request=w->request.wall;DreadzoneWallInfo info;
    u32 start_words[4],end_words[4];int result;
    request.storage=w->copy;request.caller_flags=2;request.cache_guest=0;
    request.ignored_actor_guest=word(w,w->request.record_guest+0x2EF0);
    if(w->failed)return -1;
    memcpy(start_words,start,16);memcpy(end_words,end,16);
    result=dreadzone_wall_contact_vectors(&request,start_words,end_words,&info);
    if(result<0){w->failed=result==DREADZONE_WALL_INVALID?DREADZONE_ACTOR_WALL_INVALID:
        DREADZONE_ACTOR_WALL_UNSUPPORTED_PATH;return -1;}
    ++w->info.probe_calls;w->info.outgoing_vf25_29_w[3]=0;w->info.outgoing_vf25_29_w[4]=0;
    w->info.approximate_q_exception_bits|=info.q_exception_bits;return result;
}
static int body(ResponseWork *w) {
    u32 record=w->request.record_guest,packet=w->request.wall.packet_guest,i,j;
    ResponseVector velocity,basis[4],position,firstStart,secondStart,firstDelta,firstEnd,square;
    ResponseVector normal_vector;f32 speed,threshold,height=0.37f,radius=0.5f,inclination=0.f;
    u8 *p;int result;s32 surface=-1;
    p=memory(w,record,0x31F0,16);if(!p)return w->failed;
    if(byte(w,record+0x2659)!=0)return DREADZONE_ACTOR_WALL_UNSUPPORTED_PATH;
    velocity=vector(w,record+0x130);squares(w,&velocity,&square);
    if(w->failed)return w->failed;
    {VelocityVector input;memcpy(&input,&velocity,16);speed=checked(w,func_L01_0068A920(p,&input));}
    threshold=mul(w,load(w,0x21DDF4),0.1f);threshold=mul(w,threshold,load(w,record+0x25C4));
    if(w->failed)return w->failed;
    if(speed<threshold)return DREADZONE_ACTOR_WALL_APPROXIMATE;
    if(!byte(w,record+0x266D)) {
        u8 scheduling=byte(w,0x220648);u32 parity=word(w,0x21DDB8);
        if(scheduling&&(word(w,record+0x2F10)&7U)!=(parity&7U)&&
            signed_word(w,record+0x2F5C)<0&&!word(w,record+0x1848))return w->failed?w->failed:1;
    }
    if(w->failed)return w->failed;
    if(word(w,record+0x25D4)-17U<2U){height=0.f;radius=0.7f;}
    for(i=0;i<4;++i)basis[i]=vector(w,record+0x20+16*i);
    position=vector(w,record+0xA0);
    for(i=0;i<4;++i) {
        f32 a=mul(w,basis[0].lane[i],0.f);
        a=add(w,a,mul(w,basis[1].lane[i],0.f));
        a=add(w,a,mul(w,basis[2].lane[i],height));
        secondStart.lane[i]=add(w,a,mul(w,basis[3].lane[i],0.f));
        a=mul(w,basis[0].lane[i],0.f);a=add(w,a,mul(w,basis[1].lane[i],0.f));
        a=add(w,a,mul(w,basis[2].lane[i],mul(w,height,0.3f)));
        firstStart.lane[i]=add(w,a,mul(w,basis[3].lane[i],0.f));
    }
    for(i=0;i<3;++i){secondStart.lane[i]=add(w,secondStart.lane[i],position.lane[i]);
        firstStart.lane[i]=add(w,firstStart.lane[i],position.lane[i]);}
    if(w->request.force||speed<0.01f) {
        firstDelta=normalize(w,basis[0],radius,1,0);
        firstEnd=normalize(w,vector(w,record+0x20),mul(w,radius,1.4f),0,0);
    } else {
        /* Real mode0 projection copies raw source X/Y/W and overwrites Z. */
        p=memory(w,record+0x130,16,16);if(!p||!projection(w,record,(u8 *)&firstDelta,p))return w->failed;
        firstDelta=normalize(w,firstDelta,radius,1,0);firstEnd=firstDelta;
        for(i=0;i<3;++i)firstEnd.lane[i]=mul(w,firstEnd.lane[i],1.4f);
    }
    for(i=0;i<3;++i){firstDelta.lane[i]=add(w,firstDelta.lane[i],secondStart.lane[i]);
        firstEnd.lane[i]=add(w,firstEnd.lane[i],firstStart.lane[i]);}
    if(w->failed)return w->failed;
    result=probe(w,&firstStart,&firstEnd);if(w->failed)return w->failed;
    if(result&&signed_word(w,packet+0x1C)>0) {
        p=memory(w,packet,0x20,16);if(!p)return w->failed;
        if(dreadzone_wall_surface_00542FF8(p,0x20,&surface)<0)return DREADZONE_ACTOR_WALL_INVALID;
        ++w->info.surface_calls;
    }
    result=probe(w,&secondStart,&firstDelta);if(w->failed)return w->failed;
    if(result&&signed_word(w,packet+0x1C)>0) {
        const u8 *normal_bytes;atan_tables(w);if(w->failed)return w->failed;
        p=memory(w,record,0x31F0,16);normal_bytes=memory(w,packet+0x40,16,16);
        if(!p||!normal_bytes)return w->failed;
        if(dreadzone_wall_inclination_0068AFF8(p,0x31F0,normal_bytes,16,&inclination)<0)
            return DREADZONE_ACTOR_WALL_UNSUPPORTED_FLOAT;
        ++w->info.inclination_calls;
    }
    if(w->failed)return w->failed;
    if(!(0.87266463f<=inclination)&&surface!=8&&surface!=12)return 1;
    store_word(w,record+0x210,word(w,packet+0x18));
    p=memory(w,packet+0x40,16,16);if(!p||!projection(w,record,p,p))return w->failed;
    normal_vector=normalize(w,vector(w,packet+0x40),1.f,0,1);
    store_vector(w,packet+0x40,&normal_vector);
    /* Publication can alias record130: reload both operands after the store. */
    normal_vector=vector(w,packet+0x40);velocity=vector(w,record+0x130);
    for(j=0;j<4;++j)square.lane[j]=mul(w,normal_vector.lane[j],velocity.lane[j]);
    speed=add(w,add(w,square.lane[0],square.lane[1]),mul(w,1.f,square.lane[2]));
    if(w->failed)return w->failed;
    if(speed<0.f) {
        for(i=0;i<3;++i)normal_vector.lane[i]=mul(w,normal_vector.lane[i],-speed);
        store_vector(w,packet+0x40,&normal_vector);
        velocity=vector(w,record+0x130);
        for(i=0;i<3;++i)velocity.lane[i]=add(w,velocity.lane[i],normal_vector.lane[i]);
        store_vector(w,record+0x130,&velocity);
    }
    return w->failed?w->failed:DREADZONE_ACTOR_WALL_APPROXIMATE;
}

int dreadzone_actor_wall_response_0067D6E8(const DreadzoneActorWallResponseRequest *request,
    DreadzoneActorWallResponseInfo *info) {
    ResponseWork w;DreadzoneWallRequest *r;u32 i;int result;u8 before[16];
    if(!host_span(request,sizeof(*request))||!host_span(info,sizeof(*info)))return DREADZONE_ACTOR_WALL_INVALID;
    memset(&w,0,sizeof(w));memcpy(&w.request,request,sizeof(*request));r=&w.request.wall;
    if(!r->storage_bytes||r->storage_bytes>16U*1024U*1024U||
        !host_span(r->storage,r->storage_bytes)||(u64)r->guest_base+r->storage_bytes>0x80000000ULL||
        (r->guest_base<0x70004000U&&(u64)r->guest_base+r->storage_bytes>0x70000000ULL)||
        r->packet_guest!=0x242340U||r->grid_guest!=0x26FF20U||r->actor_pool_pointer_guest!=0x222D14U||
        overlaps(info,sizeof(*info),request,sizeof(*request))||overlaps(info,sizeof(*info),r->storage,r->storage_bytes))
        return DREADZONE_ACTOR_WALL_INVALID;
    w.copy=(u8 *)malloc(r->storage_bytes);if(!w.copy)return DREADZONE_ACTOR_WALL_TRANSACTION_MEMORY;
    memcpy(w.copy,r->storage,r->storage_bytes);
    for(i=0;i<3;++i)w.info.outgoing_vf25_29_w[i]=r->incoming_vf25_27_w[i];
    for(i=0;i<2;++i)w.info.outgoing_vf25_29_w[i+3]=w.request.incoming_vf28_29_w[i];
    {u8 *p=memory(&w,w.request.record_guest,0x31F0,16);
     if(p)memcpy(before,p+0x130,16);}
    result=w.failed?w.failed:body(&w);
    if(result>0) {
        u8 *p=memory(&w,w.request.record_guest+0x130,16,16);
        if(p)w.info.velocity_changed=memcmp(before,p,16)!=0;
        if(w.failed)result=w.failed;
    }
    if(result>0){memcpy(r->storage,w.copy,r->storage_bytes);memcpy(info,&w.info,sizeof(w.info));}
    free(w.copy);return result;
}
