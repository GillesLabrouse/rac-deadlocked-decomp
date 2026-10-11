#include "dreadzone_actor_mode.h"
#include <stdio.h>
#include <string.h>
static u8 backing[0x8000],snapshot[0x8000];
static u32 checks;
#define CHECK(x) do { ++checks;if(!(x)){fprintf(stderr,"mode check failed line %d\n",__LINE__);return 1;} } while(0)
static void word(u32 a,u32 value) {memcpy(backing+a-0x21E000,&value,4);}
static void half(u32 a,s16 value) {memcpy(backing+a-0x21E000,&value,2);}
static void fixture(void) {
    memset(backing,0,sizeof(backing));word(0x2226E4,0x225000);
}
int main(void) {
    DreadzoneActorModeRequest q={backing,0x21E000,sizeof(backing),0x220000};
    DreadzoneActorModeInfo info,old;u32 i;
    fixture();half(0x2202F8,-1);
    CHECK(dreadzone_actor_mode_0068A090(&q,&info)==1);CHECK(info.mode==-1);CHECK(info.metadata_queries==0);
    word(0x21E878,1);CHECK(dreadzone_actor_mode_0068A090(&q,&info)==1);CHECK(info.mode==2);
    word(0x21E87C,1);CHECK(dreadzone_actor_mode_0068A090(&q,&info)==1);CHECK(info.mode==1);
    backing[0x438+0x2000]=1;
    CHECK(dreadzone_actor_mode_0068A090(&q,&info)==1);CHECK(info.mode==0);
    backing[0x437+0x2000]=1;
    CHECK(dreadzone_actor_mode_0068A090(&q,&info)==1);CHECK(info.mode==1);CHECK(info.metadata_queries==0);
    fixture();backing[0x437+0x2000]=1;
    CHECK(dreadzone_actor_mode_0068A090(&q,&info)==1);CHECK(info.mode==1);CHECK(info.metadata_queries==1);
    backing[0x225EF8-0x21E000]=0x80;
    CHECK(dreadzone_actor_mode_0068A090(&q,&info)==1);CHECK(info.mode==0);CHECK(info.metadata_queries==1);
    word(0x2225CC,2);backing[0x225EF8-0x21E000]=0;
    CHECK(dreadzone_actor_mode_0068A090(&q,&info)==1);CHECK(info.mode==0);CHECK(info.metadata_queries==1);
    for(i=0;i<3;++i) {
        fixture();word(0x2225CC,(u32[]){59,111,112}[i]);
        CHECK(dreadzone_actor_mode_0068A090(&q,&info)==1);CHECK(info.mode==1);CHECK(info.metadata_queries==0);
    }
    fixture();half(0x2203D6,-32768);
    CHECK(dreadzone_actor_mode_0068A090(&q,&info)==1);CHECK(info.mode==1);
    fixture();word(0x2226E4,0xFFFFFFFFU);
    CHECK(dreadzone_actor_mode_0068A090(&q,&info)==1);CHECK(info.mode==0);
    backing[0x437+0x2000]=1;memset(&info,0xA5,sizeof(info));old=info;memcpy(snapshot,backing,sizeof(backing));
    CHECK(dreadzone_actor_mode_0068A090(&q,&info)==-1);CHECK(!memcmp(&info,&old,sizeof(info)));CHECK(!memcmp(backing,snapshot,sizeof(backing)));
    fixture();memcpy(snapshot,backing,sizeof(backing));
    CHECK(dreadzone_actor_mode_0068A090(&q,(DreadzoneActorModeInfo *)backing)==-1);CHECK(!memcmp(backing,snapshot,sizeof(backing)));
    q.record_guest|=1;CHECK(dreadzone_actor_mode_0068A090(&q,&info)==-1);CHECK(!memcmp(&info,&old,sizeof(info)));
    printf("actor mode checks: %u\n",checks);return 0;
}
