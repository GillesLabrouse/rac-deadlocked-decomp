#include "dreadzone_actor_idle.h"
#include "dreadzone_actor_velocity.h"
#include <fenv.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Idle composition requires binary32
#endif
typedef char idle_binary32_size[sizeof(f32)==4?1:-1];
typedef struct Work {
    DreadzoneActorIdleRequest request;
    DreadzoneActorIdleInfo info;
    u8 *copy;
    DreadzoneActorInputContext context;
    DreadzoneActorInputMemory views[2];
    int failed;
} Work;
static int span(const void *p,uintptr_t n){return p&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *p,uintptr_t n,const void *q,uintptr_t m){
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;return a>=b?a-b<m:b-a<n;
}
static u8 *at(Work *w,u32 address,u32 bytes,u32 alignment){
    DreadzoneWallRequest *r=&w->request.wall;
    if(w->failed)return NULL;
    if((address&(alignment-1U))||address<r->guest_base||bytes>r->storage_bytes||
       address-r->guest_base>r->storage_bytes-bytes){w->failed=-1;return NULL;}
    return w->copy+address-r->guest_base;
}
static u32 word(Work *w,u32 o){u32 v=0;u8 *p=at(w,w->request.record_guest+o,4,4);if(p)memcpy(&v,p,4);return v;}
static u8 byte(Work *w,u32 o){u8 *p=at(w,w->request.record_guest+o,1,1);return p?*p:0;}
static s16 half(Work *w,u32 o){s16 v=0;u8 *p=at(w,w->request.record_guest+o,2,2);if(p)memcpy(&v,p,2);return v;}
static f32 checked(Work *w,f32 value,int input){
    u32 m;memcpy(&m,&value,4);m&=0x7FFFFFFFU;
    if(m&&(m<0x00800000U||m>=0x7F800000U||(input&&(m<0x2B800000U||m>0x41800000U))))
        if(!w->failed)w->failed=-3;
    return value;
}
static f32 load(Work *w,u32 address){f32 v=0;u8 *p=at(w,address,4,4);if(p)memcpy(&v,p,4);return checked(w,v,1);}
static f32 product(Work *w,f32 a,f32 b){volatile f32 v=a*b;
    if(a!=0.f&&b!=0.f&&v==0.f&&!w->failed)w->failed=-3;
    return checked(w,v,0);
}
static int nested(Work *w,int result){if(result<0&&!w->failed)w->failed=-2;return !w->failed;}
static int body(Work *w){
    u32 b=w->request.record_guest,index;u8 *p;u8 quad[16],scalar[4];f32 rate,scale,time,offset;
    DreadzoneMotionProbeRequest motion={w->copy,w->request.wall.guest_base,w->request.wall.storage_bytes,b};
    DreadzoneActorWallResponseRequest wall={0};
    if(word(w,0x25D8)||byte(w,0x2661))return w->failed?w->failed:-2;
    if(!nested(w,dreadzone_actor_motion_probe_006840B8(&motion,&w->info.motion)))return w->failed;
    index=word(w,0x2F10);if(w->failed)return w->failed;
    if(index>1)return -2;
    p=at(w,0x171DE0U+index,1,1);if(!p)return w->failed;if(!*p)return -2;
    p=at(w,b+0x1970U,16,16);if(!p)return w->failed;memcpy(quad,p,16);
    p=at(w,b+0x19B0U,4,4);if(!p)return w->failed;memcpy(scalar,p,4);
    p=at(w,b+0x1790U,16,16);if(!p)return w->failed;memcpy(p,quad,16);
    p=at(w,b+0x17A0U,4,4);if(!p)return w->failed;memcpy(p,scalar,4);
    if(word(w,0x25CC)||byte(w,0x2ECE)||half(w,0x3BA)||byte(w,0x2ECD))return w->failed?w->failed:-2;
    p=at(w,b+0x1780U,4,4);if(!p)return w->failed;memset(p,0,4);
    /* The branch's real delay load uses GP, not a substituted frame duration. */
    if(w->request.gp_guest<0x7B87U)return -1;
    time=load(w,w->request.gp_guest-0x7B87U);scale=load(w,b+0x25C4U);
    rate=product(w,product(w,time,17.7f),scale);
    if(w->failed)return w->failed;
    ++w->info.speed_calls;if(!nested(w,dreadzone_actor_speed_0067D350(&w->context,0.f,rate)))return w->failed;
    p=at(w,b+0x130U,16,16);if(!p)return w->failed;memset(p,0,16);
    /* Zero-planar accumulated input reaches no VF28/29 producer. Limit all
     * lanes as well, since the genuine length helper squares the full quad. */
    {u32 v[4];p=at(w,b+0x1A0U,16,16);if(!p)return w->failed;memcpy(v,p,16);
     if((v[0]&0x7FFFFFFFU)||(v[1]&0x7FFFFFFFU)||(v[2]&0x7FFFFFFFU)||(v[3]&0x7FFFFFFFU))return -2;}
    ++w->info.accumulation_calls;if(!nested(w,dreadzone_actor_accumulated_motion_006834C8(&w->context,rate)))return w->failed;
    if(byte(w,0x437))return w->failed?w->failed:-2;
    wall.wall=w->request.wall;wall.wall.storage=w->copy;wall.record_guest=b;
    memcpy(wall.incoming_vf28_29_w,w->request.incoming_vf28_29_w,8);
    if(!nested(w,dreadzone_actor_wall_response_0067D6E8(&wall,&w->info.wall)))return w->failed;
    if(half(w,0x2FE)||word(w,0x25CC)==0x94U)return w->failed?w->failed:-2;
    time=load(w,0x21DDF8U);scale=load(w,b+0x25C4U);offset=product(w,product(w,time,54.f),scale);
    if(w->failed)return w->failed;
    ++w->info.height_calls;if(!nested(w,dreadzone_actor_height_offset_0068A178(&w->context,b+0x130U,b+0x130U,offset)))return w->failed;
    return 1;
}
static int numeric_host(void){volatile f32 small=0x1p-126f,half_minimum=small*.5f;
    f32 value=half_minimum;u32 raw;memcpy(&raw,&value,4);return fegetround()==FE_TONEAREST&&raw==0x00400000U;
}
int dreadzone_actor_idle_00695570(const DreadzoneActorIdleRequest *request,DreadzoneActorIdleInfo *info){
    Work w;DreadzoneWallRequest *r;u32 o,tail,n=0;int result;
    if(!span(request,sizeof(*request))||!span(info,sizeof(*info))||overlap(request,sizeof(*request),info,sizeof(*info)))return -1;
    memset(&w,0,sizeof(w));memcpy(&w.request,request,sizeof(*request));r=&w.request.wall;
    if(!r->storage_bytes||r->storage_bytes>16U*1024U*1024U||!span(r->storage,r->storage_bytes)||
       r->guest_base>=0x10000000U||r->storage_bytes>0x10000000U-r->guest_base||
       w.request.gp_guest!=0x22597FU||
       overlap(info,sizeof(*info),r->storage,r->storage_bytes)||
       (w.request.record_guest&15U)||w.request.record_guest<r->guest_base||r->storage_bytes<0x31F0U||
       w.request.record_guest-r->guest_base>r->storage_bytes-0x31F0U)return -1;
    if(!numeric_host())return -3;
    w.copy=(u8 *)malloc(r->storage_bytes);if(!w.copy)return -4;
    memcpy(w.copy,r->storage,r->storage_bytes);o=w.request.record_guest-r->guest_base;tail=r->storage_bytes-o-0x31F0U;
    if(o)w.views[n++]=(DreadzoneActorInputMemory){w.copy,r->guest_base,o,1};
    if(tail)w.views[n++]=(DreadzoneActorInputMemory){w.copy+o+0x31F0U,w.request.record_guest+0x31F0U,tail,1};
    w.context=(DreadzoneActorInputContext){w.copy+o,w.request.record_guest,0x31F0U,w.views,n};
    result=body(&w);if(result>0){memcpy(r->storage,w.copy,r->storage_bytes);memcpy(info,&w.info,sizeof(*info));}
    free(w.copy);return result;
}
