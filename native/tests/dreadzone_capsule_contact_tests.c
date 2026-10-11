#include "dreadzone_capsule_contact.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks,failures;
#define CHECK(x) do {checks++;if(!(x)){failures++;printf("FAIL line%d: %s\n",__LINE__,#x);}}while(0)
static void put(u8 *p,u32 value){memcpy(p,&value,4);}
static u32 get(const u8 *p){u32 value;memcpy(&value,p,4);return value;}
static void half(u8 *p,u16 value){memcpy(p,&value,2);}
static void real(u8 *p,float value){memcpy(p,&value,4);}
static float value(const u8 *p){float f;memcpy(&f,p,4);return f;}
static u32 vertex(s32 x,s32 y,s32 z){return ((u32)x&1023U)|(((u32)y&1023U)<<10)|(((u32)z&4095U)<<20);}
static void fixture(u8 *p,DreadzoneCapsuleRequest *r){
    u32 i;u8 *tree=p+0x18000,*block=tree+0x80;
    memset(p,0xA5,0x30000);memset(p+0x1000,0,0x10000);
    r->storage=p;r->guest_base=0x400000;r->storage_bytes=0x30000;
    r->center_guest=0x400100;r->packet_guest=0x400200;r->grid_guest=0x401000;
    r->actor_pool_pointer_guest=0x400400;r->actor_pool_count=1;
    r->ignored_actor_guest=0;r->caller_flags=0x1000;r->radius=.5f;r->height=1.f;
    for(i=0;i<5;i++)r->incoming_vf25_29_w[i]=0xA0B0C000U+i;
    for(i=0;i<3;i++)real(p+0x100+i*4,10.f);put(p+0x10C,0x12345678);
    put(p+0x200,0x418000);put(p+0x210,0);put(p+0x400,0x412000);
    half(tree,2);half(tree+2,1);half(tree+4,8);
    half(tree+32,2);half(tree+34,1);put(tree+36,64);
    half(tree+64,2);half(tree+66,1);put(tree+68,0x8002);
    memset(block,0,32);block[0]=1;block[2]=3;
    put(block+4,vertex(-16,-16,-16));put(block+8,vertex(16,-16,-16));put(block+12,vertex(0,16,-16));
    block[16]=0;block[17]=1;block[18]=2;
}
static void unchanged(DreadzoneCapsuleRequest *r,DreadzoneCapsuleInfo *info,int expected){
    u8 *before=(u8 *)malloc(r->storage_bytes);DreadzoneCapsuleInfo old;
    memcpy(before,r->storage,r->storage_bytes);memset(info,0x7C,sizeof(*info));old=*info;
    CHECK(dreadzone_capsule_contact(r,info)==expected);
    CHECK(memcmp(before,r->storage,r->storage_bytes)==0);CHECK(memcmp(&old,info,sizeof(old))==0);free(before);
}
int main(void){
    u8 *allocation=(u8 *)malloc(0x30002),*p=allocation+1;DreadzoneCapsuleRequest r;DreadzoneCapsuleInfo info;u32 i;
    if(!allocation)return 1;allocation[0]=0xC8;allocation[0x30001]=0x9B;
    fixture(p,&r);CHECK(dreadzone_capsule_contact(&r,&info)==1);
    CHECK(get(p+0x210)==1&&get(p+0x218)==0&&get(p+0x21C)==0x1000);
    CHECK(value(p+0x220)==10.f&&value(p+0x224)==10.f&&value(p+0x228)==9.75f);
    CHECK(value(p+0x230)==10.f&&value(p+0x234)==10.f&&get(p+0x238)==0x41240083U);
    CHECK(info.selected_cells==1&&info.cells_processed==1&&info.faces_processed==1&&info.dma_memory_copies==1);
    for(i=0;i<5;i++)CHECK(get(p+0x22C+(i?16+i*16:0))==r.incoming_vf25_29_w[i]);
    CHECK(get(p+0x23C)==0x102U&&get(p+0x10C)==0x12345678);
    fixture(p,&r);r.caller_flags|=0x80;p[0x18093]=0x80;CHECK(dreadzone_capsule_contact(&r,&info)==0);CHECK(get(p+0x210)==1&&get(p+0x21C)==0xA5A5A5A5U);
    fixture(p,&r);put(p+0x210,0xFFFFFFFFU);CHECK(dreadzone_capsule_contact(&r,&info)==0);CHECK(get(p+0x210)==0xFFFFFFFFU);
    fixture(p,&r);put(p+0x210,0x7FFFFFFFU);unchanged(&r,&info,DREADZONE_CAPSULE_UNSUPPORTED);
    fixture(p,&r);r.radius=0;unchanged(&r,&info,DREADZONE_CAPSULE_UNSUPPORTED);
    fixture(p,&r);r.height=NAN;unchanged(&r,&info,DREADZONE_CAPSULE_UNSUPPORTED);
    fixture(p,&r);r.center_guest++;unchanged(&r,&info,DREADZONE_CAPSULE_INVALID);
    fixture(p,&r);r.packet_guest=0x42FFF0;unchanged(&r,&info,DREADZONE_CAPSULE_INVALID);
    fixture(p,&r);put(p+0x18044,0xFFFFF002U);unchanged(&r,&info,DREADZONE_CAPSULE_INVALID);
    fixture(p,&r);p[0x18092]=3;unchanged(&r,&info,DREADZONE_CAPSULE_UNSUPPORTED);
    fixture(p,&r);r.caller_flags|=1;half(p+0x1000,0);p[0x1002]=1;half(p+0x5000,0);
    for(i=0;i<3;i++)real(p+0x12000+i*4,10240.f);real(p+0x1200C,512.f);
    put(p+0x12098,0x414000);put(p+0x120A0,0);put(p+0x1400C,1);
    CHECK(dreadzone_capsule_contact(&r,&info)==0&&get(p+0x120A0)==1);
    CHECK(info.actor_candidates==1&&info.actors_marked==1&&info.actors_filtered==1);
    put(p+0x210,0);put(p+0x120A0,0);put(p+0x1400C,0);put(p+0x14004,1);
    for(i=0;i<3;i++)real(p+0x12010+i*4,0.f);
    unchanged(&r,&info,DREADZONE_CAPSULE_UNSUPPORTED);
    r.ignored_actor_guest=0x412000;CHECK(dreadzone_capsule_contact(&r,&info)==0&&info.actors_ignored==1&&get(p+0x120A0)==0);
    /* Definition.C aliases actor.A0: the branch must reload after marking. */
    fixture(p,&r);r.caller_flags|=0x2001;half(p+0x1000,0);p[0x1002]=1;
    for(i=0;i<3;i++){real(p+0x12000+i*4,10240.f);real(p+0x12010+i*4,0.f);}
    real(p+0x1200C,512.f);put(p+0x12098,0x412094);put(p+0x120A0,0);put(p+0x1209C,0);
    CHECK(dreadzone_capsule_contact(&r,&info)==0&&get(p+0x120A0)==1);
    CHECK(info.actors_marked==1&&info.actors_filtered==1);
    /* actor.A0 aliases the next selected cell's header; preloaded records
     * still visit its original list and observe the updated generation. */
    fixture(p,&r);r.caller_flags|=1;real(p+0x100,64.f);put(p+0x400,0x400F70);
    real(p+0xF70,65536.f);real(p+0xF74,10240.f);real(p+0xF78,10240.f);real(p+0xF7C,512.f);
    for(i=0;i<3;i++)real(p+0xF80+i*4,0.f);
    put(p+0x1008,0x414000);put(p+0x100C,0x10000);put(p+0x1010,0x10000);
    put(p+0x14004,0);put(p+0x1400C,0);half(p+0x5000,0);
    CHECK(dreadzone_capsule_contact(&r,&info)==0&&get(p+0x1010)==1);
    CHECK(info.actor_candidates==2&&info.actors_marked==1&&info.actors_filtered==2);
    CHECK(allocation[0]==0xC8&&allocation[0x30001]==0x9B);
    CHECK(dreadzone_capsule_contact(NULL,&info)==DREADZONE_CAPSULE_INVALID);
    CHECK(dreadzone_capsule_contact(&r,NULL)==DREADZONE_CAPSULE_INVALID);
    free(allocation);printf("%u checks, %u failures\n",checks,failures);return failures?1:0;
}
