#include "dreadzone_actor_visibility.h"
#include <string.h>
static int span(const void *p,uintptr_t n) {return p&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *a,uintptr_t n,const void *b,uintptr_t m) {
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x>=y?x-y<m:y-x<n;
}
static u8 *map(const DreadzoneActorVisibilityRequest *q,u32 a,u32 n,u32 alignment) {
    if((a&(alignment-1U))||a<q->guest_base||n>q->storage_bytes||a-q->guest_base>q->storage_bytes-n)return NULL;
    return q->storage+a-q->guest_base;
}
int dreadzone_actor_visibility_005A8630(const DreadzoneActorVisibilityRequest *request,DreadzoneActorVisibilityInfo *info) {
    DreadzoneActorVisibilityRequest q;DreadzoneActorVisibilityInfo result={0};
    u8 *p,*definition_byte;u32 definition;u16 flags,updated;
    if(!span(request,sizeof(*request))||!span(info,sizeof(*info))||overlap(request,sizeof(*request),info,sizeof(*info)))return -1;
    memcpy(&q,request,sizeof(q));
    if(!q.storage_bytes||q.storage_bytes>16U*1024U*1024U||!span(q.storage,q.storage_bytes)||
       (u64)q.guest_base+q.storage_bytes>0x80000000ULL||
       (q.guest_base<0x70004000U&&(u64)q.guest_base+q.storage_bytes>0x70000000ULL)||
       overlap(info,sizeof(*info),q.storage,q.storage_bytes))return -1;
    p=map(&q,q.actor_guest+0x24U,4,4);if(!p)return -1;memcpy(&definition,p,4);
    if(!definition){result.skipped=1;memcpy(info,&result,sizeof(result));return 1;}
    definition_byte=map(&q,definition+0xBU,1,1);if(!definition_byte)return -1;
    if(!*definition_byte){result.skipped=1;memcpy(info,&result,sizeof(result));return 1;}
    p=map(&q,q.actor_guest+0x36U,2,2);if(!p)return -1;memcpy(&flags,p,2);
    flags=(u16)((flags|0x8000U)&0x87FFU);
    updated=(u16)(flags|(q.mode_word==10U?0x5000U:q.mode_word<<11));
    /* Both original halfword stores are represented in their actual order.
       All mappings and metadata reads precede these stores, so failure is atomic. */
    memcpy(p,&flags,2);memcpy(p,&updated,2);
    result.halfword_stores=2;result.flags=updated;memcpy(info,&result,sizeof(result));return 1;
}
