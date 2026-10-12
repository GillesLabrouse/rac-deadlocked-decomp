#include "dreadzone_player_prefix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { BASE=0x00100000U,RAM_BYTES=0x00300000U,RECORD=0x00368F60U,IDLE_OBJECT=0x003F0000U,CALLBACK_OBJECT=0x003B0000U };
static unsigned checks,failures;
#define CHECK(x) do { ++checks; if(!(x)){++failures;fprintf(stderr,"prefix line %d: %s\n",__LINE__,#x);} } while(0)
static void put32(u8 *p,u32 a,u32 v){memcpy(p+a-BASE,&v,4);}
static u32 get32(const u8 *p,u32 a){u32 v;memcpy(&v,p+a-BASE,4);return v;}
static void put16(u8 *p,u32 a,u16 v){memcpy(p+a-BASE,&v,2);}
static void fixture(u8 *p,DreadzonePlayerPrefixRequest *r){
    memset(p,0,RAM_BYTES);memset(r,0,sizeof(*r));r->wall.storage=p;r->wall.guest_base=BASE;r->wall.storage_bytes=RAM_BYTES;
    r->wall.packet_guest=0x242340;r->wall.grid_guest=0x26FF20;r->wall.actor_pool_pointer_guest=0x222D14;
    r->wall.actor_pool_count=65536;r->wall.grid_block_count=3072;r->record_guest=RECORD;r->gp_guest=0x22597F;
    r->wall.incoming_vf25_27_w[0]=0x12345678;r->wall.incoming_vf25_27_w[1]=0x87654321;r->wall.incoming_vf25_27_w[2]=0xDEADBEEF;
    r->incoming_vf28_29_w[0]=0x3F000000;r->incoming_vf28_29_w[1]=0x80000000;
    p[0x171DE0-BASE]=1;put32(p,0x3B08C0,0x690FE0);put32(p,0x21DDF4,0x3F800000);put32(p,0x21DDF8,0x3C800000);
    put32(p,RECORD+0x25C4,0x3F800000);put32(p,RECORD+0x2EF0,IDLE_OBJECT);p[IDLE_OBJECT+0x54-BASE]=23;
    put32(p,RECORD+0x25D0,0xDEADBEEF);put32(p,RECORD+0x378,0x7FC12345);put32(p,RECORD+0x37C,0xFFC23456);
    put32(p,RECORD+0x1970,0x80000000);put32(p,RECORD+0x1974,0x7FC12345);put32(p,RECORD+0x1978,0xDEADBEEF);put32(p,RECORD+0x19B0,0xFFC23456);
    p[RECORD+0x266D-BASE]=1;put32(p,RECORD+0x14,CALLBACK_OBJECT);
    put32(p,CALLBACK_OBJECT+0x4C,0x6A9630);put32(p,CALLBACK_OBJECT+0x34,0x690F18);
}
static void nonnull_slot(u8 *p){
    u32 P=RECORD+0x58U,Q=0x003E0000U;
    put32(p,0x2303A4,P);put32(p,P+0x24,Q);put32(p,Q+0x10,0xA5B6C7D8U);
}
static int equal(const void *a,const void *b,size_t n){return memcmp(a,b,n)==0;}
static void refusal(u8 *p,DreadzonePlayerPrefixRequest *r){
    u8 *before=(u8 *)malloc(RAM_BYTES);DreadzonePlayerPrefixInfo info,old;int status;
    if(!before){CHECK(0);return;}memcpy(before,p,RAM_BYTES);memset(&info,0x6C,sizeof(info));old=info;
    status=dreadzone_player_prefix_0067C010(r,&info);CHECK(status<0);CHECK(equal(before,p,RAM_BYTES));CHECK(equal(&info,&old,sizeof(info)));free(before);
}
int main(void){
    u8 *allocation,*p,*before;
    DreadzonePlayerPrefixRequest r,*alias,captured;DreadzonePlayerPrefixInfo info;int status;
    allocation=(u8 *)malloc(RAM_BYTES+16);before=(u8 *)malloc(RAM_BYTES);if(!allocation||!before)return 2;
    p=allocation+1; /* The guest record is naturally aligned while host storage is not. */

    fixture(p,&r);memset(&info,0,sizeof(info));status=dreadzone_player_prefix_0067C010(&r,&info);
    CHECK(status==1);CHECK(info.stub_calls==1&&info.getter_calls==1&&info.slot_calls==1&&info.mode_calls==1&&info.dispatch_calls==1);
    CHECK(info.slot.return_low==0x230280U&&info.slot.did_store==0&&info.mode.mode==0);
    CHECK(get32(p,RECORD+0x2659)==0&&get32(p,RECORD+0x210)==0);
    CHECK(equal(p+RECORD+0xA0-BASE,p+RECORD+0x110-BASE,16));

    /* Full helper result 256 is retained while the original SB stores low byte zero. */
    fixture(p,&r);put16(p,RECORD+0x2F8,0x0100);CHECK(dreadzone_player_prefix_0067C010(&r,&info)==1);
    CHECK(info.mode.mode==256&&get32(p,RECORD+0x2659)==0&&info.dispatch_calls==1);

    /* Real helper chain, with P+0x98 aliasing the later F0 vector read. */
    fixture(p,&r);nonnull_slot(p);CHECK(dreadzone_player_prefix_0067C010(&r,&info)==1);
    CHECK(info.slot.did_store==1&&info.slot.entry_pointer==RECORD+0x58);
    CHECK(info.slot.store_guest==RECORD+0xF0&&get32(p,RECORD+0xF0)==0xA5B6C7D8U);
    CHECK(info.retained_f0_words[0]==0xA5B6C7D8U&&info.dispatch_calls==1);

    /* Request captured from backing memory is allowed; publication leaves the descriptor intact. */
    fixture(p,&r);alias=(DreadzonePlayerPrefixRequest *)(void *)(p+0x1000);memcpy(alias,&r,sizeof(r));
    CHECK(dreadzone_player_prefix_0067C010(alias,&info)==1);memcpy(&captured,alias,sizeof(captured));
    CHECK(captured.wall.storage==p&&captured.record_guest==RECORD&&captured.gp_guest==0x22597F);

    fixture(p,&r);r.gp_guest+=4;refusal(p,&r);
    fixture(p,&r);put32(p,CALLBACK_OBJECT+0x4C,0x6A9634);refusal(p,&r);
    fixture(p,&r);put32(p,0x21E824,1);refusal(p,&r);

    /* Late callback refusal follows a successful slot store in the private shadow; all output rolls back. */
    fixture(p,&r);nonnull_slot(p);put32(p,CALLBACK_OBJECT+0x34,0x690F20);refusal(p,&r);
    fixture(p,&r);nonnull_slot(p);put16(p,RECORD+0x2F8,1);refusal(p,&r);

    fixture(p,&r);memcpy(before,p,RAM_BYTES);memset(&info,0x39,sizeof(info));
    status=dreadzone_player_prefix_0067C010(&r,(DreadzonePlayerPrefixInfo *)(void *)(p+0x2000));CHECK(status<0);CHECK(equal(p,before,RAM_BYTES));
    printf("player-prefix: %u checks, %u failures\n",checks,failures);
    free(before);free(allocation);return failures?1:0;
}
