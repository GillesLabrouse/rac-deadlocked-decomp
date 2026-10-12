#include "dreadzone_actor_idle.h"
#include <fenv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { BASE=0x100000,SIZE=0x300000,RECORD=0x368F60 };
static u32 checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;printf("FAIL %d: %s\n",__LINE__,#x);}}while(0)
static void word(u8 *p,u32 address,u32 v){memcpy(p+address-BASE,&v,4);}
static u32 get(const u8 *p,u32 address){u32 v;memcpy(&v,p+address-BASE,4);return v;}
static void fixture(u8 *p,DreadzoneActorIdleRequest *r){
    memset(p,0,SIZE);memset(r,0,sizeof(*r));r->wall.storage=p;r->wall.guest_base=BASE;r->wall.storage_bytes=SIZE;
    r->wall.packet_guest=0x242340;r->wall.grid_guest=0x26FF20;r->wall.actor_pool_pointer_guest=0x222D14;
    r->wall.actor_pool_count=65536;r->wall.grid_block_count=3072;r->record_guest=RECORD;r->gp_guest=0x22597F;
    r->wall.incoming_vf25_27_w[0]=0x12345678;r->wall.incoming_vf25_27_w[1]=0x87654321;r->wall.incoming_vf25_27_w[2]=0xDEADBEEF;
    r->incoming_vf28_29_w[0]=0x3F000000;r->incoming_vf28_29_w[1]=0x80000000;
    p[0x171DE0-BASE]=1;word(p,0x21DDF4,0x3F800000);word(p,0x21DDF8,0x3C800000);
    word(p,RECORD+0x25C4,0x3F800000);word(p,RECORD+0x1784,0x3F800000);
    word(p,RECORD+0x1970,0xC1874BBF);word(p,RECORD+0x1974,0x7FC12345);
    word(p,RECORD+0x1978,0x80000000);word(p,RECORD+0x197C,0xDEADBEEF);word(p,RECORD+0x19B0,0xFFC23456);
}
static void refusal(u8 *p,DreadzoneActorIdleRequest *r){
    u8 *before=(u8 *)malloc(SIZE);DreadzoneActorIdleInfo info,old;int status;
    if(!before){CHECK(0);return;}memcpy(before,p,SIZE);memset(&info,0x6c,sizeof(info));old=info;
    status=dreadzone_actor_idle_00695570(r,&info);CHECK(status<0);CHECK(!memcmp(p,before,SIZE));CHECK(!memcmp(&info,&old,sizeof(info)));free(before);
}
int main(void){
    u8 *allocation=(u8 *)malloc(SIZE+16),*p;DreadzoneActorIdleRequest r,*alias;DreadzoneActorIdleInfo info;
    DreadzoneMotionProbeRequest probe;DreadzoneMotionProbeInfo pi,old;u32 i;
    if(!allocation)return 1;p=allocation+1;fixture(p,&r);
    CHECK(dreadzone_actor_idle_00695570(&r,&info)==1);
    CHECK(!memcmp(p+RECORD+0x1790-BASE,p+RECORD+0x1970-BASE,16));CHECK(get(p,RECORD+0x17A0)==0xFFC23456);
    CHECK(get(p,RECORD+0x1780)==0&&get(p,RECORD+0x1784)==0x3F393333);CHECK(get(p,RECORD+0x138)==0xBF580000);
    CHECK(info.motion.length_helper_calls==1&&info.speed_calls==1&&info.accumulation_calls==1&&info.height_calls==1);
    CHECK(info.wall.probe_calls==0&&info.wall.outgoing_vf25_29_w[0]==0x12345678&&info.wall.outgoing_vf25_29_w[3]==0x3F000000&&info.wall.outgoing_vf25_29_w[4]==0x80000000);
    /* Capture descriptors inside backing before publishing its shadow. */
    fixture(p,&r);alias=(DreadzoneActorIdleRequest *)(void *)(p+0x1000);memcpy(alias,&r,sizeof(r));
    CHECK(dreadzone_actor_idle_00695570(alias,&info)==1);CHECK(!memcmp(alias,&r,sizeof(r)));
    fixture(p,&r);r.wall.storage_bytes=0x21DDF8+4-BASE;refusal(p,&r);
    fixture(p,&r);word(p,RECORD+0x25CC,3);refusal(p,&r);
    fixture(p,&r);word(p,RECORD+0x1A0,0x3F800000);refusal(p,&r);
    fixture(p,&r);p[RECORD+0x437-BASE]=1;refusal(p,&r);
    fixture(p,&r);word(p,0x21DDF8,0x40000000);refusal(p,&r); /* late height refusal rolls back all earlier stores */
    fixture(p,&r);word(p,0x21DDF4,0x7FC12345);refusal(p,&r); /* nested wall refusal */
    fixture(p,&r);CHECK(fesetround(FE_UPWARD)==0);refusal(p,&r);CHECK(fesetround(FE_TONEAREST)==0);
    fixture(p,&r);CHECK(dreadzone_actor_idle_00695570(&r,(DreadzoneActorIdleInfo *)(void *)(p+0x1000))<0);
    fixture(p,&r);probe=(DreadzoneMotionProbeRequest){p,BASE,SIZE,RECORD};
    word(p,RECORD+0x130,0x80000000);word(p,RECORD+0x134,0x80000000);word(p,RECORD+0x138,0x2B800000);word(p,RECORD+0x13C,0xC1800000);
    CHECK(dreadzone_actor_motion_probe_006840B8(&probe,&pi)==1&&pi.return_low==0&&pi.length_result_bits==0);
    for(i=0;i<4;++i){u32 rejected[4]={0x00800000,0x2B7FFFFF,0x7F800000,0x7FC12345};
        word(p,RECORD+0x138,rejected[i]);memset(&pi,0x6c,sizeof(pi));old=pi;
        CHECK(dreadzone_actor_motion_probe_006840B8(&probe,&pi)<0);CHECK(!memcmp(&pi,&old,sizeof(pi)));}
    printf("%u checks, %u failures\n",checks,failures);free(allocation);return failures?1:0;
}
