#include "dreadzone_actor_visibility.h"
#include <stdio.h>
#include <string.h>
#define BASE 0x230000U
#define ACTOR (BASE+0x100U)
#define DEFINITION (BASE+0x800U)
static u32 checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"actor visibility check failed line%d\n",__LINE__);return 1;}}while(0)
static void word(u8 *b,u32 a,u32 value){memcpy(b+a-BASE,&value,4);}
static void half(u8 *b,u32 a,u16 value){memcpy(b+a-BASE,&value,2);}
static u16 flags(u8 *b){u16 value;memcpy(&value,b+ACTOR+0x36-BASE,2);return value;}
static void fixture(u8 *b){memset(b,0xA5,0x2000);word(b,ACTOR+0x24,DEFINITION);b[DEFINITION+0xB-BASE]=1;half(b,ACTOR+0x36,0xFFFF);}
int main(void){
    u8 b[0x2000],before[0x2000];DreadzoneActorVisibilityRequest q={b,BASE,sizeof(b),ACTOR,10};
    DreadzoneActorVisibilityInfo info,old;
    fixture(b);
    CHECK(dreadzone_actor_visibility_005A8630(&q,&info)==1);
    CHECK(info.halfword_stores==2&&!info.skipped&&info.flags==0xD7FF&&flags(b)==0xD7FF);
    q.mode_word=0;CHECK(dreadzone_actor_visibility_005A8630(&q,&info)==1);
    CHECK(info.flags==0x87FF&&flags(b)==0x87FF);
    q.mode_word=0xFFFFFFFFU;CHECK(dreadzone_actor_visibility_005A8630(&q,&info)==1);
    CHECK(info.flags==0xFFFF&&flags(b)==0xFFFF);
    fixture(b);word(b,ACTOR+0x24,0);memcpy(before,b,sizeof(b));
    CHECK(dreadzone_actor_visibility_005A8630(&q,&info)==1);
    CHECK(info.skipped&&!info.halfword_stores&&!memcmp(before,b,sizeof(b)));
    fixture(b);b[DEFINITION+0xB-BASE]=0;memcpy(before,b,sizeof(b));
    CHECK(dreadzone_actor_visibility_005A8630(&q,&info)==1);
    CHECK(info.skipped&&!info.halfword_stores&&!memcmp(before,b,sizeof(b)));
    fixture(b);word(b,ACTOR+0x24,ACTOR+0x36-0xB);half(b,ACTOR+0x36,1);q.mode_word=10;
    CHECK(dreadzone_actor_visibility_005A8630(&q,&info)==1);
    CHECK(info.flags==0xD001&&flags(b)==0xD001);
    fixture(b);word(b,ACTOR+0x24,ACTOR+0x24-0xB);q.mode_word=0;
    CHECK(dreadzone_actor_visibility_005A8630(&q,&info)==1);
    CHECK(info.flags==0x87FF&&flags(b)==0x87FF);
    fixture(b);word(b,ACTOR+0x24,0x500000);memset(&info,0xA5,sizeof(info));old=info;memcpy(before,b,sizeof(b));
    CHECK(dreadzone_actor_visibility_005A8630(&q,&info)==-1);
    CHECK(!memcmp(before,b,sizeof(b))&&!memcmp(&info,&old,sizeof(info)));
    fixture(b);q.actor_guest=BASE+sizeof(b)-0x28;word(b,q.actor_guest+0x24,DEFINITION);memcpy(before,b,sizeof(b));
    CHECK(dreadzone_actor_visibility_005A8630(&q,&info)==-1);
    CHECK(!memcmp(before,b,sizeof(b))&&!memcmp(&info,&old,sizeof(info)));
    CHECK(dreadzone_actor_visibility_005A8630(NULL,&info)==-1);
    CHECK(dreadzone_actor_visibility_005A8630(&q,NULL)==-1);
    CHECK(dreadzone_actor_visibility_005A8630(&q,(DreadzoneActorVisibilityInfo *)b)==-1);
    CHECK(!memcmp(before,b,sizeof(b)));
    fixture(b);q.actor_guest=ACTOR;memcpy(b+0x1000,&q,sizeof(q));
    CHECK(dreadzone_actor_visibility_005A8630((DreadzoneActorVisibilityRequest *)(b+0x1000),&info)==1);
    CHECK(info.flags==0x87FF&&flags(b)==0x87FF);
    printf("actor visibility checks: %u\n",checks);return 0;
}
