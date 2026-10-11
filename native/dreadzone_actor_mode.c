#include "dreadzone_actor_mode.h"
#include <string.h>

typedef struct ModeRead { DreadzoneActorModeRequest request; int invalid; } ModeRead;
static int span(const void *p,uintptr_t n) { return p && (uintptr_t)p<=UINTPTR_MAX-n; }
static int overlap(const void *a,uintptr_t n,const void *b,uintptr_t m) {
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x>=y?x-y<m:y-x<n;
}
static u32 read_word(ModeRead *r,u32 address,u32 n) {
    u32 value=0;DreadzoneActorModeRequest *q=&r->request;
    if(r->invalid)return 0;
    if((address&(n-1U)) || address<q->guest_base || n>q->storage_bytes ||
        address-q->guest_base>q->storage_bytes-n) { r->invalid=1;return 0; }
    memcpy(&value,q->storage+address-q->guest_base,n);return value;
}
static s16 read_half(ModeRead *r,u32 address) {
    u16 bits=(u16)read_word(r,address,2);s16 value;memcpy(&value,&bits,2);return value;
}
int dreadzone_actor_mode_0068A090(const DreadzoneActorModeRequest *request,DreadzoneActorModeInfo *info) {
    ModeRead r;DreadzoneActorModeInfo output={0};DreadzoneActorModeRequest *q=&r.request;
    u32 b,force,override,state;int selected=0;
    if(!span(request,sizeof(*request))||!span(info,sizeof(*info)))return -1;
    memcpy(q,request,sizeof(*request));r.invalid=0;b=q->record_guest;
    if(!span(q->storage,q->storage_bytes)||q->storage_bytes>16U*1024U*1024U||
        q->storage_bytes<0x31F0U||(b&15U)||b<q->guest_base||
        b-q->guest_base>q->storage_bytes-0x31F0U||
        (u64)q->guest_base+q->storage_bytes>0x80000000ULL||
        overlap(info,sizeof(*info),request,sizeof(*request))||
        overlap(info,sizeof(*info),q->storage,q->storage_bytes))return -1;
    force=read_word(&r,0x21E878,4);
    output.mode=read_half(&r,b+0x2F8);
    override=read_word(&r,0x21E87C,4);
    if(force && !override)output.mode=2;
    if(override)selected=1;
    else {
        state=read_word(&r,b+0x25CC,4);
        if(state==59 || (state==112 && read_word(&r,b+0x438,1)==0) || state==111)selected=1;
        else if(read_half(&r,b+0x3D6)!=0)selected=1;
        else if(read_word(&r,b+0x437,1)!=0) {
            u32 metadata=read_word(&r,b+0x26E4,4),flags=0;
            if(metadata>0xFFFFFFFFU-0xEF8U)r.invalid=1;
            else flags=read_word(&r,metadata+0xEF8U,1);
            ++output.metadata_queries;
            if(((flags^0x80U)>>7)!=0 && read_word(&r,b+0x25CC,4)==0 &&
                read_half(&r,b+0x2FE)<4)selected=1;
        }
    }
    if(selected)output.mode=1;
    if(output.mode==1 && read_word(&r,b+0x438,1)!=0 && read_word(&r,b+0x437,1)==0)output.mode=0;
    if(r.invalid)return -1;
    memcpy(info,&output,sizeof(output));return 1;
}
