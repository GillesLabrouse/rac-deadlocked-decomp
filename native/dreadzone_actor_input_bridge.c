#include "dreadzone_actor_input_bridge.h"
#include <stdlib.h>
#include <string.h>

#define BRIDGE_MAX_BYTES (16U*1024U*1024U)
static int host_span(const void *p,uintptr_t bytes) {
    return p && (uintptr_t)p<=UINTPTR_MAX-bytes;
}
static int host_overlap(const void *a,u32 an,const void *b,u32 bn) {
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
    return x>=y?x-y<bn:y-x<an;
}
static int guest_overlap(u32 a,u32 an,u32 b,u32 bn) {
    return a>=b?a-b<bn:b-a<an;
}
u32 dreadzone_actor_input_word(const u8 *p) {
    return (u32)p[0]|(u32)p[1]<<8|(u32)p[2]<<16|(u32)p[3]<<24;
}
void dreadzone_actor_input_store32(u8 *p,u32 value) {
    p[0]=(u8)value;p[1]=(u8)(value>>8);p[2]=(u8)(value>>16);p[3]=(u8)(value>>24);
}
static void region(const DreadzoneActorInputContext *ctx,u32 i,DreadzoneActorInputMemory *r) {
    if(i) *r=ctx->views[i-1];
    else {*r=(DreadzoneActorInputMemory){ctx->record,ctx->record_guest,ctx->record_bytes,1};}
}
static int valid(const DreadzoneActorInputContext *ctx) {
    DreadzoneActorInputMemory a,b;
    u32 i,j,descriptors;
    if(!host_span(ctx,sizeof(*ctx)) || ctx->view_count>64) return 0;
    descriptors=ctx->view_count*(u32)sizeof(*ctx->views);
    if(descriptors && (!host_span(ctx->views,descriptors) ||
       host_overlap(ctx,sizeof(*ctx),ctx->views,descriptors))) return 0;
    if(ctx->record_bytes<0x31F0 || (ctx->record_guest&15)) return 0;
    for(i=0;i<=ctx->view_count;++i) {
        region(ctx,i,&a);
        if(!a.bytes || !host_span(a.storage,a.bytes) ||
           a.guest_base>0xFFFFFFFFU-(a.bytes-1) || (a.writable!=0 && a.writable!=1)) return 0;
        if(host_overlap(a.storage,a.bytes,ctx,sizeof(*ctx)) ||
           (descriptors && host_overlap(a.storage,a.bytes,ctx->views,descriptors))) return 0;
        for(j=0;j<i;++j) {
            region(ctx,j,&b);
            if(host_overlap(a.storage,a.bytes,b.storage,b.bytes) ||
               guest_overlap(a.guest_base,a.bytes,b.guest_base,b.bytes)) return 0;
        }
    }
    return 1;
}
static u8 *map(const DreadzoneActorInputContext *ctx,u32 address,u32 bytes,int write) {
    DreadzoneActorInputMemory r;u32 i;
    if(!bytes || address>0xFFFFFFFFU-(bytes-1)) return 0;
    for(i=0;i<=ctx->view_count;++i) {
        region(ctx,i,&r);
        if((!write || r.writable) && address>=r.guest_base && bytes<=r.bytes &&
           address-r.guest_base<=r.bytes-bytes) return r.storage+(address-r.guest_base);
    }
    return 0;
}
const u8 *dreadzone_actor_input_read(const DreadzoneActorInputContext *ctx,u32 address,u32 bytes) {
    return map(ctx,address,bytes,0);
}
u8 *dreadzone_actor_input_write(const DreadzoneActorInputContext *ctx,u32 address,u32 bytes) {
    return map(ctx,address,bytes,1);
}
int dreadzone_actor_input_predicate(const DreadzoneActorInputContext *ctx,u8 *record,s32 *result) {
    DreadzoneSnapshotMemory views[65];
    DreadzoneSnapshotContext snapshot;
    DreadzoneActorInputMemory r;
    const u8 *modes,*state;
    u32 i,index=dreadzone_actor_input_word(record+0x2F10);
    int status;
    if(index>1) return DREADZONE_ACTOR_INPUT_UNSUPPORTED_INDEX;
    modes=dreadzone_actor_input_read(ctx,0x00171DE0,2);
    if(!modes) return DREADZONE_ACTOR_INPUT_NEEDS_MEMORY;
    snapshot.record=record;snapshot.record_guest=ctx->record_guest;snapshot.record_bytes=ctx->record_bytes;
    snapshot.mode_bytes_00171DE0=modes;snapshot.mode_count=2;
    snapshot.state_0021DDB4=0; /* Unconsumed unless the actual predicate needs it. */
    if(modes[index] && !dreadzone_actor_input_word(record+0x3F4)) {
        state=dreadzone_actor_input_read(ctx,0x0021DDB4,4);
        if(!state) return DREADZONE_ACTOR_INPUT_NEEDS_MEMORY;
        snapshot.state_0021DDB4=(s32)dreadzone_actor_input_word(state);
    }
    for(i=0;i<=ctx->view_count;++i) {
        region(ctx,i,&r);views[i]=(DreadzoneSnapshotMemory){r.storage,r.guest_base,r.bytes};
    }
    snapshot.views=views;snapshot.view_count=ctx->view_count+1;
    status=dreadzone_actor_0068DE60(&snapshot,result);
    return status?DREADZONE_ACTOR_INPUT_NEEDS_PREDICATE_MAPPING:DREADZONE_ACTOR_INPUT_COMPLETED;
}
static int run(const DreadzoneActorInputContext *ctx,int bridge) {
    DreadzoneActorInputContext transaction;
    DreadzoneActorInputMemory original[65],copies[64];
    u8 *buffers[65]={0};u32 i,total=0;
    int status;
    if(!valid(ctx)) return DREADZONE_ACTOR_INPUT_INVALID_STORAGE;
    for(i=0;i<=ctx->view_count;++i) {
        region(ctx,i,original+i);
        if(original[i].bytes>BRIDGE_MAX_BYTES-total) return DREADZONE_ACTOR_INPUT_NO_TRANSACTION_MEMORY;
        total+=original[i].bytes;
    }
    for(i=0;i<=ctx->view_count;++i) {
        buffers[i]=(u8 *)malloc(original[i].bytes);
        if(!buffers[i]) {status=DREADZONE_ACTOR_INPUT_NO_TRANSACTION_MEMORY;goto discard;}
        memcpy(buffers[i],original[i].storage,original[i].bytes);
        if(i) {copies[i-1]=original[i];copies[i-1].storage=buffers[i];}
    }
    transaction=*ctx;transaction.record=buffers[0];transaction.views=copies;
    status=bridge?func_L01_006A7D48(&transaction,transaction.record):
                  func_L01_0067BD58(&transaction,transaction.record);
    if(status<0) goto discard;
    for(i=0;i<=ctx->view_count;++i) if(original[i].writable)
        memcpy(original[i].storage,buffers[i],original[i].bytes);
discard:
    for(i=0;i<=ctx->view_count;++i) free(buffers[i]);
    return status;
}
int dreadzone_actor_input_006A7D48(const DreadzoneActorInputContext *ctx) {return run(ctx,1);}
int dreadzone_actor_input_0067BD58(const DreadzoneActorInputContext *ctx) {return run(ctx,0);}
