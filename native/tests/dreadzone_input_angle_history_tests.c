#include "dreadzone_input_angle_history.h"
#include <stdio.h>
#include <string.h>

static unsigned checks;
#define CHECK(c) do {++checks;if(!(c)){fprintf(stderr,"history check failed at line %d\n",__LINE__);return 1;}} while(0)
static void put(u8 *p,u32 value) {
    p[0]=(u8)value;p[1]=(u8)(value>>8);p[2]=(u8)(value>>16);p[3]=(u8)(value>>24);
}
int main(void) {
    static u8 data[0x600],before[0x600];
    const s32 counts[]={-2147483647-1,-31,-1,0,1,7,30,2147483647};
    const s32 requested[]={-2147483647-1,-31,-1,0,1,7,29,30,31,2147483647};
    const s32 heads[]={-32768,-30,-1,0,1,29,30,32767};
    const u32 patterns[]={0,0x80000000U,1,0x80000001U,0x3F800000U,0xBF800000U,
                          0x7F800000U,0xFF800000U,0x7FC01234U,0x7F801234U};
    DreadzoneInputAngleHistoryRequest r={data,0x1EE600,sizeof(data),0,0};
    u32 h,c,q,p,i,result=0x12345678U,expected;
    s64 lookback,value,quotient,index;
    for(h=0;h<8;++h) for(c=0;c<8;++c) for(q=0;q<10;++q) for(p=0;p<2;++p) {
        for(i=0;i<sizeof(data);i+=4) put(data+i,patterns[(i/4+h+c+q)%10]);
        data[0x18E]=(u8)heads[h];data[0x18F]=(u8)(heads[h]>>8);put(data+0x190,(u32)counts[c]);
        lookback=counts[c]<requested[q]?counts[c]:requested[q];
        value=(s64)heads[h]-lookback+30;
        if(value>2147483647LL) value-=0x100000000LL;
        if(value<-2147483648LL) value+=0x100000000LL;
        quotient=(value<0?-((-value)/30):value/30);
        index=value-quotient*30;
        expected=(u32)data[0x258+index*4]|(u32)data[0x259+index*4]<<8|
                 (u32)data[0x25A+index*4]<<16|(u32)data[0x25B+index*4]<<24;
        r.requested=requested[q];r.input_guest=p?0x1EE600:0;
        memcpy(before,data,sizeof(data));
        CHECK(dreadzone_input_angle_history_005BD900(&r,&result)==0);
        CHECK(result==expected);CHECK(!memcmp(data,before,sizeof(data)));
    }
    r.input_guest=0x1EE601;result=0x12345678U;
    CHECK(dreadzone_input_angle_history_005BD900(&r,&result)==DREADZONE_INPUT_ANGLE_HISTORY_UNALIGNED);
    CHECK(result==0x12345678U);
    r.input_guest=0x200000;
    CHECK(dreadzone_input_angle_history_005BD900(&r,&result)==DREADZONE_INPUT_ANGLE_HISTORY_NEEDS_MEMORY);
    CHECK(result==0x12345678U);
    r.input_guest=0;r.bytes=0x190;
    CHECK(dreadzone_input_angle_history_005BD900(&r,&result)==DREADZONE_INPUT_ANGLE_HISTORY_NEEDS_MEMORY);
    CHECK(result==0x12345678U);
    r.bytes=sizeof(data);r.guest_base=0xFFFFFF00U;
    CHECK(dreadzone_input_angle_history_005BD900(&r,&result)==DREADZONE_INPUT_ANGLE_HISTORY_INVALID);
    CHECK(result==0x12345678U);
    CHECK(dreadzone_input_angle_history_005BD900(0,&result)==DREADZONE_INPUT_ANGLE_HISTORY_INVALID);
    r.guest_base=0x1EE600;r.requested=1;put(data+0x190,7);data[0x18E]=0;data[0x18F]=0;
    put(data+0x2CC,0x80000001U);
    /* Caller publication may alias the already-consumed count. */
    CHECK(dreadzone_input_angle_history_005BD900(&r,(u32 *)(void *)(data+0x190))==0);
    CHECK(data[0x190]==1 && data[0x193]==0x80);
    printf("Angle history: %u checks passed\n",checks);
    return 0;
}
