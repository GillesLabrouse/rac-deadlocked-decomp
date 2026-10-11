#include "dreadzone_actor_transform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum {BASE=0x200000,BYTES=0x100000,ACTOR=0x2A0000};
static uint8_t *ram,*before,scratch[0x4000],old_scratch[0x4000],vu[0x1000];
static unsigned checks,failures;
static void check(int value,const char *message){checks++;if(!value){failures++;fprintf(stderr,"FAIL: %s\n",message);}}
static void put(uint32_t a,uint32_t x){memcpy(ram+a-BASE,&x,4);}
static uint32_t get(uint32_t a){uint32_t x;memcpy(&x,ram+a-BASE,4);return x;}
static void number(uint32_t a,float x){memcpy(ram+a-BASE,&x,4);}
static float value(uint32_t a){float x;memcpy(&x,ram+a-BASE,4);return x;}
static DreadzoneActorTransform setup(void){
    DreadzoneActorTransform q={ram,BASE,BYTES,ACTOR,1,scratch,sizeof(scratch),vu,sizeof(vu)};
    memset(ram,0,BYTES);memset(scratch,0x39,sizeof(scratch));memset(vu,0,sizeof(vu));
    number(ACTOR+0x10,1);number(ACTOR+0x14,2);number(ACTOR+0x18,3);number(ACTOR+0x70,0.5f);
    number(ACTOR+0x80,1);number(ACTOR+0x84,2);number(ACTOR+0x88,3);number(ACTOR+0x8C,4);
    put(ACTOR+0xA4,0xFFFFFFFF);put(0x222D14,ACTOR);return q;
}
static void refuse(DreadzoneActorTransform *q,int status,const char *message){
    memcpy(before,ram,BYTES);memcpy(old_scratch,scratch,sizeof(scratch));
    check(dreadzone_actor_transform(q)==status,message);check(!memcmp(ram,before,BYTES),"refusal preserves all RAM");check(!memcmp(scratch,old_scratch,sizeof(scratch)),"refusal preserves full SPR");
}
int main(void){
    DreadzoneActorTransform q;unsigned i;
    ram=malloc(BYTES);before=malloc(BYTES);if(!ram||!before){free(ram);free(before);return 2;}
    q=setup();check(dreadzone_actor_transform(&q)==1,"complete transform succeeds");
    check(value(ACTOR)==1024.5f&&value(ACTOR+4)==2049.0f&&value(ACTOR+8)==3073.5f&&value(ACTOR+12)==2,"actual scaled vector and preserved radius");
    check((get(ACTOR+0x94)&65535)==1&&get(ACTOR+0xA4)==0xFFFFFFFF,"generation advances without volume");
    check(value(ACTOR+0xC0)==1&&value(ACTOR+0xD4)==1&&value(ACTOR+0xE8)==1,"computed basis identity");
    q=setup();put(ACTOR+0x98,1);check(dreadzone_actor_transform(&q)==1&&get(ACTOR+0xA4)==0,"spatial membership is composed");
    check(ram[0x26FF22-BASE]==1&&ram[0x28BF20-BASE]==1,"first cell and bitmap published");
    memcpy(before,ram,BYTES);check(dreadzone_actor_transform(&q)==1,"repeated same-cell transform succeeds");
    check(!memcmp(before+0x26FF20-BASE,ram+0x26FF20-BASE,0x10000),"same-cell grid and blocks stay unchanged");
    q=setup();ram[ACTOR-BASE+0x20]=0xFF;memcpy(before,ram,BYTES);check(dreadzone_actor_transform(&q)==0&&!memcmp(before,ram,BYTES),"negative actor skips with no writes");
    q=setup();put(ACTOR+0x94,32767);check(dreadzone_actor_transform(&q)==1&&(get(ACTOR+0x94)&65535)==32767,"signed generation saturates");
    q=setup();q.actor++;refuse(&q,-3,"unaligned actor refuses");
    q=setup();put(ACTOR+0xFC,0x80000000);refuse(&q,-3,"computed basis requires positive-zero W");
    q=setup();put(ACTOR+0x50,ACTOR+0x100);refuse(&q,-3,"unclosed blend list refuses");
    q=setup();put(ACTOR+0x70,0x7F800000);refuse(&q,-3,"infinite scale refuses");
    q=setup();ram[ACTOR-BASE+0x56]=1;refuse(&q,-2,"changed animation needs mapped current vector");
    q=setup();put(ACTOR+0x98,1);q.actor_pool_count=0;refuse(&q,-3,"missing pool contract refuses after private vector calculation");
    q=setup();put(ACTOR+0x98,1);put(ACTOR+0xA4,0x01010101);refuse(&q,-3,"missing old membership refuses atomically");
    q=setup();put(ACTOR+0x98,1);memset(ram+0x28BF20-BASE,255,0xC0);refuse(&q,-3,"exhausted spatial bitmap refuses atomically");
    q=setup();q.scratch=ram;refuse(&q,-1,"overlapping backing refuses");
    q=setup();q.guest_base=0x70000000;q.actor=q.guest_base;refuse(&q,-1,"guest RAM cannot shadow scratchpad");
    q=setup();q.actor=BASE+BYTES;refuse(&q,-2,"missing actor span refuses");
    q=setup();put(ACTOR+0x98,1);ram[ACTOR-BASE+0x55]=0x20;put(ACTOR+0x58,0x2B0000);refuse(&q,-3,"late model failure rolls back earlier spatial mutations");
    q=setup();put(ACTOR+0x34,0x100);put(ACTOR+0xFC,0x7F800000);
    for(i=0;i<12;i++)number(ACTOR+0xC0+i*4,(i==0||i==5||i==10)?1.0f:0.0f);
    number(ACTOR+0xCC,2);number(ACTOR+0xDC,-3);number(ACTOR+0xEC,4);
    check(dreadzone_actor_transform(&q)==1&&value(ACTOR+0xCC)==2&&value(ACTOR+0xDC)==-3&&value(ACTOR+0xEC)==4,"existing basis preserves W and never consumes rotation W");
    q=setup();memcpy(ram+ACTOR-BASE+0xA0,&q,sizeof(q));check(dreadzone_actor_transform((const DreadzoneActorTransform *)(ram+ACTOR-BASE+0xA0))==1,"borrowed descriptor captured before basis stores");
    free(ram);free(before);printf("%u checks, %u failures\n",checks,failures);return failures?1:0;
}
