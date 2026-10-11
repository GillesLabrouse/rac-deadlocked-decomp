#include "dreadzone_guest_model_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum {BASE=0x200000,SIZE=0x400000,STATE=0x222D78,DESC=0x300000,
      MATRIX=0x400000,ACTOR=0x500000,DEF=0x580000,FRESH=0x590000,FRESH_DEF=0x591000};
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"guest model pool failed line %d\n",__LINE__);free(before);free(ram);return 1;}}while(0)
static uint32_t get(const uint8_t *ram,uint32_t a){const uint8_t *p=ram+a-BASE;return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void put(uint8_t *ram,uint32_t a,uint32_t v){uint8_t *p=ram+a-BASE;p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static void setup(uint8_t *ram,DreadzoneGuestModelPool *q,uint32_t n){
    uint32_t i,k;
    memset(ram,0xA5,SIZE);q->storage=ram;q->guest_base=BASE;q->bytes=SIZE;
    q->gp=0x22597F;q->actor=FRESH;q->operation=DREADZONE_GUEST_MODEL_ALLOCATE;
    put(ram,STATE,DESC);put(ram,STATE+4,MATRIX);put(ram,STATE+8,MATRIX+n*64);
    put(ram,STATE+12,0x3094-n);put(ram,STATE+16,0);put(ram,STATE+20,n);
    put(ram,STATE+24,0);put(ram,STATE+28,0x7FFFFFFF);
    put(ram,STATE+32,0x13579BDF);put(ram,STATE+36,0x2468ACE0);
    for(i=0;i<n;i++){
        uint8_t *d=ram+DESC+i*8-BASE;
        put(ram,DESC+i*8,ACTOR+i*256);d[4]=(uint8_t)i;d[5]=(uint8_t)(i>>8);d[6]=1;d[7]=0;
        put(ram,ACTOR+i*256+0x24,DEF+i*16);put(ram,ACTOR+i*256+0x58,MATRIX+i*64);
        ram[DEF+i*16+8-BASE]=1;
        for(k=0;k<64;k++)ram[MATRIX+i*64+k-BASE]=(uint8_t)(i*7+k);
    }
    put(ram,FRESH+0x24,FRESH_DEF);put(ram,FRESH+0x58,0xDEADBEEF);ram[FRESH_DEF+8-BASE]=2;
}
int main(void){
    uint8_t *ram=(uint8_t *)malloc(SIZE),*before=(uint8_t *)malloc(SIZE);
    DreadzoneGuestModelPool q;uint32_t result=0xD15EA5ED,i,k;
    if(!ram||!before){free(ram);free(before);return 1;}
    setup(ram,&q,0);CHECK(dreadzone_guest_model_pool_apply(&q,&result)==1&&result==MATRIX);
    CHECK(get(ram,FRESH+0x58)==MATRIX&&get(ram,STATE+8)==MATRIX+128);
    CHECK(get(ram,STATE+12)==0x3092&&get(ram,STATE+20)==1);
    CHECK(get(ram,DESC)==FRESH&&ram[DESC+6-BASE]==2&&ram[DESC+7-BASE]==0);
    q.operation=DREADZONE_GUEST_MODEL_RELEASE;result=0xD15EA5ED;
    CHECK(dreadzone_guest_model_pool_apply(&q,&result)==1&&result==0xD15EA5ED);
    CHECK(get(ram,FRESH+0x58)==MATRIX&&ram[DESC+7-BASE]==1);
    CHECK(get(ram,STATE+16)==2&&get(ram,STATE+24)==1&&get(ram,STATE+28)==0);
    q.operation=DREADZONE_GUEST_MODEL_COMPACT;memcpy(before,ram,SIZE);
    CHECK(dreadzone_guest_model_pool_apply(&q,NULL)==1&&!memcmp(before,ram,SIZE));
    /* Descriptor pressure forces forward overlapping movement even with many
     * free matrix records. Verify every survivor and all matrix bytes. */
    setup(ram,&q,1023);q.actor=ACTOR;q.operation=DREADZONE_GUEST_MODEL_RELEASE;
    CHECK(dreadzone_guest_model_pool_apply(&q,NULL)==1);
    q.operation=DREADZONE_GUEST_MODEL_COMPACT;
    CHECK(dreadzone_guest_model_pool_apply(&q,NULL)==1);
    CHECK(get(ram,STATE+20)==1022&&get(ram,STATE+12)==0x3094-1022);
    CHECK(get(ram,STATE+8)==MATRIX+1022*64&&get(ram,STATE+16)==0);
    CHECK(get(ram,STATE+24)==0&&get(ram,STATE+28)==0x7FFFFFFF);
    CHECK(get(ram,STATE+32)==0x13579BDF&&get(ram,STATE+36)==0x2468ACE0);
    CHECK(get(ram,ACTOR+0x58)==MATRIX);
    for(i=0;i<1022;i++){
        CHECK(get(ram,ACTOR+(i+1)*256+0x58)==MATRIX+i*64);
        CHECK(get(ram,DESC+i*8)==ACTOR+(i+1)*256);
        CHECK(ram[DESC+i*8+4-BASE]==(uint8_t)i&&ram[DESC+i*8+5-BASE]==(uint8_t)(i>>8));
        for(k=0;k<64;k++)CHECK(ram[MATRIX+i*64+k-BASE]==(uint8_t)((i+1)*7+k));
    }
    /* All unsuccessful calls preserve RAM and result, including malformed
     * counters, reallocation, absent mappings, and borrowed output buffers. */
    setup(ram,&q,1);q.actor=ACTOR;memcpy(before,ram,SIZE);
    CHECK(dreadzone_guest_model_pool_apply(&q,&result)==-3&&!memcmp(before,ram,SIZE));
    CHECK(result==0xD15EA5ED);
    setup(ram,&q,1);put(ram,STATE+12,1);memcpy(before,ram,SIZE);
    CHECK(dreadzone_guest_model_pool_apply(&q,&result)==-3&&!memcmp(before,ram,SIZE));
    setup(ram,&q,1);q.bytes=0x100000;memcpy(before,ram,SIZE);
    CHECK(dreadzone_guest_model_pool_apply(&q,&result)==-2&&!memcmp(before,ram,SIZE));
    setup(ram,&q,1);memcpy(before,ram,SIZE);
    CHECK(dreadzone_guest_model_pool_apply(&q,(uint32_t *)(ram+128))==-1&&!memcmp(before,ram,SIZE));
    CHECK(dreadzone_guest_model_pool_apply(&q,&q.actor)==-1&&!memcmp(before,ram,SIZE));
    q.gp++;CHECK(dreadzone_guest_model_pool_apply(&q,&result)==-1&&!memcmp(before,ram,SIZE));
    /* No borrowed request or legacy binding survives a transaction. */
    setup(ram,&q,0);
    {
        DreadzoneGuestModelPool *alias=(DreadzoneGuestModelPool *)(ram+0x3FE000);
        memcpy(alias,&q,sizeof(q));CHECK(dreadzone_guest_model_pool_apply(alias,&result)==1&&result==MATRIX);
    }
    setup(ram,&q,3);CHECK(dreadzone_guest_model_pool_apply(&q,&result)==1&&result==MATRIX+192);
    /* Zero-count release must read222D7C in its original branch delay slot.
     * The null-matrix and null-definition paths have NOP delay slots. */
    memset(ram,0xA5,SIZE);q.storage=ram;q.guest_base=ACTOR;q.bytes=512;q.actor=ACTOR;
    q.gp=0x22597F;q.operation=DREADZONE_GUEST_MODEL_RELEASE;
    memset(ram+0x58,0,4);CHECK(dreadzone_guest_model_pool_apply(&q,NULL)==1);
    ram[0x58]=1;memset(ram+0x24,0,4);CHECK(dreadzone_guest_model_pool_apply(&q,NULL)==1);
    ram[0x24]=0;ram[0x25]=1;ram[0x26]=0x50;ram[0x27]=0;ram[0x108]=0;
    memcpy(before,ram,SIZE);CHECK(dreadzone_guest_model_pool_apply(&q,NULL)==-2&&!memcmp(before,ram,SIZE));
    CHECK(dreadzone_guest_model_pool_apply(NULL,NULL)==-1);
    free(before);free(ram);printf("guest model pool checks: %u\n",checks);return 0;
}
