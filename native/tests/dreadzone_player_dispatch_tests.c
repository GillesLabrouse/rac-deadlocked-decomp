#include "dreadzone_player_dispatch.h"
#include <fenv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { BASE=0x100000,SIZE=0x300000,RECORD=0x368F60,OBJECT=0x3F0000 };
static u32 checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;printf("FAIL %d: %s\n",__LINE__,#x);}}while(0)
static void word(u8 *p,u32 a,u32 v){memcpy(p+a-BASE,&v,4);}
static u32 get(const u8 *p,u32 a){u32 v;memcpy(&v,p+a-BASE,4);return v;}
static void fixture(u8 *p,DreadzonePlayerDispatchRequest *r){
    memset(p,0,SIZE);memset(r,0,sizeof(*r));r->wall.storage=p;r->wall.guest_base=BASE;r->wall.storage_bytes=SIZE;
    r->wall.packet_guest=0x242340;r->wall.grid_guest=0x26FF20;r->wall.actor_pool_pointer_guest=0x222D14;
    r->wall.actor_pool_count=65536;r->wall.grid_block_count=3072;r->record_guest=RECORD;r->gp_guest=0x22597F;
    r->wall.incoming_vf25_27_w[0]=0x12345678;r->wall.incoming_vf25_27_w[1]=0x87654321;r->wall.incoming_vf25_27_w[2]=0xDEADBEEF;
    r->incoming_vf28_29_w[0]=0x3F000000;r->incoming_vf28_29_w[1]=0x80000000;
    p[0x171DE0-BASE]=1;word(p,0x3B08C0,0x690FE0);word(p,0x21DDF4,0x3F800000);word(p,0x21DDF8,0x3C800000);
    word(p,RECORD+0x25C4,0x3F800000);word(p,RECORD+0x2EF0,OBJECT);p[OBJECT+0x54-BASE]=23;
    word(p,RECORD+0x25D0,0xDEADBEEF);word(p,RECORD+0x378,0x7FC12345);word(p,RECORD+0x37C,0xFFC23456);
    word(p,RECORD+0x1970,0x80000000);word(p,RECORD+0x1974,0x7FC12345);word(p,RECORD+0x1978,0xDEADBEEF);word(p,RECORD+0x19B0,0xFFC23456);
}
static void refusal(u8 *p,DreadzonePlayerDispatchRequest *r){
    u8 *before=(u8 *)malloc(SIZE);DreadzonePlayerDispatchInfo info,old;int status;
    if(!before){CHECK(0);return;}memcpy(before,p,SIZE);memset(&info,0x6c,sizeof(info));old=info;
    status=dreadzone_player_dispatch_00690F18(r,&info);CHECK(status<0);CHECK(!memcmp(before,p,SIZE));CHECK(!memcmp(&info,&old,sizeof(info)));free(before);
}
int main(void){
    u8 *allocation=(u8 *)malloc(SIZE+16),*p;DreadzonePlayerDispatchRequest r,*alias;DreadzonePlayerDispatchInfo info;
    if(!allocation)return 1;p=allocation+1;fixture(p,&r);
    CHECK(dreadzone_player_dispatch_00690F18(&r,&info)==1);CHECK(info.idle_calls==1&&info.cleared_378==0&&info.cleared_37c==0);
    CHECK(get(p,RECORD+0x378)==0x7FC12345&&get(p,RECORD+0x37C)==0xFFC23456);
    CHECK(!memcmp(p+RECORD+0x1790-BASE,p+RECORD+0x1970-BASE,16)&&get(p,RECORD+0x17A0)==0xFFC23456);
    CHECK(info.idle.wall.outgoing_vf25_29_w[0]==0x12345678&&info.idle.wall.outgoing_vf25_29_w[4]==0x80000000);
    fixture(p,&r);word(p,RECORD+0x2EF0,RECORD+0x1740);
    CHECK(dreadzone_player_dispatch_00690F18(&r,&info)==1);CHECK(info.cleared_37c==1&&info.cleared_378==0);
    CHECK(get(p,RECORD+0x37C)==0&&get(p,RECORD+0x378)==0x7FC12345);
    fixture(p,&r);alias=(DreadzonePlayerDispatchRequest *)(void *)(p+0x1000);memcpy(alias,&r,sizeof(r));
    CHECK(dreadzone_player_dispatch_00690F18(alias,&info)==1);CHECK(!memcmp(alias,&r,sizeof(r)));
    fixture(p,&r);word(p,0x3B08C0,0x690F90);refusal(p,&r);
    fixture(p,&r);word(p,RECORD+0x25CC,1);refusal(p,&r);
    fixture(p,&r);word(p,RECORD+0x2EF0,0xFFFFFFC0);refusal(p,&r);
    fixture(p,&r);word(p,RECORD+0x2EF0,0x500000);refusal(p,&r);
    fixture(p,&r);word(p,RECORD+0x1A0,0x3F800000);refusal(p,&r); /* inner refusal discards earlier copied payload */
    fixture(p,&r);word(p,0x21DDF8,0x40000000);refusal(p,&r); /* late inner height failure */
    fixture(p,&r);r.wall.storage_bytes=0x3B08C0-BASE;refusal(p,&r);
    fixture(p,&r);CHECK(fesetround(FE_UPWARD)==0);refusal(p,&r);CHECK(fesetround(FE_TONEAREST)==0);
    fixture(p,&r);CHECK(dreadzone_player_dispatch_00690F18(&r,(DreadzonePlayerDispatchInfo *)(void *)(p+0x1000))<0);
    printf("%u checks, %u failures\n",checks,failures);free(allocation);return failures?1:0;
}
