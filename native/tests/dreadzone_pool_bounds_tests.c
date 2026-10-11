#include "dreadzone_pool_bounds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum {BASE=0x200000,SIZE=0x400000,CURSOR=0x5E0000,POOL=0x300000,GP=0x22597F};
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"pool bounds failed line %d\n",__LINE__);free(before);free(ram);return 1;}}while(0)
static uint32_t get(const uint8_t *ram,uint32_t a){const uint8_t *p=ram+a-BASE;return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void put(uint8_t *ram,uint32_t a,uint32_t v){uint8_t *p=ram+a-BASE;p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static void setup(uint8_t *ram,DreadzonePoolBoundsRequest *q,uint32_t reserve){
 memset(ram,0xA5,SIZE);q->storage=ram;q->guest_base=BASE;q->storage_bytes=SIZE;q->cursor_guest=CURSOR;q->gp=GP;
 put(ram,GP-0x6EA7,3);put(ram,GP-0x6EBB,1);put(ram,GP-0x6EAB,2);put(ram,GP-0x6E9F,reserve);put(ram,0x222D14,POOL);put(ram,CURSOR,POOL);
}
int main(void){
 uint8_t *ram=(uint8_t *)malloc(SIZE),*before=(uint8_t *)malloc(SIZE);DreadzonePoolBoundsRequest q;unsigned i;
 if(!ram||!before){free(ram);free(before);return 1;}
 setup(ram,&q,4);CHECK(dreadzone_pool_bounds_0057BAC0(&q)==1);
 CHECK(get(ram,0x21EC00)==POOL&&get(ram,0x21EC04)==POOL+0x300);
 CHECK(get(ram,0x21EC08)==POOL+0x300&&get(ram,0x21EC0C)==POOL+0x300);
 CHECK(get(ram,0x21EC10)==POOL+0x400&&get(ram,0x222D2C)==POOL+0x400);
 CHECK(ram[POOL+0x420-BASE]==0xFF&&ram[POOL+0x421-BASE]==0xA5);
 CHECK(get(ram,0x222D30)==POOL+0x900&&get(ram,0x21EAEC)==0x300);
 CHECK(get(ram,0x243790)==POOL+0xA00&&get(ram,0x222D3C)==POOL+0xD00);
 CHECK(get(ram,0x222D40)==24&&get(ram,0x222D44)==POOL+0x1300);
 CHECK(get(ram,0x222D50)==POOL+0x1300&&get(ram,0x222D54)==POOL+0x1318);
 CHECK(get(ram,0x222D48)==0&&get(ram,0x222D4C)==0&&get(ram,CURSOR)==POOL+0x1340);
 for(i=0;i<24;i++)CHECK(ram[POOL+0x1300-BASE+i]==0);
 CHECK(ram[POOL+0x12FF-BASE]==0xA5&&ram[POOL+0x1318-BASE]==0xA5);
 setup(ram,&q,0);put(ram,GP-0x6EA7,0);put(ram,GP-0x6EBB,0);put(ram,GP-0x6EAB,0);
 CHECK(dreadzone_pool_bounds_0057BAC0(&q)==1&&get(ram,POOL)==0&&ram[POOL-BASE+4]==0xA5);
 setup(ram,&q,0x20000000);put(ram,GP-0x6EAB,0);memcpy(before,ram,SIZE);
 CHECK(dreadzone_pool_bounds_0057BAC0(&q)==-2&&!memcmp(before,ram,SIZE));
 setup(ram,&q,4);q.cursor_guest=CURSOR+1;memcpy(before,ram,SIZE);
 CHECK(dreadzone_pool_bounds_0057BAC0(&q)==-2&&!memcmp(before,ram,SIZE));
 setup(ram,&q,4);q.gp=GP+1;memcpy(before,ram,SIZE);
 CHECK(dreadzone_pool_bounds_0057BAC0(&q)==-2&&!memcmp(before,ram,SIZE));
 setup(ram,&q,4);put(ram,0x222D14,BASE+SIZE);memcpy(before,ram,SIZE);
 CHECK(dreadzone_pool_bounds_0057BAC0(&q)==-2&&!memcmp(before,ram,SIZE));
 setup(ram,&q,4);
 {
  DreadzonePoolBoundsRequest *alias=(DreadzonePoolBoundsRequest *)(ram+POOL+0x1300-BASE);
  memcpy(alias,&q,sizeof(q));CHECK(dreadzone_pool_bounds_0057BAC0(alias)==1);
  for(i=0;i<24;i++)CHECK(ram[POOL+0x1300-BASE+i]==0);
 }
 setup(ram,&q,8);put(ram,GP-0x6EA7,0);put(ram,GP-0x6EBB,0);put(ram,GP-0x6EAB,0);put(ram,CURSOR,0x222D40-0x1000);
 CHECK(dreadzone_pool_bounds_0057BAC0(&q)==1&&get(ram,0x222D54)==0&&get(ram,CURSOR)==0);
 setup(ram,&q,4);q.guest_base=0x6FFFF000;q.storage_bytes=0x3000;memcpy(before,ram,SIZE);
 CHECK(dreadzone_pool_bounds_0057BAC0(&q)==-1&&!memcmp(before,ram,SIZE));
 CHECK(dreadzone_pool_bounds_0057BAC0(NULL)==-1);
 free(before);free(ram);printf("pool bounds checks: %u\n",checks);return 0;
}
