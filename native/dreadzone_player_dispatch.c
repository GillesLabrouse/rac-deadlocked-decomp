#include "dreadzone_player_dispatch.h"
#include <stdlib.h>
#include <string.h>
typedef struct Work {
    DreadzonePlayerDispatchRequest request;
    DreadzonePlayerDispatchInfo info;
    u8 *copy;
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
static u32 word(Work *w,u32 address){u32 value=0;u8 *p=at(w,address,4,4);if(p)memcpy(&value,p,4);return value;}
static u8 byte(Work *w,u32 address){u8 *p=at(w,address,1,1);return p?*p:0;}
static void clear(Work *w,u32 address){u8 *p=at(w,address,4,4);if(p)memset(p,0,4);}
static int body(Work *w){
    u32 b=w->request.record_guest,object,substate,saved_state,current_state;u8 saved_byte,current_byte;int result;
    object=word(w,b+0x2EF0U);substate=word(w,b+0x25CCU);
    /* The original LBU uses the retained S7 object pointer, not a later reload
     * of actor2EF0. Reject unmapped/wrapping bytes without inventing an object. */
    if(w->failed)return w->failed;
    if(object>UINT32_MAX-0x54U)return -1;
    saved_byte=byte(w,object+0x54U);saved_state=word(w,b+0x25D0U);
    if(w->failed)return w->failed;
    if(substate!=0)return -2;
    if(word(w,0x3B08C0U)!=0x690FE0U)return w->failed?w->failed:-2;
    {
        DreadzoneActorIdleRequest idle=w->request;idle.wall.storage=w->copy;
        ++w->info.idle_calls;result=dreadzone_actor_idle_00695570(&idle,&w->info.idle);
    }
    if(result<0)return -2;
    current_state=word(w,b+0x25D0U); /* The actual690FEC branch delay load. */
    if(w->failed)return w->failed;
    if(saved_state!=current_state){clear(w,b+0x378U);w->info.cleared_378=1;}
    current_byte=byte(w,object+0x54U);
    if(w->failed)return w->failed;
    if(saved_byte!=current_byte){clear(w,b+0x37CU);w->info.cleared_37c=1;}
    return w->failed?w->failed:1;
}
int dreadzone_player_dispatch_00690F18(const DreadzonePlayerDispatchRequest *request,DreadzonePlayerDispatchInfo *info){
    Work w;DreadzoneWallRequest *r;int result;
    if(!span(request,sizeof(*request))||!span(info,sizeof(*info))||overlap(request,sizeof(*request),info,sizeof(*info)))return -1;
    memset(&w,0,sizeof(w));memcpy(&w.request,request,sizeof(*request));r=&w.request.wall;
    if(!r->storage_bytes||r->storage_bytes>16U*1024U*1024U||!span(r->storage,r->storage_bytes)||
       r->guest_base>=0x10000000U||r->storage_bytes>0x10000000U-r->guest_base||
       overlap(info,sizeof(*info),r->storage,r->storage_bytes)||
       (w.request.record_guest&15U)||w.request.record_guest<r->guest_base||r->storage_bytes<0x31F0U||
       w.request.record_guest-r->guest_base>r->storage_bytes-0x31F0U)return -1;
    w.copy=(u8 *)malloc(r->storage_bytes);if(!w.copy)return -4;
    memcpy(w.copy,r->storage,r->storage_bytes);result=body(&w);
    if(result>0){memcpy(r->storage,w.copy,r->storage_bytes);memcpy(info,&w.info,sizeof(*info));}
    free(w.copy);return result;
}
