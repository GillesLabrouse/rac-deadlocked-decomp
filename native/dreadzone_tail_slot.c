#include "dreadzone_tail_slot.h"
#include <string.h>

static int span(const void *p,uintptr_t n){return p&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *p,uintptr_t n,const void *q,uintptr_t m){
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;
    return a>=b?a-b<m:b-a<n;
}
static u8 *word_at(const DreadzoneTailSlotRequest *r,u32 address){
    if((address&3U)||address<r->guest_base||r->storage_bytes<4U||
       address-r->guest_base>r->storage_bytes-4U)return NULL;
    return r->storage+(address-r->guest_base);
}
static u32 load_le32(const u8 *p){
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}
int dreadzone_tail_slot_00535A30(const DreadzoneTailSlotRequest *request,DreadzoneTailSlotInfo *info){
    DreadzoneTailSlotRequest r;
    DreadzoneTailSlotInfo out={0};
    u8 *p;
    if(!span(request,sizeof(*request))||!span(info,sizeof(*info))||
       overlap(request,sizeof(*request),info,sizeof(*info)))return -1;
    memcpy(&r,request,sizeof(r));
    if(!r.storage_bytes||r.storage_bytes>16U*1024U*1024U||
       !span(r.storage,r.storage_bytes)||r.guest_base>=0x10000000U||
       r.storage_bytes>0x10000000U-r.guest_base||r.index>255U||
       overlap(info,sizeof(*info),r.storage,r.storage_bytes))return -1;
    out.slot_guest=0x230280U+r.index*0x470U;
    out.entry_guest=out.slot_guest+0x124U;
    out.read_guest=out.entry_guest;
    out.read_count=1;
    p=word_at(&r,out.entry_guest);
    if(!p)return -2;
    out.entry_pointer=load_le32(p);
    if(out.entry_pointer){
        out.clear_guest=out.entry_pointer+0x98U;
        p=word_at(&r,out.clear_guest);
        if(!p)return -2;
        out.return_low=out.entry_pointer;
        out.did_clear=1;
        /* The original branch-likely delay store is reached only for P != 0. */
        p[0]=p[1]=p[2]=p[3]=0;
    }
    memcpy(info,&out,sizeof(out));
    return 1;
}
