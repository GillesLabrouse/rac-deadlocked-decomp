#include "dreadzone_actor_motion_probe.h"
#include <float.h>
#include <string.h>
#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Motion probe requires binary32
#endif
typedef char idle_binary32_size[sizeof(f32)==4?1:-1];
static int span(const void *p,uintptr_t n){return p&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *p,uintptr_t n,const void *q,uintptr_t m){
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;return a>=b?a-b<m:b-a<n;
}
static u32 magnitude(f32 value){u32 bits;memcpy(&bits,&value,4);return bits&0x7FFFFFFFU;}
static int square_domain(f32 value){u32 m=magnitude(value);return !m||(m>=0x2B800000U&&m<=0x41800000U);}
int dreadzone_actor_motion_probe_006840B8(const DreadzoneMotionProbeRequest *request,DreadzoneMotionProbeInfo *info){
    DreadzoneMotionProbeRequest r;DreadzoneMotionProbeInfo out={0};VelocityVector v;
    const u8 *actor;f32 length;
    if(!span(request,sizeof(*request))||!span(info,sizeof(*info))||
       overlap(request,sizeof(*request),info,sizeof(*info)))return -1;
    memcpy(&r,request,sizeof(r));
    if(!r.storage_bytes||r.storage_bytes>16U*1024U*1024U||!span(r.storage,r.storage_bytes)||
       r.guest_base>=0x10000000U||r.storage_bytes>0x10000000U-r.guest_base||
       (r.record_guest&15U)||r.record_guest<r.guest_base||r.storage_bytes<0x31F0U||
       r.record_guest-r.guest_base>r.storage_bytes-0x31F0U||
       overlap(info,sizeof(*info),r.storage,r.storage_bytes))return -1;
    actor=r.storage+r.record_guest-r.guest_base;
    if(actor[0x2659])return -2;
    memcpy(&v,actor+0x130,16);
    if(magnitude(v.v[0])||magnitude(v.v[1])||!square_domain(v.v[2])||!square_domain(v.v[3]))return -2;
    length=func_L01_0068A920(actor,&v);
    if(length!=0.f)return -2;
    memcpy(&out.length_result_bits,&length,4);out.length_helper_calls=1;
    memcpy(info,&out,sizeof(out));return 1;
}
