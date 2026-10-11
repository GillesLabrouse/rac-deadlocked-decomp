#include "dreadzone_actor_float_timer.h"
#include <math.h>
#include <string.h>
static int span(const void *p,uintptr_t n){return p&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *a,uintptr_t n,const void *b,uintptr_t m){
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x>=y?x-y<m:y-x<n;
}
static u8 *map(const DreadzoneActorFloatTimerRequest *q,u32 a){
    if((a&3U)||a<q->guest_base||q->storage_bytes<4||a-q->guest_base>q->storage_bytes-4)return NULL;
    return q->storage+a-q->guest_base;
}
static int input(u32 word,float maximum,float *out){
    u32 magnitude=word&0x7FFFFFFFU;memcpy(out,&word,4);
    return (!magnitude||(magnitude>=0x2B800000U&&magnitude<0x7F800000U))&&fabsf(*out)<=maximum;
}
int dreadzone_actor_float_timer_0055D240(const DreadzoneActorFloatTimerRequest *request,DreadzoneActorFloatTimerInfo *info){
    DreadzoneActorFloatTimerRequest q;DreadzoneActorFloatTimerInfo result;u8 *threshold_pointer,*timer_pointer;
    u32 threshold_word,timer_word,magnitude;float threshold,timer;volatile float difference;
    if(!span(request,sizeof(*request))||!span(info,sizeof(*info))||overlap(request,sizeof(*request),info,sizeof(*info)))return -1;
    memcpy(&q,request,sizeof(q));
    if(!q.storage_bytes||q.storage_bytes>16U*1024U*1024U||!span(q.storage,q.storage_bytes)||
       (u64)q.guest_base+q.storage_bytes>0x80000000ULL||
       (q.guest_base<0x70004000U&&(u64)q.guest_base+q.storage_bytes>0x70000000ULL)||
       overlap(info,sizeof(*info),q.storage,q.storage_bytes))return -1;
    threshold_pointer=map(&q,q.gp_guest-0x7B7BU);if(!threshold_pointer)return -1;
    memcpy(&threshold_word,threshold_pointer,4);
    timer_pointer=map(&q,q.timer_guest);if(!timer_pointer)return -1;memcpy(&timer_word,timer_pointer,4);
    if(!input(threshold_word,16.f,&threshold)||!input(timer_word,8192.f,&timer))return -2;
    if(timer<=threshold){
        result.stored_word=0;
        result.returned=(timer_word==0||(timer_word&0x80000000U))?-1:-2;
    }else{
        difference=timer-threshold;memcpy(&result.stored_word,(const void *)&difference,4);
        magnitude=result.stored_word&0x7FFFFFFFU;
        if(magnitude&&(magnitude<0x00800000U||magnitude>=0x7F800000U))return -2;
        result.returned=0;
    }
    memcpy(timer_pointer,&result.stored_word,4);memcpy(info,&result,sizeof(result));return 1;
}
