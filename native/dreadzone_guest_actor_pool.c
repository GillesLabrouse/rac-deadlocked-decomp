#include "dreadzone_guest_actor_pool.h"
#include "dreadzone_actor_pool.h"
#include "dreadzone_fresh_class.h"
#include <stdlib.h>
#include <string.h>
/* Existing payload release/allocate sources share this internal validator. */
extern int dreadzone_payload_validate_arena(const DreadzonePayloadArena *, const DreadzonePayloadState *, u32);
static uint32_t r32(const uint8_t *p) { return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24; }
static void w32(uint8_t *p,uint32_t x) { p[0]=(uint8_t)x;p[1]=(uint8_t)(x>>8);p[2]=(uint8_t)(x>>16);p[3]=(uint8_t)(x>>24); }
static int overlap(uint64_t a,uint64_t n,uint64_t b,uint64_t m) { return a<b+m&&b<a+n; }
static uint8_t *map(const DreadzoneGuestActorAllocation *q,uint8_t *ram,uint32_t a,uint32_t n) {
    if(a<q->guest_base||(uint64_t)a+n>(uint64_t)q->guest_base+q->bytes)return NULL;
    return ram+a-q->guest_base;
}
int dreadzone_guest_actor_allocate(const DreadzoneGuestActorAllocation *request,uint32_t *result) {
    DreadzoneGuestActorAllocation q;
    uint8_t *ram=NULL,*globals,*counter,*clock,*pool,*actor=NULL,*p;
    uint8_t scratch[0x4000],vu[0x1000];
    uint32_t start,end,address,payload=0,descriptor_base,descriptor_end,payload_base,capacity,i;
    int status=-2;
    if(!request||!result||sizeof(*request)>UINTPTR_MAX-(uintptr_t)request||
       sizeof(*result)>UINTPTR_MAX-(uintptr_t)result)return -1;
    q=*request;
    if(!q.storage||!q.bytes||q.bytes>0x1000000||q.guest_base&15||q.guest_base>=0x80000000u||
       (uint64_t)q.guest_base+q.bytes>0x80000000u||q.bytes>UINTPTR_MAX-(uintptr_t)q.storage||
       !q.scratch||q.scratch_bytes!=sizeof(scratch)||sizeof(scratch)>UINTPTR_MAX-(uintptr_t)q.scratch||
       !q.vu_data||q.vu_data_bytes!=sizeof(vu)||sizeof(vu)>UINTPTR_MAX-(uintptr_t)q.vu_data||
       q.gp!=0x22597F||q.object_class>0xFFFF||q.payload_bytes>255*64u||
       overlap((uintptr_t)q.storage,q.bytes,(uintptr_t)q.scratch,sizeof(scratch))||
       overlap((uintptr_t)q.storage,q.bytes,(uintptr_t)q.vu_data,sizeof(vu))||
       overlap((uintptr_t)q.scratch,sizeof(scratch),(uintptr_t)q.vu_data,sizeof(vu))||
       overlap((uintptr_t)result,sizeof(*result),(uintptr_t)q.storage,q.bytes)||
       overlap((uintptr_t)result,sizeof(*result),(uintptr_t)q.scratch,sizeof(scratch))||
       overlap((uintptr_t)result,sizeof(*result),(uintptr_t)q.vu_data,sizeof(vu)))return -1;
    ram=malloc(q.bytes);if(!ram)return -4;
    memcpy(ram,q.storage,q.bytes);memcpy(scratch,q.scratch,sizeof(scratch));memcpy(vu,q.vu_data,sizeof(vu));
    globals=map(&q,ram,0x222D2C,8);clock=map(&q,ram,0x21DDB8,4);counter=map(&q,ram,0x222D10,4);
    if(!globals||!clock||!counter)goto finish;
    start=r32(globals);end=r32(globals+4);
    if((start&15)||(end&15)||end<start||(end-start)&255||(end-start)>1024*256u){status=-3;goto finish;}
    if(start!=end&&(overlap(start,end-start,0x222D10,0x48)||
       overlap(start,end-start,0x21DDB8,4)||overlap(start,end-start,0x21EC28,1))){status=-3;goto finish;}
    if(start!=end){pool=map(&q,ram,start,end-start);if(!pool)goto finish;}
    for(address=start;address<end;address+=0x100){
        actor=map(&q,ram,address,0x100);
        if(actor[0x20]<0xFE||r32(clock)<r32(actor+0xA0))continue;
        if(q.payload_bytes){
            DreadzonePayloadArena arena;DreadzonePayloadState state;
            p=map(&q,ram,0x222D3C,28);if(!p)goto finish;
            payload_base=r32(p);capacity=r32(p+4);descriptor_base=r32(p+8);descriptor_end=r32(p+24);
            if(capacity>4096||!capacity||descriptor_end<descriptor_base||
               (descriptor_end-descriptor_base)&3||(descriptor_end-descriptor_base)>2048*4u){status=-3;goto finish;}
            arena.descriptor_base=descriptor_base;arena.descriptor_count=(descriptor_end-descriptor_base)/4;
            arena.descriptors=(u32 *)map(&q,ram,descriptor_base,(descriptor_end-descriptor_base)+4);
            arena.payload_base=payload_base;arena.payload_bytes=capacity*64;
            arena.payload=map(&q,ram,payload_base,arena.payload_bytes);
            if(!arena.descriptors||!arena.payload)goto finish;
            if(overlap(descriptor_base,(uint64_t)arena.descriptor_count*4+4,payload_base,arena.payload_bytes)||
               overlap(descriptor_base,(uint64_t)arena.descriptor_count*4+4,start,end-start)||
               overlap(payload_base,arena.payload_bytes,start,end-start)||
               overlap(descriptor_base,(uint64_t)arena.descriptor_count*4+4,0x222D10,0x48)||
               overlap(payload_base,arena.payload_bytes,0x222D10,0x48)){status=-3;goto finish;}
            state.head=r32(p+12);state.tail=r32(p+16);state.cursor=r32(p+20);
            if(!dreadzone_payload_validate_arena(&arena,&state,q.payload_bytes)){status=-3;goto finish;}
            payload=dreadzone_payload_allocate(&arena,&state,q.payload_bytes);
            if(!payload)break;
            w32(p+12,state.head);w32(p+16,state.tail);w32(p+20,state.cursor);
        }
        if(actor[0x20]==0xFF){
            if(overlap(address+0x120,1,0x222D10,0x48)||overlap(address+0x120,1,0x21DDB8,4)||
               overlap(address+0x120,1,0x21EC28,1)){status=-3;goto finish;}
            p=map(&q,ram,address+0x120,1);if(!p)goto finish;*p=0xFF;
        }
        {
            DreadzoneFreshClass fresh={ram,q.guest_base,q.bytes,q.gp,address,q.object_class,1,scratch,sizeof(scratch),vu,sizeof(vu)};
            status=dreadzone_fresh_class_initialize(&fresh);if(status!=1)goto finish;
        }
        w32(actor+0xAC,payload);
        if(q.payload_bytes){p=map(&q,ram,payload,(q.payload_bytes+3)&~3u);if(!p){status=-2;goto finish;}
            for(i=0;i<q.payload_bytes;i+=4)w32(p+i,0);
        }
        i=r32(counter);if(i)w32(counter,i-1);
        memcpy(q.storage,ram,q.bytes);memcpy(q.scratch,scratch,sizeof(scratch));*result=address;
        status=1;goto finish;
    }
    p=map(&q,ram,0x21EC28,1);if(!p){status=-2;goto finish;}
    if(*p){status=-3;goto finish;}
    *result=0;status=0;
finish:
    free(ram);return status;
}
