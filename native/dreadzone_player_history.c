/* Bounded original integer RAM order; original API/storage remain hypotheses. */
#include "dreadzone_player_history.h"
#include <stdlib.h>
#include <string.h>

typedef struct HistoryRun {
    DreadzonePlayerHistoryRequest request;
    DreadzonePlayerHistoryInfo info;
    uint8_t *shadow;
    int status;
} HistoryRun;
static int span(const void *p,size_t n) {
    return p && (uintptr_t)p<=UINTPTR_MAX-n;
}
static int overlap(const void *a,size_t n,const void *b,size_t m) {
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
    return x>=y?x-y<m:y-x<n;
}
static uint8_t *field(HistoryRun *r,uint32_t offset) {
    return r->shadow+r->request.record_guest-r->request.guest_base+offset;
}
static uint32_t word(HistoryRun *r,uint32_t offset) {
    uint32_t value;memcpy(&value,field(r,offset),4);return value;
}
static void store(HistoryRun *r,uint32_t offset,uint32_t value) {
    memcpy(field(r,offset),&value,4);
}
static int counters(HistoryRun *r,uint32_t cursor,uint32_t count) {
    if(cursor>31 || count>32){r->status=-2;return 0;}
    return 1;
}
static void angle_history(HistoryRun *r) {
    uint32_t cursor=word(r,0x2C70),angle;
    if(cursor>31){r->status=-2;return;}
    angle=word(r,0xB8);store(r,0x28F0+cursor*4,angle);
    r->info.angle_slot=cursor;
    cursor=word(r,0x2C70);
    {
        uint32_t count=word(r,0x2C74);
        if(!counters(r,cursor,count))return;
        store(r,0x2C74,count<32?count+1:32);
        /* 67C208 JAL delay store precedes the position callee. */
        store(r,0x2C70,(cursor+1)&31);
    }
    r->info.angle_written=1;
}
static void position_history(HistoryRun *r) {
    uint32_t cursor=word(r,0x2C78),raw[4];
    if(cursor>31){r->status=-2;return;}
    /* 68EFF0 LQ captures all source words before 68F000 SQ. */
    memcpy(raw,field(r,0xA0),16);
    memcpy(field(r,0x26F0+cursor*16),raw,16);
    r->info.position_slot=cursor;
    cursor=word(r,0x2C78);
    {
        uint32_t count=word(r,0x2C7C);
        if(!counters(r,cursor,count))return;
        store(r,0x2C7C,count<32?count+1:32);
        /* Actual JR delay publishes cursor after count. */
        store(r,0x2C78,(cursor+1)&31);
    }
    r->info.position_written=1;
}
static int execute(const DreadzonePlayerHistoryRequest *request,
                   DreadzonePlayerHistoryInfo *info,int with_angle) {
    HistoryRun r;uint32_t n,b,base;
    if(!span(request,sizeof(*request)) || !span(info,sizeof(*info)) ||
       overlap(request,sizeof(*request),info,sizeof(*info)))return -1;
    memset(&r,0,sizeof(r));memcpy(&r.request,request,sizeof(r.request));
    n=r.request.storage_bytes;b=r.request.record_guest;base=r.request.guest_base;
    if(!n || n>16u*1024u*1024u || !span(r.request.storage,n) ||
       base>=0x80000000u || n>0x80000000u-base ||
       (base<0x70004000u && base+n>0x70000000u) ||
       overlap(r.request.storage,n,info,sizeof(*info)) || (b&15) || b<base ||
       b-base>n || 0x2C80u>n-(b-base))return -1;
    r.shadow=(uint8_t *)malloc(n);if(!r.shadow)return -4;
    memcpy(r.shadow,r.request.storage,n);r.status=1;
    if(with_angle)angle_history(&r);
    if(r.status>0)position_history(&r);
    if(r.status>0){memcpy(r.request.storage,r.shadow,n);memcpy(info,&r.info,sizeof(*info));}
    free(r.shadow);return r.status;
}
int dreadzone_player_position_history_0068EFE8(
    const DreadzonePlayerHistoryRequest *request,DreadzonePlayerHistoryInfo *info) {
    return execute(request,info,0);
}
int dreadzone_player_history_before_basis(
    const DreadzonePlayerHistoryRequest *request,DreadzonePlayerHistoryInfo *info) {
    return execute(request,info,1);
}
