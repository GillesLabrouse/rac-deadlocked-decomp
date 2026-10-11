#include "dreadzone_initial_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum {BASE=0x21E800,SIZE=0x40000,HEADER=0x21F000,POINTER=HEADER+0x40,POOL=0x230000,GP=0x22597F};
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"initial pool failed line %d\n",__LINE__);free(before);free(ram);return 1;}}while(0)
static uint32_t read32(const uint8_t *p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void put(uint8_t *ram,uint32_t a,uint32_t v){uint8_t *p=ram+a-BASE;p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static void setup(uint8_t *ram,DreadzoneInitialPoolRequest *q,uint32_t count,uint32_t reserve,uint32_t world){
    memset(ram,0xA5,SIZE);q->storage=ram;q->guest_base=BASE;q->storage_bytes=SIZE;q->header_guest=HEADER;q->pool_pointer_guest=POINTER;q->gp=GP;
    put(ram,0x21E824,world);put(ram,HEADER,count);put(ram,HEADER+4,reserve);put(ram,POINTER,POOL);put(ram,0x21EAC4,1);put(ram,0x21EAD4,1);
}
int main(void){
    uint8_t *ram=(uint8_t *)malloc(SIZE),*before=(uint8_t *)malloc(SIZE);DreadzoneInitialPoolRequest q;unsigned i;
    if(!ram||!before){free(ram);free(before);return 1;}
    setup(ram,&q,3,2,0);CHECK(dreadzone_initial_pool_0057B6B8(&q)==1);
    CHECK(read32(ram+0x222D14-BASE)==POOL&&read32(ram+0x21EADC-BASE)==3&&read32(ram+0x21EAD8-BASE)==3);
    CHECK(read32(ram+0x21EAE0-BASE)==2&&read32(ram+0x21EAE4-BASE)==3*256);
    for(i=0;i<7*256;i++)CHECK(ram[POOL-BASE+i]==0);
    CHECK(ram[POOL-BASE-1]==0xA5&&ram[POOL-BASE+7*256]==0xA5);
    for(i=0;i<0x180;i++)CHECK(ram[0x247A18-BASE+i]==0);
    CHECK(ram[0x247A18-BASE-1]==0xA5&&ram[0x247B98-BASE]==0xA5);
    setup(ram,&q,0,0,0);put(ram,0x21EAC4,0);put(ram,0x21EAD4,0);
    CHECK(dreadzone_initial_pool_0057B6B8(&q)==1&&read32(ram+POOL-BASE)==0&&ram[POOL-BASE+4]==0xA5);
    setup(ram,&q,0x00FFFFFF,0,0);put(ram,0x21EAC4,0);put(ram,0x21EAD4,0);
    CHECK(dreadzone_initial_pool_0057B6B8(&q)==1&&read32(ram+POOL-BASE)==0&&ram[POOL-BASE+4]==0xA5);
    setup(ram,&q,0x00800000,0,0);put(ram,0x21EAC4,0);put(ram,0x21EAD4,0);memcpy(before,ram,SIZE);
    CHECK(dreadzone_initial_pool_0057B6B8(&q)==-2&&!memcmp(before,ram,SIZE));
    setup(ram,&q,1,0,0);q.pool_pointer_guest=0x21EAD8;memcpy(before,ram,SIZE);
    CHECK(dreadzone_initial_pool_0057B6B8(&q)==-2&&!memcmp(before,ram,SIZE));
    setup(ram,&q,1,0,0);put(ram,0x21EAC4,0);put(ram,0x21EAD4,0);
    {
        DreadzoneInitialPoolRequest *alias=(DreadzoneInitialPoolRequest *)(ram+POOL-BASE);
        memcpy(alias,&q,sizeof(q));CHECK(dreadzone_initial_pool_0057B6B8(alias)==1);
        for(i=0;i<sizeof(q);i++)CHECK(ram[POOL-BASE+i]==0);
    }
    setup(ram,&q,1,0,0);q.header_guest=HEADER+1;memcpy(before,ram,SIZE);
    CHECK(dreadzone_initial_pool_0057B6B8(&q)==-2&&!memcmp(before,ram,SIZE));
    setup(ram,&q,1,0,0);q.gp=GP+1;memcpy(before,ram,SIZE);
    CHECK(dreadzone_initial_pool_0057B6B8(&q)==-2&&!memcmp(before,ram,SIZE));
    setup(ram,&q,1,0,0);q.guest_base=0x6FFFF000;q.storage_bytes=0x3000;
    CHECK(dreadzone_initial_pool_0057B6B8(&q)==-1&&!memcmp(before,ram,SIZE));
    CHECK(dreadzone_initial_pool_0057B6B8(NULL)==-1);
    free(before);free(ram);printf("initial pool checks: %u\n",checks);return 0;
}
