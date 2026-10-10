#include "dreadzone_bangle_selection.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static u32 checks,failures;
static void check(int ok,const char *name) { checks++;if(!ok){failures++;printf("FAIL %s\n",name);} }
int main(void)
{
    u8 raw[0x1000];
    DreadzoneBangleRequest q;
    DreadzoneBanglePlan p,before;
    u32 mask,i;
    memset(raw,0,sizeof(raw));raw[0]=0;raw[1]=4;raw[4]=37;raw[5]=20;raw[0x2C]=0x23;
    raw[0x231]=54;
    for(i=0;i<15;i++) {raw[0x234+i*4]=(u8)(57+i);raw[0x235+i*4]=1;}
    raw[0x234+12*4]=101;raw[0x235+12*4]=7;raw[0x236+12*4]=108;raw[0x237+12*4]=3;
    raw[0x234+1*4]=59;raw[0x235+1*4]=2;raw[0x236+1*4]=61;raw[0x237+1*4]=2;
    q.raw_class=raw;q.class_bytes=sizeof(raw);q.actor_mask=0x1002;q.low_detail=0;q.reserved=0;
    check(dreadzone_bangle_select(&q,&p)==1,"high real mask");
    check(p.range_count==3 && p.packet_count==46,"high totals");
    check(p.ranges[0].first_packet==0 && p.ranges[0].packet_count==37 && p.ranges[0].bangle==-1,"primary high");
    check(p.ranges[1].first_packet==101 && p.ranges[1].packet_count==7 && p.ranges[1].bangle==12,"descending bit12");
    check(p.ranges[2].first_packet==59 && p.ranges[2].packet_count==2 && p.ranges[2].bangle==1,"descending bit1");
    q.low_detail=1;check(dreadzone_bangle_select(&q,&p)==1,"low selection");
    check(p.ranges[0].first_packet==37 && p.ranges[0].packet_count==20 && p.ranges[1].first_packet==108 && p.ranges[2].first_packet==61 && p.packet_count==25,"low ranges");
    for(mask=0;mask<0x8000;mask++) {
        u32 expected=0,previous=15;
        q.actor_mask=(u16)mask;
        check(dreadzone_bangle_select(&q,&p)==1,"all positive masks");
        for(i=0;i<15;i++) expected+=(mask>>i)&1U;
        check(p.range_count==expected+1,"mask range count");
        for(i=1;i<p.range_count;i++) {
            check((u32)p.ranges[i].bangle<previous && (mask&(1U<<p.ranges[i].bangle))!=0,"descending exact bit");
            previous=(u32)p.ranges[i].bangle;
        }
    }
    q.actor_mask=1;q.low_detail=1;check(dreadzone_bangle_select(&q,&p)==1 && p.ranges[1].first_packet==57,"low fallback");
    raw[5]=0;q.actor_mask=2;check(dreadzone_bangle_select(&q,&p)==1 && p.ranges[0].first_packet==0 && p.ranges[0].packet_count==37 && p.ranges[1].first_packet==59,"primary low fallback forces high bangles");raw[5]=20;
    q.actor_mask=0x8000;memset(&p,0xA5,sizeof(p));before=p;
    check(dreadzone_bangle_select(&q,&p)==-2 && memcmp(&p,&before,sizeof(p))==0,"negative signed mask atomic");
    q.actor_mask=1;raw[0x234]=111;raw[0x235]=1;
    check(dreadzone_bangle_select(&q,&p)==-1 && memcmp(&p,&before,sizeof(p))==0,"packet boundary refusal atomic");
    raw[0x234]=57;raw[0x235]=1;q.class_bytes=0x230;
    check(dreadzone_bangle_select(&q,&p)==-1,"truncated descriptor");q.class_bytes=sizeof(raw);
    check(dreadzone_bangle_select(&q,(DreadzoneBanglePlan *)(raw+0x800))==-1,"container output overlap");
    check(dreadzone_bangle_select(&q,(DreadzoneBanglePlan *)&q)==-1,"request output overlap");
    check(dreadzone_bangle_select((const DreadzoneBangleRequest *)(UINTPTR_MAX-3),&p)==-1,"wrapping request");
    q.raw_class=(const u8 *)(UINTPTR_MAX-3);check(dreadzone_bangle_select(&q,&p)==-1,"wrapping container");q.raw_class=raw;
    raw[6]=1;check(dreadzone_bangle_select(&q,&p)==-2,"metal unsupported");raw[6]=0;
    raw[0x2C]=0;check(dreadzone_bangle_select(&q,&p)==1 && p.range_count==1,"no bangle table");
    printf("%u checks, %u failures\n",checks,failures);return failures!=0;
}
