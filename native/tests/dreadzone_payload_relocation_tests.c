#include "dreadzone_payload_relocation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { BASE=0x200000, BYTES=0x100000, ACTOR=0x2C0000, TABLE=0x2B0000, SOURCE=0x2B8000, DEST=0x2C8000, CELL=0x2A0000 };
static uint8_t *ram,*before;static unsigned checks,failures;
#define CHECK(x) do{checks++;if(!(x)){failures++;fprintf(stderr,"FAIL line%d: %s\n",__LINE__,#x);}}while(0)
static void put(uint32_t a,uint32_t x){memcpy(ram+a-BASE,&x,4);}
static uint32_t get(uint32_t a){uint32_t x;memcpy(&x,ram+a-BASE,4);return x;}
static DreadzonePayloadRelocation setup(uint32_t count){
    DreadzonePayloadRelocation q={ram,BASE,BYTES,TABLE,SOURCE,CELL,0,0,8};uint32_t i;
    memset(ram,0,BYTES);put(0x222D14,ACTOR);put(0x222D2C,ACTOR+count*256);put(CELL,DEST+32);
    put(TABLE,0);put(TABLE+4,48);put(TABLE+8,256);put(TABLE+12,32);
    for(i=0;i<512;i++)ram[SOURCE+i-BASE]=(uint8_t)(i*7+3);
    for(i=0;i<count&&i<2;i++){put(ACTOR+i*256+0xBC,0x23F7);put(ACTOR+i*256+0xAC,i);}
    return q;
}
static void refuse(DreadzonePayloadRelocation *q,int status){
    memcpy(before,ram,BYTES);CHECK(dreadzone_payload_relocate(q)==status);CHECK(!memcmp(before,ram,BYTES));
}
int main(void){
    DreadzonePayloadRelocation q;ram=malloc(BYTES);before=malloc(BYTES);if(!ram||!before){free(ram);free(before);return 2;}
    q=setup(2);CHECK(dreadzone_payload_relocate(&q)==1);
    CHECK(get(ACTOR+0xAC)==DEST+32&&get(ACTOR+256+0xAC)==DEST+80);
    CHECK(get(TABLE)==DEST+32&&get(TABLE+8)==DEST+80&&get(CELL)==DEST+128);
    CHECK(!memcmp(ram+DEST+32-BASE,ram+SOURCE-BASE,48)&&!memcmp(ram+DEST+80-BASE,ram+SOURCE+256-BASE,32));
    CHECK(ram[DEST+112-BASE]==0);
    q=setup(1);q.actor_begin=ACTOR;q.actor_end=ACTOR+256;put(0x222D14,0);put(0x222D2C,0);CHECK(dreadzone_payload_relocate(&q)==1&&get(ACTOR+0xAC)==DEST+32);
    q=setup(1);put(ACTOR+0xBC,0);CHECK(dreadzone_payload_relocate(&q)==1&&get(ACTOR+0xAC)==0&&get(TABLE)==0&&get(CELL)==DEST+64);
    q=setup(1);put(ACTOR+0xAC,0xFFFFFFFF);CHECK(dreadzone_payload_relocate(&q)==1&&get(ACTOR+0xAC)==0&&get(TABLE)==0);
    q=setup(0);q.actor_limit=0;CHECK(dreadzone_payload_relocate(&q)==1&&get(CELL)==DEST+64);
    q=setup(1);put(TABLE+4,16);CHECK(dreadzone_payload_relocate(&q)==1);CHECK(!memcmp(ram+DEST+32-BASE,ram+SOURCE-BASE,16)&&ram[DEST+48-BASE]==0);
    q=setup(2);put(ACTOR+256+0xAC,65536);refuse(&q,-3);
    q=setup(2);put(TABLE+12,0);refuse(&q,-3);
    q=setup(1);put(TABLE+4,17);refuse(&q,-3);
    q=setup(1);put(TABLE+4,0x100010);refuse(&q,-3);
    q=setup(1);put(TABLE,(BASE+BYTES-48)-SOURCE);refuse(&q,-2);
    q=setup(1);put(CELL,BASE+BYTES-16);refuse(&q,-2);
    q=setup(1);put(CELL,DEST+1);refuse(&q,-3);
    q=setup(1);put(TABLE,1);refuse(&q,-3);
    q=setup(1);q.actor_end=ACTOR+16;refuse(&q,-3);
    q=setup(1);q.actor_limit=0;refuse(&q,-3);
    q=setup(0);put(CELL,0x7FFFFFC1);refuse(&q,-3);
    q=setup(1);q.cursor_cell++;refuse(&q,-3);
    q=setup(1);q.bytes=0;refuse(&q,-1);
    q=setup(1);memcpy(ram+DEST+32-BASE,&q,sizeof(q));CHECK(dreadzone_payload_relocate((DreadzonePayloadRelocation *)(ram+DEST+32-BASE))==1);
    CHECK(dreadzone_payload_relocate(NULL)==-1);
    free(before);free(ram);printf("payload relocation: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
