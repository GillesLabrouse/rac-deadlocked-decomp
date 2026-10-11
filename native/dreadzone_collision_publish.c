#include "dreadzone_collision_publish.h"
#include <stdlib.h>
#include <string.h>

typedef struct TableRun {
    DreadzoneCollisionPublishRequest request;
    DreadzoneCollisionPublishInfo info;
    int status;
} TableRun;
static int span(const void *p,uintptr_t bytes) {
    return p && (uintptr_t)p<=UINTPTR_MAX-bytes;
}
static int overlap(const void *p,uintptr_t n,const void *q,uintptr_t m) {
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;
    return a>=b?a-b<m:b-a<n;
}
static u8 *word_address(TableRun *r,u32 address) {
    u32 base=r->request.guest_base,bytes=r->request.storage_bytes;
    if((address&3) || address<base || bytes<4 || address-base>bytes-4) {
        r->status=DREADZONE_COLLISION_PUBLISH_INVALID;return NULL;
    }
    return r->request.storage+(address-base);
}
static u32 word(TableRun *r,u32 address) {
    u32 value=0;u8 *p;
    if(r->status<0)return 0;
    p=word_address(r,address);if(p)memcpy(&value,p,4);
    return value;
}
static s32 count(TableRun *r,u32 address) {
    u32 raw=word(r,address);s32 value;memcpy(&value,&raw,4);return value;
}
static void store(TableRun *r,u32 address,u32 value) {
    u8 *p;if(r->status<0)return;
    p=word_address(r,address);if(p)memcpy(p,&value,4);
}
static void table(TableRun *r,u32 container,u32 offset,u32 global,u32 *iterations) {
    u32 relative=word(r,container+offset),pointer=container+relative,index=0;
    s32 limit;
    if(r->status<0)return;
    if(!relative) {store(r,global,0);++r->info.published_words;return;}
    store(r,global,pointer);++r->info.published_words;
    limit=count(r,pointer);
    if(limit<=0 || r->status<0)return;
    do {
        u32 destination,payload;
        if(index>=65536U) {r->status=DREADZONE_COLLISION_PUBLISH_UNSUPPORTED;return;}
        pointer=word(r,global);
        destination=pointer+0x1CU+(index<<4);
        ++index;
        payload=word(r,destination);
        store(r,destination,pointer+payload);
        if(r->status<0)return;
        ++*iterations;
        pointer=word(r,global);
        limit=count(r,pointer);
        /* Original branch-delay GP load executes even on loop exit. */
        (void)word(r,global);
    } while(r->status>0 && (s32)index<limit);
}
int dreadzone_collision_publish_00577C70(const DreadzoneCollisionPublishRequest *request,DreadzoneCollisionPublishInfo *info) {
    TableRun r;u8 *destination,*shadow;u32 container,relative;int status;
    if(!span(request,sizeof(*request)) || !span(info,sizeof(*info)) ||
       overlap(request,sizeof(*request),info,sizeof(*info)))return DREADZONE_COLLISION_PUBLISH_INVALID;
    memset(&r,0,sizeof(r));r.request=*request;container=r.request.container_guest;
    if(!r.request.storage_bytes || r.request.storage_bytes>16U*1024U*1024U ||
       r.request.guest_base>=0x80000000U || r.request.storage_bytes>0x80000000U-r.request.guest_base ||
       !span(r.request.storage,r.request.storage_bytes) || (container&3) ||
       overlap(info,sizeof(*info),r.request.storage,r.request.storage_bytes))return DREADZONE_COLLISION_PUBLISH_INVALID;
    destination=r.request.storage;
    shadow=(u8 *)malloc(r.request.storage_bytes);if(!shadow)return DREADZONE_COLLISION_PUBLISH_MEMORY;
    memcpy(shadow,destination,r.request.storage_bytes);r.request.storage=shadow;r.status=DREADZONE_COLLISION_PUBLISH_OK;
    relative=word(&r,container);
    if(relative) {store(&r,0x242340,container+relative);++r.info.published_words;}
    table(&r,container,4,0x222BE0,&r.info.main_relocations);
    table(&r,container,8,0x222BE4,&r.info.alternative_relocations);
    status=r.status;
    if(status>0) {memcpy(destination,shadow,r.request.storage_bytes);*info=r.info;}
    free(shadow);return status;
}
