#include "dreadzone_actor_walking.h"
#include "dreadzone_actor_input_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static u32 checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;printf("FAIL line%d: %s\n",__LINE__,#x);}}while(0)
enum {BASE=0x171000,RECORD=0x368F60,SIZE=0x400000-BASE+160};
static void word(u8 *p,u32 address,u32 value){memcpy(p+address-BASE,&value,4);}
static void scalar(u8 *p,u32 address,f32 value){memcpy(p+address-BASE,&value,4);}
static void quad(u8 *p,u32 address,f32 x,f32 y,f32 z,f32 w){f32 v[4]={x,y,z,w};memcpy(p+address-BASE,v,16);}
static u32 get(const u8 *p,u32 address){u32 value;memcpy(&value,p+address-BASE,4);return value;}
static void setup(u8 *p,DreadzoneActorWalkingRequest *r,f32 x,f32 y){
    u32 i;DreadzoneActorInputMemory views[2];DreadzoneActorInputContext context;
    memset(p,0,SIZE);memset(r,0,sizeof(*r));
    r->wall.storage=p;r->wall.guest_base=BASE;r->wall.storage_bytes=SIZE;
    r->wall.packet_guest=0x242340;r->wall.grid_guest=0x26FF20;
    r->wall.actor_pool_pointer_guest=0x222D14;r->wall.actor_pool_count=65536;r->wall.grid_block_count=3072;
    r->record_guest=RECORD;r->incoming_vf28_29_w[0]=0x3F000000;r->incoming_vf28_29_w[1]=0xBF000000;
    r->wall.incoming_vf25_27_w[0]=0x3F800000;r->wall.incoming_vf25_27_w[1]=0x80000000;
    r->wall.incoming_vf25_27_w[2]=0xBF800000;
    word(p,RECORD+0x2EF0,0x230000);word(p,RECORD+0x2F00,0x1EE600);
    word(p,RECORD+0x25CC,2);word(p,RECORD+0x2314,2);word(p,RECORD+0x2318,2);
    word(p,RECORD+0x3B4,3);p[RECORD+0x266D-BASE]=1;
    scalar(p,RECORD+0x25C4,1.f);scalar(p,RECORD+0x1B4,10.f);scalar(p,RECORD+0x2FC0,1.f);
    for(i=0;i<4;++i){f32 v[4]={0,0,0,0};v[i]=1.f;memcpy(p+RECORD+0x20+16*i-BASE,v,16);}
    for(i=0;i<3;++i)scalar(p,RECORD+0x2CF0+20*i,1.f);
    scalar(p,0x2300C0,1.f);scalar(p,0x2300D4,1.f);scalar(p,0x2300E8,1.f);
    word(p,0x1EE790,7);scalar(p,0x1EE708,x);scalar(p,0x1EE70C,y);
    scalar(p,0x21DDEC,.5f);scalar(p,0x21DDF4,1.f/60.f);scalar(p,0x21DDF8,1.f/60.f);
    scalar(p,0x3B01A0,7.25f);
    word(p,0x242340,0x400000);word(p,0x222D14,0x2A0000);
    quad(p,RECORD+0xA0,10,10,10,1.f);
    views[0]=(DreadzoneActorInputMemory){p,BASE,RECORD-BASE,1};
    views[1]=(DreadzoneActorInputMemory){p+RECORD-BASE+0x31F0,RECORD+0x31F0,SIZE-(RECORD-BASE)-0x31F0,1};
    context=(DreadzoneActorInputContext){p+RECORD-BASE,RECORD,0x31F0,views,2};
    CHECK(dreadzone_actor_input_006A7D48(&context)==1);
}
static void refusal(u8 *p,DreadzoneActorWalkingRequest *r,int expected){
    DreadzoneActorWalkingInfo info,before_info;u8 *before=(u8 *)malloc(SIZE);
    if(!before){CHECK(0);return;}memcpy(before,p,SIZE);memset(&info,0x6C,sizeof(info));before_info=info;
    CHECK(dreadzone_actor_walking_00695CB8(r,&info)==expected);CHECK(!memcmp(p,before,SIZE));
    CHECK(!memcmp(&info,&before_info,sizeof(info)));free(before);
}
int main(void){
    u8 *allocation=(u8 *)malloc(SIZE+2),*p;DreadzoneActorWalkingRequest r;DreadzoneActorWalkingInfo info;u32 i;
    if(!allocation)return 1;p=allocation+1;allocation[0]=0xCD;allocation[SIZE+1]=0xDE;
    setup(p,&r,0,0);CHECK(dreadzone_actor_walking_00695CB8(&r,&info)==1);
    CHECK(info.parameter_calls==1&&info.speed_calls==1&&info.velocity_calls==1&&info.height_calls==1);
    CHECK(info.angle_history_calls==2&&!info.direction_calls&&!info.wall.probe_calls);
    CHECK(info.wall.outgoing_vf25_29_w[3]==0&&info.wall.outgoing_vf25_29_w[4]==0xBF000000);
    CHECK(get(p,RECORD+0xA0)==0x41200000&&get(p,RECORD+0xAC)==0x3F800000);
    setup(p,&r,.5f,-.5f);CHECK(dreadzone_actor_walking_00695CB8(&r,&info)==1);
    CHECK(info.direction_calls==1&&info.turn_calls==1&&info.wall.probe_calls==2);
    CHECK(info.wall.outgoing_vf25_29_w[3]==0&&info.wall.outgoing_vf25_29_w[4]==0);
    CHECK(get(p,0x242350)==2);CHECK(get(p,RECORD+0x1784)!=0);
    setup(p,&r,.5f,-.5f);scalar(p,RECORD+0x1E70,.5f);quad(p,RECORD+0x1E60,.01f,-.02f,0,.5f);
    CHECK(dreadzone_actor_walking_00695CB8(&r,&info)==1&&info.heading_calls==1);
    CHECK(!get(p,RECORD+0x1E60)&&!get(p,RECORD+0x1E6C));
    setup(p,&r,.5f,-.5f);p[RECORD+0x2FE-BASE]=1;scalar(p,RECORD+0x2CC,.2f);
    CHECK(dreadzone_actor_walking_00695CB8(&r,&info)==1&&info.slope_calls==1);
    for(i=0;i<3;++i){setup(p,&r,.5f,-.5f);if(i==0)word(p,RECORD+0x25CC,1);
        if(i==1)p[RECORD+0x2659-BASE]=1;if(i==2)p[RECORD+0x2661-BASE]=1;refusal(p,&r,-2);}
    setup(p,&r,.5f,-.5f);p[RECORD+0x434-BASE]=1;scalar(p,RECORD+0x2E14,.1f);refusal(p,&r,-2);
    setup(p,&r,.5f,-.5f);p[RECORD+0x5F4-BASE]=1;refusal(p,&r,-2);
    setup(p,&r,.5f,-.5f);p[RECORD+0x3D0-BASE]=1;refusal(p,&r,-2); /* Late after real wall update. */
    setup(p,&r,.5f,-.5f);p[RECORD+0x2FE-BASE]=1;p[RECORD+0x226-BASE]=1;
    quad(p,RECORD+0x1D0,1,0,1,0);scalar(p,RECORD+0x158,1.f);refusal(p,&r,-2); /* Late tables. */
    setup(p,&r,.5f,-.5f);word(p,RECORD+0x25C4,1);refusal(p,&r,-3);
    setup(p,&r,.5f,-.5f);word(p,0x21DDF4,0x7FC00000);refusal(p,&r,-3);
    setup(p,&r,.5f,-.5f);word(p,RECORD+0x2F00,0x7FFFFFF0);refusal(p,&r,-2);
    setup(p,&r,.5f,-.5f);r.record_guest++;refusal(p,&r,-1);
    setup(p,&r,.5f,-.5f);r.wall.storage_bytes=17U*1024U*1024U;refusal(p,&r,-1);
    setup(p,&r,.5f,-.5f);CHECK(dreadzone_actor_walking_00695CB8(&r,(DreadzoneActorWalkingInfo *)(p+100))==-1);
    setup(p,&r,.5f,-.5f);CHECK(dreadzone_actor_walking_00695CB8(NULL,&info)==-1);CHECK(dreadzone_actor_walking_00695CB8(&r,NULL)==-1);
    CHECK(allocation[0]==0xCD&&allocation[SIZE+1]==0xDE);
    free(allocation);printf("%u walking checks, %u failures\n",checks,failures);return failures?1:0;
}
