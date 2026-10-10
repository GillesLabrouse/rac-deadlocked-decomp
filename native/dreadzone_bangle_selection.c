#include "dreadzone_bangle_selection.h"
#include <stdint.h>
#include <string.h>

static int valid_span(const void *p,size_t bytes)
{
    return p!=NULL && bytes!=0 && (uintptr_t)p<=UINTPTR_MAX-bytes;
}
static int overlaps(const void *a,size_t an,const void *b,size_t bn)
{
    return (uintptr_t)a<(uintptr_t)b+bn && (uintptr_t)b<(uintptr_t)a+an;
}
static u32 read32(const u8 *p)
{
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}
static int append(DreadzoneBanglePlan *p,u32 first,u32 count,s32 bit,u32 total)
{
    if(first>total || count>total-first) return 0;
    if(count!=0) {
        DreadzoneBangleRange *r=&p->ranges[p->range_count++];
        r->first_packet=first;r->packet_count=count;r->bangle=bit;
        p->packet_count+=count;
    }
    return 1;
}
int dreadzone_bangle_select(const DreadzoneBangleRequest *request,DreadzoneBanglePlan *output)
{
    DreadzoneBangleRequest q;
    DreadzoneBanglePlan plan;
    u32 offset,total,table,first,count,mask,bit,low;
    const u8 *raw;
    if(!valid_span(request,sizeof(*request)) || !valid_span(output,sizeof(*output)))
        return DREADZONE_BANGLE_INVALID;
    memcpy(&q,request,sizeof(q));
    raw=q.raw_class;
    if(q.class_bytes<0x4C || q.class_bytes>0x1000000U
        || !valid_span(raw,q.class_bytes)
        || overlaps(request,sizeof(*request),output,sizeof(*output))
        || overlaps(raw,q.class_bytes,output,sizeof(*output))) return DREADZONE_BANGLE_INVALID;
    if(q.reserved!=0 || q.low_detail>1 || (q.actor_mask&0x8000U)!=0
        || raw[6]!=0 || raw[0xA]!=0) return DREADZONE_BANGLE_UNSUPPORTED;
    memset(&plan,0,sizeof(plan));
    offset=(u32)raw[0x2C]*16U;
    total=(u32)raw[4]+raw[5];
    if(offset!=0) {
        if(offset>q.class_bytes-4U) return DREADZONE_BANGLE_INVALID;
        total+=raw[offset+1];
    }
    table=read32(raw);
    if(table%16U!=0 || table>q.class_bytes || total>(q.class_bytes-table)/16U)
        return DREADZONE_BANGLE_INVALID;
    low=(q.low_detail!=0 && raw[5]!=0);
    first=0;count=raw[4];
    if(low!=0) { first=raw[4];count=raw[5]; }
    if(!append(&plan,first,count,-1,total)) return DREADZONE_BANGLE_INVALID;
    mask=q.actor_mask;
    while(mask!=0) {
        u32 descriptor;
        bit=14;
        while((mask&(1U<<bit))==0) bit--;
        mask^=1U<<bit;
        if(offset==0) continue; /* Retail consumes the bit, then bypasses. */
        descriptor=offset+4U+bit*4U;
        if(descriptor>q.class_bytes-4U) return DREADZONE_BANGLE_INVALID;
        first=raw[descriptor];count=raw[descriptor+1];
        if(low!=0 && raw[descriptor+2]!=0) {
            first=raw[descriptor+2];count=raw[descriptor+3];
        }
        if(!append(&plan,first,count,(s32)bit,total)) return DREADZONE_BANGLE_INVALID;
    }
    memcpy(output,&plan,sizeof(plan));
    return DREADZONE_BANGLE_CANDIDATES;
}
