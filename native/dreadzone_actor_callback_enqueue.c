#include "dreadzone_actor_callback_enqueue.h"
#include <stdlib.h>
#include <string.h>
typedef struct CallbackRun {DreadzoneCallbackEnqueueRequest q;DreadzoneCallbackEnqueueInfo info;int failed;} CallbackRun;
static int span(const void *p,uintptr_t n) {return p&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *a,uintptr_t n,const void *b,uintptr_t m) {
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x>=y?x-y<m:y-x<n;
}
static u8 *at(CallbackRun *r,u32 address,u32 n,u32 align) {
    if(r->failed)return NULL;
    if((address&(align-1U))||address<r->q.guest_base||n>r->q.storage_bytes||
       address-r->q.guest_base>r->q.storage_bytes-n) {r->failed=DREADZONE_CALLBACK_ENQUEUE_INVALID;return NULL;}
    return r->q.storage+address-r->q.guest_base;
}
static u32 word(CallbackRun *r,u32 a) {u32 v=0;u8 *p=at(r,a,4,4);if(p)memcpy(&v,p,4);return v;}
static void store(CallbackRun *r,u32 a,u32 v) {u8 *p=at(r,a,4,4);if(p)memcpy(p,&v,4);}
static void run(CallbackRun *r) {
    u32 record=r->q.record_guest,argument,count,entry,head,tail;u8 *flag;s32 signed_count;
    flag=at(r,record+0x25C8,1,1);if(!flag)return;
    if(*flag) {r->info.skipped=1;return;}
    argument=word(r,record+0x2EF0);if(r->failed)return;
    *flag=1;r->info.queue_calls=1;
    count=word(r,0x222AF4);memcpy(&signed_count,&count,4);if(r->failed)return;
    if(signed_count>=256) {r->info.capacity_exits=1;return;}
    entry=0x241140U+(count<<4);
    store(r,0x222AF4,count+1U);
    store(r,entry,0x66ACA0);store(r,entry+4U,argument);
    head=word(r,0x222A9C);
    if(!head)store(r,0x222A9C,entry);
    else {tail=word(r,head+12U);store(r,tail+8U,entry);}
    head=word(r,0x222A9C);store(r,head+12U,entry);
    if(!r->failed){r->info.queued=1;r->info.entry_guest=entry;r->info.argument_guest=argument;}
}
int dreadzone_actor_callback_enqueue_0066ABD8(const DreadzoneCallbackEnqueueRequest *request,DreadzoneCallbackEnqueueInfo *info) {
    CallbackRun r;u8 *shadow,*destination;int status;
    if(!span(request,sizeof(*request))||!span(info,sizeof(*info))||overlap(request,sizeof(*request),info,sizeof(*info)))return DREADZONE_CALLBACK_ENQUEUE_INVALID;
    memset(&r,0,sizeof(r));memcpy(&r.q,request,sizeof(r.q));
    if(!r.q.storage_bytes||r.q.storage_bytes>16U*1024U*1024U||!span(r.q.storage,r.q.storage_bytes)||
       (u64)r.q.guest_base+r.q.storage_bytes>0x80000000ULL||
       (r.q.guest_base<0x70004000U&&(u64)r.q.guest_base+r.q.storage_bytes>0x70000000ULL)||
       overlap(info,sizeof(*info),r.q.storage,r.q.storage_bytes)||!at(&r,r.q.record_guest,0x31F0,16))return DREADZONE_CALLBACK_ENQUEUE_INVALID;
    destination=r.q.storage;shadow=(u8 *)malloc(r.q.storage_bytes);if(!shadow)return DREADZONE_CALLBACK_ENQUEUE_MEMORY;
    memcpy(shadow,destination,r.q.storage_bytes);r.q.storage=shadow;run(&r);status=r.failed?r.failed:DREADZONE_CALLBACK_ENQUEUE_OK;
    if(status>0){memcpy(destination,shadow,r.q.storage_bytes);memcpy(info,&r.info,sizeof(*info));}
    free(shadow);return status;
}
