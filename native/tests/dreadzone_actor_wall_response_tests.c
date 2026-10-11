#include "dreadzone_actor_wall_response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static u32 checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;printf("FAIL line%d: %s\n",__LINE__,#x);}}while(0)
enum { BASE=0x21D000,RECORD=0x2B0000,SIZE=0x400000-BASE+160 };
static void word(u8 *p,u32 address,u32 value){memcpy(p+address-BASE,&value,4);}
static u32 get(const u8 *p,u32 address){u32 value;memcpy(&value,p+address-BASE,4);return value;}
static void scalar(u8 *p,u32 address,f32 value){memcpy(p+address-BASE,&value,4);}
static void quad(u8 *p,u32 address,f32 x,f32 y,f32 z,f32 w){f32 v[4]={x,y,z,w};memcpy(p+address-BASE,v,16);}
static u32 vertex(s32 x,s32 y,s32 z){return ((u32)x&1023U)|(((u32)y&1023U)<<10)|(((u32)z&4095U)<<20);}
static void half(u8 *p,u32 address,u16 value){memcpy(p+address-BASE,&value,2);}
static void fixture(u8 *p,DreadzoneActorWallResponseRequest *r) {
    u32 i;memset(p,0xA5,SIZE);memset(r,0,sizeof(*r));
    r->wall.storage=p;r->wall.guest_base=BASE;r->wall.storage_bytes=SIZE;
    r->wall.packet_guest=0x242340;r->wall.grid_guest=0x26FF20;r->wall.actor_pool_pointer_guest=0x222D14;
    r->wall.actor_pool_count=65536;r->wall.grid_block_count=3072;r->record_guest=RECORD;
    for(i=0;i<3;++i)r->wall.incoming_vf25_27_w[i]=0;
    r->incoming_vf28_29_w[0]=0x12345678;r->incoming_vf28_29_w[1]=0x87654321;
    memset(p+RECORD-BASE,0,0x31F0);memset(p+0x26FF20-BASE,0,0x1C000);
    word(p,0x242340,0x400000);word(p,0x242350,0);word(p,0x222D14,0x2A0000);
    scalar(p,0x21DDF4,1.f);word(p,0x21DDB8,0);p[0x220648-BASE]=0;
    scalar(p,RECORD+0x25C4,.01f);word(p,RECORD+0x25D4,2);word(p,RECORD+0x2EF0,0x2A0100);
    for(i=0;i<4;++i){f32 v[4]={0,0,0,0};v[i]=1.f;memcpy(p+RECORD+0x20+16*i-BASE,v,16);}
    quad(p,RECORD+0xA0,10,10,10,0);quad(p,RECORD+0x130,.05f,0,0,0);
    memset(p+0x400000-BASE,0,160);
    half(p,0x400000,2);half(p,0x400002,1);half(p,0x400004,8);
    half(p,0x400020,2);half(p,0x400022,1);word(p,0x400024,64);
    half(p,0x400040,2);half(p,0x400042,1);word(p,0x400044,0x8002);
    half(p,0x400080,1);p[0x400082-BASE]=3;
    word(p,0x400084,vertex(-16,-16,-16));word(p,0x400088,vertex(16,-16,-16));word(p,0x40008C,vertex(0,16,-16));
    p[0x400090-BASE]=0;p[0x400091-BASE]=1;p[0x400092-BASE]=2;p[0x400093-BASE]=12;
}
static void refusal(u8 *p,DreadzoneActorWallResponseRequest *r,int expected) {
    DreadzoneActorWallResponseInfo info,old;u8 *before=(u8 *)malloc(SIZE);
    if(!before){CHECK(0);return;}memcpy(before,p,SIZE);memset(&info,0x6C,sizeof(info));old=info;
    CHECK(dreadzone_actor_wall_response_0067D6E8(r,&info)==expected);
    CHECK(!memcmp(p,before,SIZE));CHECK(!memcmp(&info,&old,sizeof(info)));free(before);
}
int main(void) {
    u8 *allocation=(u8 *)malloc(SIZE+2),*p;DreadzoneActorWallResponseRequest r;
    DreadzoneActorWallResponseInfo info;u32 i;
    if(!allocation)return 1;p=allocation+1;
    fixture(p,&r);quad(p,RECORD+0x130,0,0,0,0);
    CHECK(dreadzone_actor_wall_response_0067D6E8(&r,&info)==1);
    CHECK(info.probe_calls==0&&get(p,0x242350)==0);
    CHECK(info.outgoing_vf25_29_w[3]==0x12345678&&info.outgoing_vf25_29_w[4]==0x87654321);
    fixture(p,&r);CHECK(dreadzone_actor_wall_response_0067D6E8(&r,&info)==1);
    CHECK(info.probe_calls==2&&get(p,0x242350)==2&&!info.velocity_changed);
    CHECK(!info.outgoing_vf25_29_w[3]&&!info.outgoing_vf25_29_w[4]);
    /* Upward basis ray hits surface12 first; second misses. The real retained
     * surface still selects zero projected normal and Q division-by-zero. */
    fixture(p,&r);r.force=1;quad(p,RECORD+0x20,0,0,1,0);quad(p,RECORD+0xA0,10,10,9.6f,0);
    CHECK(dreadzone_actor_wall_response_0067D6E8(&r,&info)==1);
    CHECK(info.probe_calls==2&&info.surface_calls==1&&!info.inclination_calls);
    CHECK(info.projection_calls==1&&info.approximate_q_exception_bits==0x20);
    CHECK(get(p,0x242380)==0&&get(p,0x242384)==0&&get(p,0x242388)==0);
    CHECK(get(p,0x24238C)==0x3F800000U); /* Final VSUB.xyz preserves ones.W. */
    for(i=1;i<=2;++i){fixture(p,&r);p[RECORD+0x2659-BASE]=(u8)i;refusal(p,&r,-2);}
    fixture(p,&r);word(p,RECORD+0x13C,0x7FC00000);refusal(p,&r,-3);
    fixture(p,&r);word(p,RECORD+0xAC,0x7FC00000);
    CHECK(dreadzone_actor_wall_response_0067D6E8(&r,&info)==1); /* Position W genuinely ignored. */
    fixture(p,&r);p[0x220648-BASE]=1;word(p,RECORD+0x2F10,1);word(p,RECORD+0x2F5C,0xFFFFFFFF);
    CHECK(dreadzone_actor_wall_response_0067D6E8(&r,&info)==1&&info.probe_calls==0);
    fixture(p,&r);r.record_guest=RECORD+1;refusal(p,&r,-1);
    fixture(p,&r);r.wall.packet_guest++;refusal(p,&r,-1);
    fixture(p,&r);r.wall.guest_base=0x80000000U;refusal(p,&r,-1);
    fixture(p,&r);r.wall.guest_base=0x70000000U;r.record_guest=0x70001000U;refusal(p,&r,-1);
    fixture(p,&r);r.wall.storage_bytes=17U*1024U*1024U;refusal(p,&r,-1);
    fixture(p,&r);CHECK(dreadzone_actor_wall_response_0067D6E8(&r,(DreadzoneActorWallResponseInfo *)(p+0x100))==-1);
    fixture(p,&r);CHECK(dreadzone_actor_wall_response_0067D6E8(NULL,&info)==-1);
    fixture(p,&r);CHECK(dreadzone_actor_wall_response_0067D6E8(&r,NULL)==-1);
    free(allocation);printf("%u checks, %u failures\n",checks,failures);return failures?1:0;
}
