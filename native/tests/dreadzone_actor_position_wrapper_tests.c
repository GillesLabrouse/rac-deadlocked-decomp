#include "dreadzone_actor_position_wrapper.h"
#include <fenv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static u32 checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"position check failed line%d\n",__LINE__);return 1;}}while(0)
static void word(u8 *b,u32 a,u32 v){memcpy(b+a-0x21D000,&v,4);}
static void scalar(u8 *b,u32 a,f32 v){memcpy(b+a-0x21D000,&v,4);}
static void short_word(u8 *b,u32 a,u16 v){memcpy(b+a-0x21D000,&v,2);}
static u32 packed(s32 x,s32 y,s32 z){return ((u32)x&1023U)|(((u32)y&1023U)<<10)|(((u32)z&4095U)<<20);}
static void fixture(u8 *b,u32 bytes) {
    u32 r=0x368F60,c,l;memset(b,0,bytes);
    word(b,0x242340,0x400000);word(b,0x222D14,0x2A0000);word(b,r+0x2EF0,0x230000);
    word(b,r+0x25CC,2);word(b,r+0x25D4,2);word(b,0x21DDB8,1);
    scalar(b,0x21DDF4,1.f/60.f);scalar(b,0x21DDEC,1.f);scalar(b,r+0x25C4,1.f);
    for(c=0;c<4;++c)for(l=0;l<4;++l){scalar(b,r+0x20+c*16+l*4,c==l?1.f:0.f);scalar(b,r+0x60+c*16+l*4,c==l?1.f:0.f);}
    scalar(b,r+0x270+8,-1.f);scalar(b,0x2300E8,1.f);
    scalar(b,r+0xA0,10.f);scalar(b,r+0xA4,10.f);scalar(b,r+0xA8,10.f);scalar(b,r+0xAC,1.f);
    scalar(b,r+0x130,.05f);scalar(b,r+0x14C,.25f);scalar(b,r+0x204,.75f);scalar(b,r+0x240,.75f);
    scalar(b,r+0x1F0,2.5f);scalar(b,r+0x1F4,1.f);
    for(c=0;c<32;++c)memcpy(b+r+0x26F0+16*c-0x21D000,b+r+0xA0-0x21D000,16);
    word(b,r+0x2C7C,32);
    /* Independent authored flat triangle, not copied level geometry. */
    short_word(b,0x400000,2);short_word(b,0x400002,1);short_word(b,0x400004,8);
    short_word(b,0x400020,2);short_word(b,0x400022,1);word(b,0x400024,64);
    short_word(b,0x400040,2);short_word(b,0x400042,1);word(b,0x400044,0x8002);
    b[0x400080-0x21D000]=1;b[0x400082-0x21D000]=3;
    word(b,0x400084,packed(-16,-16,-16));word(b,0x400088,packed(16,-16,-16));word(b,0x40008C,packed(0,16,-16));
    b[0x400090-0x21D000]=0;b[0x400091-0x21D000]=2;b[0x400092-0x21D000]=1;b[0x400093-0x21D000]=15;
    scalar(b,r+0xA8,9.76f);
    for(c=0;c<32;++c)memcpy(b+r+0x26F0+16*c-0x21D000,b+r+0xA0-0x21D000,16);
}
int main(void) {
    u32 size=0x400100-0x21D000,r=0x368F60,i;
    u8 *allocation=malloc(size+1),*before=malloc(size),*b;
    DreadzonePositionWrapperRequest q={0};DreadzonePositionWrapperInfo info,old;
    int round=fegetround();
    CHECK(allocation&&before);b=allocation+1;
    q.storage=b;q.guest_base=0x21D000;q.storage_bytes=size;q.record_guest=r;
    q.actor_pool_count=65536;q.grid_block_count=3072;
    for(i=0;i<5;++i)q.incoming_vf25_29_w[i]=0x3F000000U+i;
    fixture(b,size);b[r+0x2672-0x21D000]=1;memcpy(before,b,size);
    CHECK(dreadzone_actor_position_wrapper_00684A60(&q,&info)==1);
    CHECK(info.early_exit==1 && info.position_calls==0 && !memcmp(b,before,size));
    CHECK(!memcmp(info.outgoing_vf25_29_w,q.incoming_vf25_29_w,20));
    fixture(b,size);b[r+0x266D-0x21D000]=1;
    scalar(b,r+0x140,.01f);scalar(b,r+0x2D4,.25f);scalar(b,r+0x2E08,.75f);
    word(b,r+0x2F10,1);short_word(b,r+0x2FE,2);short_word(b,r+0x3A0,1);
    CHECK(dreadzone_actor_position_wrapper_00684A60(&q,&info)==1);
    CHECK(info.position_calls==1 && info.offset_calls==0 && info.mod5_resets==0);
    CHECK(info.position.position_changed==1);
    fixture(b,size);b[r+0x266D-0x21D000]=1;
    scalar(b,r+0x140,.01f);scalar(b,r+0x2D4,.25f);scalar(b,r+0x2E08,.75f);
    word(b,r+0x2F10,1);short_word(b,r+0x2FE,2);short_word(b,r+0x3A0,1);
    word(b,0x21DDB8,2);scalar(b,r+0x2F90,.7f);scalar(b,r+0x2F94,5.f);scalar(b,r+0x2F98,2000.f);
    b[r+0x224-0x21D000]=7;b[r+0x225-0x21D000]=9;
    CHECK(dreadzone_actor_position_wrapper_00684A60(&q,&info)==1);
    CHECK(info.position_calls==1 && info.offset_calls==1 && info.angle_calls==0);
    CHECK(info.offset.original_integer==0 && !b[r+0x224-0x21D000] && !b[r+0x225-0x21D000]);
    /* A reached dependency refusal may follow prior shadow writes. */
    fixture(b,size);b[r+0x2659-0x21D000]=1;memcpy(before,b,size);
    memset(&info,0xA5,sizeof(info));old=info;
    CHECK(dreadzone_actor_position_wrapper_00684A60(&q,&info)<0);
    CHECK(!memcmp(before,b,size) && !memcmp(&info,&old,sizeof(info)));
    fixture(b,size);memcpy(before,b,size);q.record_guest++;
    CHECK(dreadzone_actor_position_wrapper_00684A60(&q,&info)==-1);
    CHECK(!memcmp(before,b,size) && !memcmp(&info,&old,sizeof(info)));q.record_guest--;
    if(fesetround(FE_UPWARD)==0) {
        CHECK(dreadzone_actor_position_wrapper_00684A60(&q,&info)==-3);
        CHECK(!memcmp(before,b,size) && !memcmp(&info,&old,sizeof(info)));
        CHECK(fesetround(round)==0);
    }
    free(before);free(allocation);printf("position wrapper checks: %u\n",checks);return 0;
}
