#include "dreadzone_placed_actors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { BASE=0x200000, BYTES=0x100000, HEADER=0x2A0000, CELL=0x2A1000,
       ACTOR=0x2C0000, DEF=0x2B0000, AUX=0x2B0080, CLASS=4249 };
static uint8_t *ram,*before,spr[0x4000],old_spr[0x4000],vu[0x1000];
static unsigned checks,failures;
#define CHECK(x) do {checks++;if(!(x)){failures++;fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);}}while(0)
static void put(uint32_t a,uint32_t x){memcpy(ram+a-BASE,&x,4);}
static uint32_t get(uint32_t a){uint32_t x;memcpy(&x,ram+a-BASE,4);return x;}
static void number(uint32_t a,float f){memcpy(ram+a-BASE,&f,4);}
static float value(uint32_t a){float f;memcpy(&f,ram+a-BASE,4);return f;}
static DreadzonePlacedActors setup(uint32_t count){
    DreadzonePlacedActors q={ram,BASE,BYTES,0x22597F,HEADER,CELL,0,8,spr,sizeof(spr),vu,sizeof(vu)};
    uint32_t i;
    memset(ram,0,BYTES);memset(spr,0x93,sizeof(spr));memset(vu,0,sizeof(vu));
    put(0x220160,0x1170D0);put(0x222D14,ACTOR);put(0x21EAD8,2);put(0x21EADC,3);put(HEADER,count);
    ram[0x26B590+CLASS-BASE]=3;ram[0x26B2B0+6-BASE]=(uint8_t)(CLASS&255);
    ram[0x26B2B0+7-BASE]=(uint8_t)(CLASS>>8);put(0x26AA40+12,DEF);
    number(DEF+0x24,2.0f);put(DEF+0x48,AUX);
    number(AUX,1.0f);number(AUX+4,2.0f);number(AUX+8,3.0f);number(AUX+12,4.0f);ram[AUX+0x10-BASE]=17;
    for(i=0;i<count&&i<3;i++){
        uint32_t r=HEADER+16+i*0x70;
        put(r,0x71);put(r+4,0x12345678+i);put(r+8,0x1234ABCD+i);put(r+0xC,0x89ABCDEF);
        put(r+0x10,CLASS);number(r+0x14,3.0f);put(r+0x18,0xFEDC4321);put(r+0x1C,0x87654321);
        number(r+0x28,1.0f);number(r+0x2C,2.0f);number(r+0x30,3.0f);
        put(r+0x40,3);put(r+0x48,0x7F800000);put(r+0x4C,0xDEADBEEF);put(r+0x50,0x2E0000+i*0x100);
        put(r+0x58,0x20);put(r+0x5C,0xFFFF001A);put(r+0x60,0xABCD0017);put(r+0x64,0xABCDE026);put(r+0x68,0x12345678);
    }
    return q;
}
static void refuse(DreadzonePlacedActors *q,int status){
    memcpy(before,ram,BYTES);memcpy(old_spr,spr,sizeof(spr));
    int actual=dreadzone_placed_actors_load(q);if(actual!=status)fprintf(stderr,"refusal expected %d got %d secondflags %08X\n",status,actual,get(HEADER+16+0x70+0x58));CHECK(actual==status);CHECK(!memcmp(before,ram,BYTES));CHECK(!memcmp(old_spr,spr,sizeof(spr)));
}
int main(void){
    DreadzonePlacedActors q;uint32_t r=HEADER+16;
    ram=malloc(BYTES);before=malloc(BYTES);if(!ram||!before){free(ram);free(before);return 2;}
    q=setup(1);CHECK(dreadzone_placed_actors_load(&q)==1);CHECK(get(CELL)==0x243800);
    CHECK(value(ACTOR)==1030.0f&&value(ACTOR+4)==2060.0f&&value(ACTOR+8)==3090.0f&&value(ACTOR+12)==24.0f);
    CHECK((get(ACTOR+0xB0)&255)==0x78&&(get(ACTOR+0xB0)>>16)==0xABCD);
    CHECK(get(ACTOR+0xB4)==0x89ABCDEF&&get(ACTOR+0xAC)==0x2E0000);
    CHECK((get(ACTOR+0x30)&255)==0x21&&(get(ACTOR+0x30)>>16)==0x4321);
    CHECK(get(ACTOR+0x38)==0x12345678&&get(ACTOR+0x3C)==(0xFFFF001Au+(0xABCD0017u<<8)+(0xABCDE026u<<16)));
    CHECK((get(DEF+0x44)&65535)==0x20&&get(r+0x4C)==0xDEADBEEF);
    q=setup(3);q.mode=7;CHECK(dreadzone_placed_actors_load(&q)==1);
    CHECK((get(0x243804)>>16)==2&&(get(0x243808)&65535)==3&&(get(0x243808)>>16)==4);
    CHECK(get(ACTOR+2*256+0xAC)==0x2E0000&&get(ACTOR+4*256+0xAC)==0x2E0200);
    q=setup(0);put(CELL,0x243800);memcpy(before,ram,BYTES);CHECK(dreadzone_placed_actors_load(&q)==1&&!memcmp(before,ram,BYTES));
    q=setup(0);put(HEADER,0xFFFFFFFF);CHECK(dreadzone_placed_actors_load(&q)==1&&get(CELL)==0x243800);
    q=setup(1);q.gp=0;refuse(&q,-1);
    q=setup(1);q.header++;refuse(&q,-3);
    q=setup(1);q.header=BASE+BYTES;refuse(&q,-2);
    q=setup(1);q.output_cell=BASE+BYTES;refuse(&q,-2);
    q=setup(1);put(0x220160,0);refuse(&q,-3);
    q=setup(1);put(HEADER,1025);refuse(&q,-3);
    q=setup(1);q.mode=1;put(0x21EAD8,8);refuse(&q,-3);
    q=setup(1);q.mode=1;put(0x21EADC,65536);refuse(&q,-3);
    q=setup(1);put(0x222D14,ACTOR+1);refuse(&q,-3);
    q=setup(1);put(r+0x44,1);refuse(&q,-3);
    q=setup(1);put(r+0x14,0x7F800000);refuse(&q,-3);
    q=setup(2);put(r+0x70+0x44,1);refuse(&q,-3);
    q=setup(2);put(r+0x70+0x34,0x7FC00000);refuse(&q,-3);
    q=setup(1);put(0x222C50,DEF);put(DEF,0);refuse(&q,-3);
    q=setup(1);put(r+0x10,0x7D0);refuse(&q,-3);
    q=setup(1);q.scratch=ram;refuse(&q,-1);
    q=setup(1);q.vu_data=spr;refuse(&q,-1);
    q=setup(1);q.guest_base=0x70000000;refuse(&q,-1);
    q=setup(1);memcpy(ram+ACTOR-BASE,&q,sizeof(q));CHECK(dreadzone_placed_actors_load((DreadzonePlacedActors *)(ram+ACTOR-BASE))==1);
    q=setup(1);memcpy(spr,&q,sizeof(q));CHECK(dreadzone_placed_actors_load((DreadzonePlacedActors *)spr)==1);
    CHECK(dreadzone_placed_actors_load(NULL)==-1);
    free(before);free(ram);printf("placed loader: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
