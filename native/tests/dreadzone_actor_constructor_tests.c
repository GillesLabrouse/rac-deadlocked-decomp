#include "dreadzone_actor_constructor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Synthetic integration fixture, without retail resources or startup claims.
 * The authentic whole-RAM reference comparisons are separate private proofs. */
enum { BASE=0x100000, BYTES=0xA00000, RECORD=0x368F60,
       ACTOR=0x700000, DEF=0x400000, VTABLE=0x3B10C0, FRAMES=0x401000 };
static u8 *ram,*before,*completed,spr[0x4000],vu[0x1000];
static u32 w[3],side[2];
static unsigned checks,failures;
#define CHECK(x) do { checks++;if(!(x)){failures++;fprintf(stderr,"line%d: %s\n",__LINE__,#x);} } while(0)
static void put(u32 a,u32 x){memcpy(ram+a-BASE,&x,4);}
static void half(u32 a,u16 x){memcpy(ram+a-BASE,&x,2);}
static u32 get(u32 a){u32 x;memcpy(&x,ram+a-BASE,4);return x;}
static DreadzoneActorConstructor setup(void){
    DreadzoneActorConstructor q={0};unsigned item;
    memset(ram,0,BYTES);memset(spr,0,sizeof(spr));memset(vu,0,sizeof(vu));
    memset(w,0,sizeof(w));memset(side,0,sizeof(side));
    put(RECORD+0x14,VTABLE);put(VTABLE+0x28,0x6A6598);
    put(0x222D14,ACTOR);put(0x222D2C,ACTOR+0x100);
    half(ACTOR+0xBC,0x23F7);put(ACTOR+0x24,DEF);put(DEF+0x24,0x3F000000);
    put(DEF+0x48,FRAMES);half(FRAMES+0x20,4);
    /* A genuine negative packet generation takes the existing early return. */
    put(0x242350,0xFFFFFFFF);
    for(item=0;item<20;item++)put(0x3BE158+item*0x28,100);
    q.storage=ram;q.guest_base=BASE;q.bytes=BYTES;q.record=RECORD;
    q.actor_pool_count=1;q.grid_block_count=3072;
    q.incoming_vf25_27_w=w;q.incoming_side_output_w=side;
    q.initial_scratch=spr;q.scratch_bytes=sizeof(spr);
    q.initial_vu_data=vu;q.vu_data_bytes=sizeof(vu);return q;
}
static void refuse(const DreadzoneActorConstructor *q,int expected){
    u8 old_spr[0x4000],old_vu[0x1000];
    memcpy(before,ram,BYTES);memcpy(old_spr,spr,sizeof(spr));memcpy(old_vu,vu,sizeof(vu));
    CHECK(dreadzone_actor_construct(q)==expected);
    CHECK(!memcmp(before,ram,BYTES));CHECK(!memcmp(old_spr,spr,sizeof(spr)));
    CHECK(!memcmp(old_vu,vu,sizeof(vu)));
}
int main(void){
    DreadzoneActorConstructor q;u8 *descriptor;
    ram=malloc(BYTES);before=malloc(BYTES);completed=malloc(BYTES);
    if(!ram||!before||!completed){free(ram);free(before);free(completed);return 2;}
    q=setup();CHECK(dreadzone_actor_construct(&q)==DREADZONE_CONSTRUCTOR_COMPLETED);
    CHECK(get(RECORD+0x2EF0)==ACTOR&&get(ACTOR+0x90)==RECORD);
    CHECK(get(RECORD+0x1870)==218&&get(0x21DC80)==0x269EC3);
    CHECK(get(RECORD+0x1A54)==0x68FE70&&get(RECORD+0x1AA0)==RECORD);
    CHECK(get(RECORD+0x2F10)==0&&get(RECORD+0x2D98)==4);
    CHECK(ram[0x1D3F90-BASE]==1&&ram[RECORD+0x2ED5-BASE]==255);
    memcpy(completed,ram,BYTES);
    /* Reset overwrites a borrowed descriptor only after the API retains it. */
    q=setup();descriptor=ram+RECORD+0x100-BASE;memcpy(descriptor,&q,sizeof(q));
    CHECK(dreadzone_actor_construct((const DreadzoneActorConstructor *)descriptor)==1);
    CHECK(!memcmp(completed,ram,BYTES));
    q=setup();q.incoming_vf25_27_w=(const u32 *)(ram+RECORD+0x8000-BASE);
    q.incoming_side_output_w=(const u32 *)(ram+RECORD+0x8040-BASE);
    q.initial_scratch=ram+RECORD+0x2000-BASE;q.initial_vu_data=ram+RECORD+0x6000-BASE;
    CHECK(dreadzone_actor_construct(&q)==1);CHECK(!memcmp(completed,ram,BYTES));
    q=setup();put(0x21E754,0x7FC01234);CHECK(dreadzone_actor_construct(&q)==1);
    CHECK(get(RECORD+0x1970+0x134)==0x7FC01234);
    /* Reject missing inventory progression after ground/RNG/camera clone work. */
    q=setup();put(0x3BE158+2*0x28,0);refuse(&q,-2);
    q=setup();put(RECORD+0x14,VTABLE+1);refuse(&q,-2);
    q=setup();put(ACTOR+0x24,DEF+1);refuse(&q,-2);
    q=setup();half(ACTOR+0xBC,0x171C);refuse(&q,-2);
    q=setup();put(0x21E824,1);refuse(&q,-2);
    q=setup();put(0x21E878,1);refuse(&q,-2);
    q=setup();q.record++;refuse(&q,-1);
    q=setup();q.guest_base++;refuse(&q,-1);
    q=setup();q.actor_pool_count=0;refuse(&q,-1);
    q=setup();q.grid_block_count=3073;refuse(&q,-1);
    q=setup();q.scratch_bytes--;refuse(&q,-1);
    q=setup();q.vu_data_bytes--;refuse(&q,-1);
    q=setup();q.incoming_side_output_w=NULL;refuse(&q,-1);
    CHECK(dreadzone_actor_construct(NULL)==-1);
    free(completed);free(before);free(ram);
    printf("actor constructor: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
