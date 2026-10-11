#include "dreadzone_input_angle_history.h"
#include <string.h>

static int host_span(const void *p,uintptr_t bytes) {
    return p && (uintptr_t)p<=UINTPTR_MAX-bytes;
}
static const u8 *read_bytes(const DreadzoneInputAngleHistoryRequest *r,u32 address,u32 bytes) {
    if(address>0xFFFFFFFFU-(bytes-1) || address<r->guest_base || bytes>r->bytes ||
       address-r->guest_base>r->bytes-bytes) return 0;
    return r->storage+(address-r->guest_base);
}
static u32 word(const u8 *p) {
    return (u32)p[0]|(u32)p[1]<<8|(u32)p[2]<<16|(u32)p[3]<<24;
}
int dreadzone_input_angle_history_005BD900(const DreadzoneInputAngleHistoryRequest *request,u32 *out) {
    DreadzoneInputAngleHistoryRequest r;
    const u8 *head_bytes,*count_bytes,*value_bytes;
    u32 input,head_raw,count_raw,wrapped,address,result;
    s64 head,count,lookback,dividend,index;
    if(!host_span(request,sizeof(*request)) || !host_span(out,sizeof(*out)))
        return DREADZONE_INPUT_ANGLE_HISTORY_INVALID;
    memcpy(&r,request,sizeof(r));
    if(!r.bytes || !host_span(r.storage,r.bytes) || r.guest_base>0xFFFFFFFFU-(r.bytes-1))
        return DREADZONE_INPUT_ANGLE_HISTORY_INVALID;
    input=r.input_guest?r.input_guest:0x001EE600U;
    if(input&3U) return DREADZONE_INPUT_ANGLE_HISTORY_UNALIGNED;
    /* LW count precedes LH head in the original. Neither read publishes. */
    count_bytes=read_bytes(&r,input+0x190U,4);
    if(!count_bytes) return DREADZONE_INPUT_ANGLE_HISTORY_NEEDS_MEMORY;
    count_raw=word(count_bytes);
    count=(count_raw&0x80000000U)?(s64)count_raw-0x100000000LL:(s64)count_raw;
    head_bytes=read_bytes(&r,input+0x18EU,2);
    if(!head_bytes) return DREADZONE_INPUT_ANGLE_HISTORY_NEEDS_MEMORY;
    head_raw=(u32)head_bytes[0]|(u32)head_bytes[1]<<8;
    head=(head_raw&0x8000U)?(s64)head_raw-0x10000LL:(s64)head_raw;
    lookback=count<(s64)r.requested?count:(s64)r.requested;
    wrapped=(u32)(head-lookback+30);
    dividend=(wrapped&0x80000000U)?(s64)wrapped-0x100000000LL:(s64)wrapped;
    index=dividend%30;
    address=input+0x258U+(u32)(index*4);
    value_bytes=read_bytes(&r,address,4);
    if(!value_bytes) return DREADZONE_INPUT_ANGLE_HISTORY_NEEDS_MEMORY;
    result=word(value_bytes);
    memcpy(out,&result,sizeof(result));
    return DREADZONE_INPUT_ANGLE_HISTORY_COMPLETED;
}
