#include "dreadzone_actor_record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE 0x200000u
#define SIZE 0x680000u
#define ACTOR 0x810000u
#define PAYLOAD 0x820000u
#define META 0x3C6D8Cu
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"actor record check failed line %d\n",__LINE__); free(ram); free(before); return 1; } } while (0)
static void word(uint8_t *ram,uint32_t a,uint32_t v) {
    unsigned i;for(i=0;i<4;++i)ram[a-BASE+i]=(uint8_t)(v>>(i*8));
}
static void half(uint8_t *ram,uint32_t a,uint16_t v) {
    ram[a-BASE]=(uint8_t)v;ram[a-BASE+1]=(uint8_t)(v>>8);
}
static void setup(uint8_t *ram) {
    memset(ram,0,SIZE);
    ram[ACTOR+0x22-BASE]=255;half(ram,ACTOR+0xBC,0x8000);
    word(ram,ACTOR+0xAC,PAYLOAD);word(ram,PAYLOAD+0xE0,0x81234567);
    half(ram,0x26B2B0+510,0x8000);half(ram,0x26B490+510,0x8000);
    ram[0x26B1C0+255-BASE]=7;ram[0x26DF90+255-BASE]=8;
    word(ram,META+4,0x666140);
}
int main(void) {
    uint8_t *ram=(uint8_t *)malloc(SIZE),*before=(uint8_t *)malloc(SIZE);
    DreadzoneActorRecordRequest request;
    DreadzoneActorRecordInfo info,old;
    int status;unsigned i;
    const uint32_t words[]={0,0x7FFFFFFF,0x80000000,0xFFFFFFFF};
    const int64_t values[]={0,2147483647LL,-2147483648LL,-1};
    if(!ram||!before){free(ram);free(before);return 1;}
    request.ram=ram;request.ram_base=BASE;request.ram_size=SIZE;request.actor=ACTOR;
    setup(ram);
    for(i=0;i<4;++i){
        word(ram,PAYLOAD+0xE0,words[i]);memcpy(before,ram,SIZE);
        CHECK(dreadzone_actor_record_00667008(&request,&info)==DREADZONE_ACTOR_RECORD_OK);
        CHECK(info.status==DREADZONE_ACTOR_RECORD_OK&&info.metadata_address==META&&info.method_address==0x666140&&info.value==values[i]);
        CHECK(!memcmp(ram,before,SIZE));
    }
    /* Both match: the normal table wins even when the alternate method differs. */
    word(ram,META+32+4,0x586938);
    CHECK(dreadzone_actor_record_00667008(&request,&info)==DREADZONE_ACTOR_RECORD_OK&&info.metadata_address==META);
    half(ram,0x26B2B0+510,0x8001);ram[0x26DF90+255-BASE]=7;
    CHECK(dreadzone_actor_record_00667008(&request,&info)==DREADZONE_ACTOR_RECORD_OK&&info.metadata_address==META);
    word(ram,ACTOR+0xAC,0);
    CHECK(dreadzone_actor_record_00667008(&request,&info)==DREADZONE_ACTOR_RECORD_OK&&info.value==0);
    word(ram,ACTOR+0xAC,ACTOR);word(ram,ACTOR+0xE0,0x80000000);
    CHECK(dreadzone_actor_record_00667008(&request,&info)==DREADZONE_ACTOR_RECORD_OK&&info.value==-2147483648LL);
    /* Each refusal must preserve the entire output including padding. */
    memset(&old,0xA5,sizeof(old));info=old;
    word(ram,ACTOR+0xAC,0x2000000);memcpy(before,ram,SIZE);
    CHECK(dreadzone_actor_record_00667008(&request,&info)==DREADZONE_ACTOR_RECORD_UNSUPPORTED&&!memcmp(&info,&old,sizeof(info))&&!memcmp(ram,before,SIZE));
    word(ram,META+4,0x586938);info=old;
    CHECK(dreadzone_actor_record_00667008(&request,&info)==DREADZONE_ACTOR_RECORD_UNSUPPORTED&&!memcmp(&info,&old,sizeof(info)));
    setup(ram);half(ram,0x26B2B0+510,0x8001);half(ram,0x26B490+510,0x8002);word(ram,0x3C6CAC+4,0x586938);info=old;
    CHECK(dreadzone_actor_record_00667008(&request,&info)==DREADZONE_ACTOR_RECORD_UNSUPPORTED&&!memcmp(&info,&old,sizeof(info)));
    word(ram,0x3C5E00,0xFFFFFFFF);word(ram,0x3B8320+4,0x586938);info=old;
    CHECK(dreadzone_actor_record_00667008(&request,&info)==DREADZONE_ACTOR_RECORD_UNSUPPORTED&&!memcmp(&info,&old,sizeof(info)));
    /* True null actor does not read selector/global fields in this small span. */
    request.actor=0;request.ram_size=1;
    CHECK(dreadzone_actor_record_00667008(&request,&info)==DREADZONE_ACTOR_RECORD_OK&&info.metadata_address==0&&info.method_address==0&&info.value==0);
    info=old;
    CHECK(dreadzone_actor_record_00667008(NULL,&info)==DREADZONE_ACTOR_RECORD_INVALID&&!memcmp(&info,&old,sizeof(info)));
    CHECK(dreadzone_actor_record_00667008(&request,NULL)==DREADZONE_ACTOR_RECORD_INVALID);
    request.ram_base=0x6FFFF000;request.ram_size=0x3000;info=old;
    CHECK(dreadzone_actor_record_00667008(&request,&info)==DREADZONE_ACTOR_RECORD_INVALID&&!memcmp(&info,&old,sizeof(info)));
    setup(ram);request.ram_base=BASE;request.ram_size=SIZE;request.actor=ACTOR;
    memcpy(before,ram,SIZE);
    status=dreadzone_actor_record_00667008(&request,(DreadzoneActorRecordInfo *)ram);
    CHECK(status==DREADZONE_ACTOR_RECORD_INVALID&&!memcmp(ram,before,SIZE));
    {
        union {DreadzoneActorRecordRequest r;DreadzoneActorRecordInfo i;} alias;
        uint8_t saved[sizeof(alias)];memset(&alias,0,sizeof(alias));alias.r=request;memcpy(saved,&alias,sizeof(alias));
        CHECK(dreadzone_actor_record_00667008(&alias.r,&alias.i)==DREADZONE_ACTOR_RECORD_INVALID&&!memcmp(&alias,saved,sizeof(alias)));
    }
    {
        DreadzoneActorRecordRequest *alias=(DreadzoneActorRecordRequest *)(ram+SIZE-256);
        memcpy(alias,&request,sizeof(request));memcpy(before,ram,SIZE);
        CHECK(dreadzone_actor_record_00667008(alias,&info)==DREADZONE_ACTOR_RECORD_OK&&info.value==-2128394905LL&&!memcmp(ram,before,SIZE));
    }
    free(ram);free(before);printf("actor record checks: %u\n",checks);return 0;
}
