#include "dreadzone_actor_environment_read.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum {SIZE=512,BASE=0x100000,ACTOR=BASE+64};
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"environment reader failed line %d\n",__LINE__);free(ram);return 1;}}while(0)
static uint32_t read_word(const uint8_t *p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void put(uint8_t *p,uint32_t w){p[0]=(uint8_t)w;p[1]=(uint8_t)(w>>8);p[2]=(uint8_t)(w>>16);p[3]=(uint8_t)(w>>24);}
int main(void){
    uint8_t *ram=(uint8_t *)malloc(SIZE),before[SIZE];
    DreadzoneActorEnvironmentReadRequest q;
    if(!ram)return 1;
    memset(ram,0xA5,SIZE);q.storage=ram;q.guest_base=BASE;q.storage_bytes=SIZE;q.actor_guest=ACTOR;
    q.output_guest[0]=BASE+256;q.output_guest[1]=BASE+260;q.output_guest[2]=BASE+264;
    put(ram+64+0x3C,0x81234567);
    CHECK(dreadzone_actor_environment_read_0058D4A0(&q)==1);
    CHECK(read_word(ram+256)==0x67&&read_word(ram+260)==0x45&&read_word(ram+264)==0x23);
    /* Repeated destinations retain the last store, even when they alias input. */
    q.output_guest[0]=q.output_guest[1]=q.output_guest[2]=ACTOR+0x3C;
    put(ram+64+0x3C,0x81234567);
    CHECK(dreadzone_actor_environment_read_0058D4A0(&q)==1&&read_word(ram+64+0x3C)==0x23);
    q.output_guest[0]=ACTOR+0x3C;q.output_guest[1]=BASE+256;q.output_guest[2]=BASE+260;
    put(ram+64+0x3C,0xFEDCBA98);
    CHECK(dreadzone_actor_environment_read_0058D4A0(&q)==1&&read_word(ram+64+0x3C)==0x98&&read_word(ram+256)==0xBA&&read_word(ram+260)==0xDC);
    /* Last write would fault in retail after earlier writes; native refuses all. */
    q.output_guest[2]=BASE+SIZE;memcpy(before,ram,SIZE);
    CHECK(dreadzone_actor_environment_read_0058D4A0(&q)==-2&&!memcmp(ram,before,SIZE));
    q.output_guest[2]=BASE+260;q.output_guest[1]=BASE+257;memcpy(before,ram,SIZE);
    CHECK(dreadzone_actor_environment_read_0058D4A0(&q)==-2&&!memcmp(ram,before,SIZE));
    q.output_guest[1]=BASE+256;q.actor_guest=ACTOR+1;memcpy(before,ram,SIZE);
    CHECK(dreadzone_actor_environment_read_0058D4A0(&q)==-2&&!memcmp(ram,before,SIZE));
    q.actor_guest=ACTOR;q.guest_base=0x6FFFF000;q.storage_bytes=0x3000;
    CHECK(dreadzone_actor_environment_read_0058D4A0(&q)==-1&&!memcmp(ram,before,SIZE));
    CHECK(dreadzone_actor_environment_read_0058D4A0(NULL)==-1);
    q.guest_base=BASE;q.storage_bytes=SIZE;
    /* The original request's pointer gets overwritten after it is captured. */
    {
        DreadzoneActorEnvironmentReadRequest *alias=(DreadzoneActorEnvironmentReadRequest *)(ram+320);
        q.output_guest[0]=BASE+320;q.output_guest[1]=BASE+324;q.output_guest[2]=BASE+328;
        memcpy(alias,&q,sizeof(q));put(ram+64+0x3C,0xFEDCBA98);
        CHECK(dreadzone_actor_environment_read_0058D4A0(alias)==1&&read_word(ram+320)==0x98&&read_word(ram+324)==0xBA&&read_word(ram+328)==0xDC);
    }
    free(ram);printf("environment reader checks: %u\n",checks);return 0;
}
