#include "dreadzone_actor_position_step.h"
#include "dreadzone_actor_ground_step.h"
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
    u32 size=0x400100-0x21D000,r=0x368F60,i;u8 *b=malloc(size),*before=malloc(size);
    DreadzonePositionStepRequest q={0};DreadzonePositionStepInfo info,old;int status;
    CHECK(b&&before);q.storage=b;q.guest_base=0x21D000;q.storage_bytes=size;q.record_guest=r;
    q.actor_pool_count=65536;q.grid_block_count=3072;
    for(i=0;i<5;++i)q.incoming_vf25_29_w[i]=0x3F000000U+i;
    fixture(b,size);word(b,r+0x364,0xFFFFFFFFU);
    status=dreadzone_actor_position_step_0067F5E8(&q,&info);
    CHECK(status==1);CHECK(info.object_exits==1);CHECK(info.capsule_resolve_calls==1);
    CHECK(info.ground_calls==1);CHECK(info.settle_calls==1);CHECK(info.position_changed==1);
    CHECK(info.outgoing_vf25_29_w[0]==q.incoming_vf25_29_w[0]);CHECK(info.outgoing_vf25_29_w[3]==0x3C055360U);
    CHECK(info.outgoing_vf25_29_w[1]==q.incoming_vf25_29_w[1]);
    CHECK(info.outgoing_vf25_29_w[2]==q.incoming_vf25_29_w[2]);
    CHECK(info.outgoing_vf25_29_w[4]==0);
    {u32 flags;memcpy(&flags,b+r+0x364-0x21D000,4);CHECK(flags==0xFFFFFFFCU);}
    fixture(b,size);scalar(b,r+0xAC,0.f);status=dreadzone_actor_position_step_0067F5E8(&q,&info);
    CHECK(status==1);CHECK(info.outgoing_vf25_29_w[3]==0);
    /* A genuine terrain miss bypasses EA88's full-width A0 producer. Its
     * definite earlier ground publisher closes the C60 W input instead. */
    {
        DreadzoneGroundRequest ground={0};DreadzoneGroundInfo observed;u16 timer;
        fixture(b,size);scalar(b,r+0xA0,100.f);scalar(b,r+0xA4,100.f);
        for(i=0;i<32;++i)memcpy(b+r+0x26F0+16*i-0x21D000,b+r+0xA0-0x21D000,16);
        memcpy(before,b,size);
        ground.storage=before;ground.guest_base=q.guest_base;ground.storage_bytes=size;
        ground.record_guest=r;ground.actor_pool_count=q.actor_pool_count;ground.grid_block_count=q.grid_block_count;
        CHECK(dreadzone_actor_ground_step(&ground,&observed)==1);
        memcpy(&timer,before+r+0x2FE-0x21D000,2);
        CHECK(observed.probe_calls!=0&&timer!=0);CHECK(observed.ground_contacts==0);
        CHECK(dreadzone_actor_position_step_0067F5E8(&q,&info)==1);
        CHECK(info.ground_calls==1 && info.settle_calls==1 && info.ground_contacts==0);
        CHECK(info.outgoing_vf25_29_w[3]==0);
        CHECK(info.position_changed==1);

        /* A clipped terminal ray preserves actual endpoint W1 rather than
         * clearing it based on its miss result. */
        fixture(b,size);scalar(b,r+0xA0,100.f);scalar(b,r+0xA4,100.f);scalar(b,r+0xA8,1.f);
        for(i=0;i<32;++i)memcpy(b+r+0x26F0+16*i-0x21D000,b+r+0xA0-0x21D000,16);
        b[r+0x266D-0x21D000]=1;
        CHECK(dreadzone_actor_position_step_0067F5E8(&q,&info)==1);
        CHECK(info.ground_calls==1 && info.ground_contacts==0 && info.settle_calls==1);
        CHECK(info.outgoing_vf25_29_w[3]==0x3C055360U);

        /* A scheduled miss has no definite ground publisher. Nonzero half2FE
         * cannot consume unknown capsule state even when all RAM effects fit. */
        fixture(b,size);scalar(b,r+0xA0,100.f);scalar(b,r+0xA4,100.f);
        scalar(b,r+0x2D4,.25f);scalar(b,r+0x2E08,.75f);word(b,r+0x2F10,1);short_word(b,r+0x2FE,2);
        b[r+0x266D-0x21D000]=1;
        for(i=0;i<32;++i)memcpy(b+r+0x26F0+16*i-0x21D000,b+r+0xA0-0x21D000,16);
        memset(&info,0xA5,sizeof(info));old=info;memcpy(before,b,size);
        CHECK(dreadzone_actor_position_step_0067F5E8(&q,&info)==DREADZONE_POSITION_STEP_UNSUPPORTED_PATH);
        CHECK(!memcmp(b,before,size) && !memcmp(&info,&old,sizeof(info)));
    }
    for(i=0;i<7;++i) {
        fixture(b,size);memset(&info,0xA5,sizeof(info));old=info;
        if(i==0)word(b,r+0x2EC,0x230000);
        if(i==1)b[r+0x2659-0x21D000]=1;
        if(i==2)word(b,r+0x204,0x7FC00000U);
        if(i==3){word(b,r+0x360,0x230000);b[r+0x2FE-0x21D000]=1;}
        if(i==4)word(b,r+0x2EF0,0x500000);
        if(i==5)scalar(b,r+0x140,.01f);
        if(i==6)word(b,r+0xAC,0x7FC00000U);
        memcpy(before,b,size);status=dreadzone_actor_position_step_0067F5E8(&q,&info);
        CHECK(status<0);CHECK(memcmp(b,before,size)==0);CHECK(memcmp(&info,&old,sizeof(info))==0);
    }
    fixture(b,size);memset(&info,0xA5,sizeof(info));old=info;q.record_guest|=1U;
    memcpy(before,b,size);CHECK(dreadzone_actor_position_step_0067F5E8(&q,&info)<0);
    CHECK(!memcmp(b,before,size));CHECK(!memcmp(&info,&old,sizeof(info)));
    free(before);free(b);printf("position step checks: %u\n",checks);return 0;
}

