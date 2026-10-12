#include "dreadzone_object_predicate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BASE 0x00200000u
#define SIZE 0x00060000u
#define PACKET 0x00240000u
#define OBJECT 0x00241000u
#define LOWER_ADDR 0x00222D14u
#define UPPER_ADDR 0x00222D30u
#define MODE_ADDR 0x0021E824u
static uint8_t ram[SIZE], before[SIZE];
static unsigned checks;
static void put8(uint32_t a,uint8_t x){ram[a-BASE]=x;}
static void put16(uint32_t a,uint16_t x){ram[a-BASE]=(uint8_t)x;ram[a-BASE+1]=(uint8_t)(x>>8);}
static void put32(uint32_t a,uint32_t x){put16(a,(uint16_t)x);put16(a+2,(uint16_t)(x>>16));}
static void reset(void){memset(ram,0,sizeof(ram));put32(LOWER_ADDR,0x00230000u);put32(UPPER_ADDR,0x00250000u);put32(MODE_ADDR,0);}
static void expect_result(uint32_t packet,int expected){
    DreadzoneObjectPredicateRequest q={ram,BASE,SIZE,packet};
    DreadzoneObjectPredicateInfo info={0xA5A5A5A5u};
    int status;memcpy(before,ram,SIZE);
    status=dreadzone_object_predicate_00596E48(&q,&info);
    if(status!=DREADZONE_OBJECT_PREDICATE_COMPLETE||info.original_return!=(uint32_t)expected||memcmp(before,ram,SIZE)!=0){
        printf("case failed packet=%08X expected=%d status=%d result=%u\\n",packet,expected,status,info.original_return);exit(1);
    }checks++;
}
static void expect_unsupported(DreadzoneObjectPredicateRequest *q,uint32_t sentinel){
    DreadzoneObjectPredicateInfo info={sentinel};memcpy(before,q->storage,q->storage_bytes);
    if(dreadzone_object_predicate_00596E48(q,&info)!=DREADZONE_OBJECT_PREDICATE_UNSUPPORTED||info.original_return!=sentinel||memcmp(before,q->storage,q->storage_bytes)!=0){puts("atomic refusal failed");exit(1);}checks++;
}
int main(void){
    reset();expect_result(0,0);
    reset();put32(LOWER_ADDR,PACKET);put16(PACKET+0xBC,0x01F4);expect_result(PACKET,1);
    reset();put32(UPPER_ADDR,PACKET);put16(PACKET+0xBC,0x021C);expect_result(PACKET,1);
    reset();put32(LOWER_ADDR,PACKET+4);expect_result(PACKET,0);
    reset();put32(UPPER_ADDR,PACKET-4);expect_result(PACKET,0);
    reset();put16(PACKET+0xBC,0x01F3);expect_result(PACKET,0);
    reset();put16(PACKET+0xBC,0x01F4);expect_result(PACKET,1);
    reset();put16(PACKET+0xBC,0x021C);expect_result(PACKET,1);
    reset();put16(PACKET+0xBC,0x021D);expect_result(PACKET,0);
    reset();put16(PACKET+0xBC,0xFFFF);expect_result(PACKET,0);
    reset();{DreadzoneObjectPredicateRequest q={ram,BASE,SIZE,PACKET+1};DreadzoneObjectPredicateInfo info={0xCAFEBABEu};put16(PACKET+1+0xBC,0x01F4);memcpy(before,ram,SIZE);if(dreadzone_object_predicate_00596E48(&q,&info)!=DREADZONE_OBJECT_PREDICATE_UNSUPPORTED||info.original_return!=0xCAFEBABEu||memcmp(before,ram,SIZE)!=0){puts("unaligned halfword refusal failed");return 1;}checks++;}
    reset();put32(MODE_ADDR,1);put16(PACKET+0xBC,0x01F4);expect_result(PACKET,0); /* mode-1 null object skips halfword */
    reset();put32(MODE_ADDR,1);put32(PACKET+0x24,OBJECT);put8(OBJECT+0x46,9);put16(PACKET+0xBC,0xFFFF);expect_result(PACKET,1);
    reset();put32(MODE_ADDR,1);put32(PACKET+0x24,OBJECT);put8(OBJECT+0x46,8);put16(PACKET+0xBC,0x021C);expect_result(PACKET,0);
    reset();put32(MODE_ADDR,2);put32(PACKET+0x24,OBJECT);put8(OBJECT+0x46,9);put16(PACKET+0xBC,0x01F3);expect_result(PACKET,0); /* non-one mode ignores object */
    {
        uint32_t truncated=PACKET-BASE+0x28u;
        DreadzoneObjectPredicateRequest q={ram,BASE,truncated,PACKET};
        reset();put32(MODE_ADDR,1);put32(PACKET+0x24,0);q.storage_bytes=truncated;
        /* Omitted +BC is not reached for mode-1 null; result must be complete false. */
        {DreadzoneObjectPredicateInfo info={0x76543210u};memcpy(before,ram,truncated);
         if(dreadzone_object_predicate_00596E48(&q,&info)!=DREADZONE_OBJECT_PREDICATE_COMPLETE||info.original_return!=0||memcmp(before,ram,truncated)!=0){puts("lazy null path failed");return 1;}checks++;}
        reset();put32(MODE_ADDR,0);put32(PACKET+0x24,0);q.storage_bytes=truncated;
        expect_unsupported(&q,0x12345678u); /* fallback reaches unmapped +BC */
        reset();put32(MODE_ADDR,1);put32(PACKET+0x24,BASE+0x100u);put8(BASE+0x146u,9);q.storage_bytes=truncated;
        {DreadzoneObjectPredicateInfo info={0x76543210u};memcpy(before,ram,truncated);
         if(dreadzone_object_predicate_00596E48(&q,&info)!=DREADZONE_OBJECT_PREDICATE_COMPLETE||info.original_return!=1||memcmp(before,ram,truncated)!=0){puts("lazy kind-9 path failed");return 1;}checks++;}
        reset();put32(MODE_ADDR,1);put32(PACKET+0x24,BASE+0x100u);put8(BASE+0x146u,8);q.storage_bytes=truncated;
        {DreadzoneObjectPredicateInfo info={0x76543210u};memcpy(before,ram,truncated);
         if(dreadzone_object_predicate_00596E48(&q,&info)!=DREADZONE_OBJECT_PREDICATE_COMPLETE||info.original_return!=0||memcmp(before,ram,truncated)!=0){puts("lazy kind-not-nine path failed");return 1;}checks++;}
    }
    {
        uint32_t n=LOWER_ADDR-BASE+4u;
        DreadzoneObjectPredicateRequest q={ram,BASE,n,PACKET};
        reset();put32(LOWER_ADDR,PACKET+1);q.storage_bytes=n;
        {DreadzoneObjectPredicateInfo info={77};memcpy(before,ram,n);
         if(dreadzone_object_predicate_00596E48(&q,&info)!=DREADZONE_OBJECT_PREDICATE_COMPLETE||info.original_return!=0||memcmp(before,ram,n)!=0){puts("lower early gate failed");return 1;}checks++;}
    }
    {
        DreadzoneObjectPredicateRequest q={ram,BASE,SIZE,PACKET};
        DreadzoneObjectPredicateInfo info={99};
        if(dreadzone_object_predicate_00596E48(NULL,&info)!=DREADZONE_OBJECT_PREDICATE_INVALID||info.original_return!=99){puts("null request guard failed");return 1;}checks++;
        if(dreadzone_object_predicate_00596E48(&q,(DreadzoneObjectPredicateInfo *)&q)!=DREADZONE_OBJECT_PREDICATE_INVALID||q.packet_guest!=PACKET){puts("overlap guard failed");return 1;}checks++;
    }
    printf("%u predicate and read-only cases passed\n",checks);return 0;
}
