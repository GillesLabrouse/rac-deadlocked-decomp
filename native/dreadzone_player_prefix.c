#include "dreadzone_player_prefix.h"
#include "dreadzone_equipment_selection.h"
#include <stdlib.h>
#include <string.h>
typedef struct Work {
    DreadzonePlayerPrefixRequest request;
    DreadzonePlayerPrefixInfo info;
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
static u32 word(Work *w,u32 address){u32 v=0;u8 *p=at(w,address,4,4);if(p)memcpy(&v,p,4);return v;}
static u8 byte(Work *w,u32 address){u8 *p=at(w,address,1,1);return p?*p:0;}
/* Raw5A8628 is JR RA/NOP. Its complete RAM effect is empty; no return value
 * or original register/stack effect is invented by this native RAM model. */
static void leaf_005A8628(void){}
static int callback(Work *w,u32 offset,u32 expected){
    u32 table=word(w,w->request.record_guest+0x14U);
    if(w->failed)return 0;
    if(table>UINT32_MAX-offset){w->failed=-1;return 0;}
    if(word(w,table+offset)!=expected&&!w->failed)w->failed=-2;
    return !w->failed;
}
static int body(Work *w){
    u32 b=w->request.record_guest;u8 *p;u8 position[16];int result;
    leaf_005A8628();w->info.stub_calls=1;
    {u8 active=byte(w,b+0x266DU);
     (void)word(w,w->request.gp_guest-0x715BU); /* Actual67C034 branch delay read even if unused. */
     if(w->failed)return w->failed;if(!active)return -2;}
    if(!callback(w,0x4CU,0x6A9630U))return w->failed;
    p=at(w,b,0x31F0U,16);if(!p)return w->failed;
    w->info.getter_index=(u32)func_L01_006A9630(p);w->info.getter_calls=1;
    {
        DreadzoneSlotWordRequest r={w->copy,w->request.wall.guest_base,w->request.wall.storage_bytes,w->info.getter_index};
        ++w->info.slot_calls;result=dreadzone_slot_word_00535A58(&r,&w->info.slot);if(result<0)return -2;
    }
    if(word(w,0x21E824U)==1U)return w->failed?w->failed:-2;
    p=at(w,b+0xA0U,16,16);if(!p)return w->failed;memcpy(position,p,16);
    p=at(w,b+0x110U,16,16);if(!p)return w->failed;memcpy(p,position,16);
    if(word(w,b+0x25CCU)!=0U)return w->failed?w->failed:-2;
    {
        DreadzoneActorModeRequest r={w->copy,w->request.wall.guest_base,w->request.wall.storage_bytes,b};
        ++w->info.mode_calls;result=dreadzone_actor_mode_0068A090(&r,&w->info.mode);if(result<0)return -2;
    }
    p=at(w,b+0x2659U,1,1);if(!p)return w->failed;*p=(u8)w->info.mode.mode;
    p=at(w,b+0x210U,4,4);if(!p)return w->failed;memset(p,0,4);
    if(word(w,0x21E824U)==1U)return w->failed?w->failed:-2;
    if(!callback(w,0x34U,0x690F18U))return w->failed;
    {
        DreadzonePlayerDispatchRequest r=w->request;r.wall.storage=w->copy;
        ++w->info.dispatch_calls;result=dreadzone_player_dispatch_00690F18(&r,&w->info.dispatch);if(result<0)return -2;
    }
    if(!byte(w,b+0x266DU))return w->failed?w->failed:-2;
    p=at(w,b+0xF0U,16,16);if(!p)return w->failed;memcpy(w->info.retained_f0_words,p,16);
    return 1;
}
int dreadzone_player_prefix_0067C010(const DreadzonePlayerPrefixRequest *request,DreadzonePlayerPrefixInfo *info){
    Work w;DreadzoneWallRequest *r;int result;
    if(!span(request,sizeof(*request))||!span(info,sizeof(*info))||overlap(request,sizeof(*request),info,sizeof(*info)))return -1;
    memset(&w,0,sizeof(w));memcpy(&w.request,request,sizeof(*request));r=&w.request.wall;
    if(!r->storage_bytes||r->storage_bytes>16U*1024U*1024U||!span(r->storage,r->storage_bytes)||
       r->guest_base>=0x10000000U||r->storage_bytes>0x10000000U-r->guest_base||w.request.gp_guest!=0x22597FU||
       overlap(info,sizeof(*info),r->storage,r->storage_bytes)||
       (w.request.record_guest&15U)||w.request.record_guest<r->guest_base||r->storage_bytes<0x31F0U||
       w.request.record_guest-r->guest_base>r->storage_bytes-0x31F0U)return -1;
    w.copy=(u8 *)malloc(r->storage_bytes);if(!w.copy)return -4;
    memcpy(w.copy,r->storage,r->storage_bytes);result=body(&w);
    if(result>0){memcpy(r->storage,w.copy,r->storage_bytes);memcpy(info,&w.info,sizeof(*info));}
    free(w.copy);return result;
}
