#include "dreadzone_slot_word.h"
#include <string.h>
static int span(const void *p,uintptr_t n){return p&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *p,uintptr_t n,const void *q,uintptr_t m){
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;return a>=b?a-b<m:b-a<n;
}
static u8 *at(const DreadzoneSlotWordRequest *r,u32 address){
    if((address&3U)||address<r->guest_base||r->storage_bytes<4U||
       address-r->guest_base>r->storage_bytes-4U)return NULL;
    return r->storage+address-r->guest_base;
}
static u32 load(const u8 *p){return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);}
int dreadzone_slot_word_00535A58(const DreadzoneSlotWordRequest *request,DreadzoneSlotWordInfo *info){
    DreadzoneSlotWordRequest r;DreadzoneSlotWordInfo out={0};u8 *p,*destination;u32 value;
    if(!span(request,sizeof(*request))||!span(info,sizeof(*info))||overlap(request,sizeof(*request),info,sizeof(*info)))return -1;
    memcpy(&r,request,sizeof(r));
    if(!r.storage_bytes||r.storage_bytes>16U*1024U*1024U||!span(r.storage,r.storage_bytes)||
       r.guest_base>=0x10000000U||r.storage_bytes>0x10000000U-r.guest_base||r.index>255U||
       overlap(info,sizeof(*info),r.storage,r.storage_bytes))return -1;
    out.slot_guest=0x230280U+r.index*0x470U;out.entry_guest=out.slot_guest+0x124U;
    out.read_guest[out.read_count++]=out.entry_guest;p=at(&r,out.entry_guest);if(!p)return -2;
    out.entry_pointer=load(p);out.return_low=0x230280U;
    if(out.entry_pointer){
        out.child_pointer_guest=out.entry_pointer+0x24U;
        out.read_guest[out.read_count++]=out.child_pointer_guest;p=at(&r,out.child_pointer_guest);if(!p)return -2;
        out.child_pointer=load(p);out.value_guest=out.child_pointer+0x10U;
        out.read_guest[out.read_count++]=out.value_guest;p=at(&r,out.value_guest);if(!p)return -2;
        value=load(p);out.store_guest=out.entry_pointer+0x98U;destination=at(&r,out.store_guest);if(!destination)return -2;
        out.return_low=out.child_pointer;out.did_store=1;
        destination[0]=(u8)value;destination[1]=(u8)(value>>8);destination[2]=(u8)(value>>16);destination[3]=(u8)(value>>24);
    }
    memcpy(info,&out,sizeof(out));return 1;
}
