#include "dreadzone_guest_actor_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { BASE=0x200000, BYTES=0x100000, ACTOR=0x280000, DESC=0x290000, PAYLOAD=0x2A0000 };
static uint8_t *ram,*before,scratch[0x4000],old_scratch[0x4000],vu[0x1000];
static unsigned checks,failures;
static void check(int ok,const char *message){checks++;if(!ok){failures++;fprintf(stderr,"FAIL: %s\n",message);}}
static void put(uint32_t a,uint32_t v){uint8_t *p=ram+a-BASE;p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static uint32_t get(uint32_t a){const uint8_t *p=ram+a-BASE;return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static DreadzoneGuestActorAllocation setup(void){
    DreadzoneGuestActorAllocation q={ram,BASE,BYTES,0x22597F,0x2000,3,scratch,sizeof(scratch),vu,sizeof(vu)};
    memset(ram,0,BYTES);memset(scratch,0x39,sizeof(scratch));memset(vu,0,sizeof(vu));
    put(0x222D2C,ACTOR);put(0x222D30,ACTOR+0x200);put(0x222D10,2);put(0x21DDB8,7);
    put(0x222D3C,PAYLOAD);put(0x222D40,8);put(0x222D44,DESC);put(0x222D50,DESC);put(0x222D54,DESC+32);
    ram[ACTOR-BASE+0x20]=0xFF;ram[ACTOR-BASE+0x120]=0xFF;put(ACTOR+0xA0,7);put(ACTOR+0x1A0,7);
    memset(ram+PAYLOAD-BASE,0xA5,512);return q;
}
static void refuse(DreadzoneGuestActorAllocation *q,int expected,const char *message){
    uint32_t out=0xDEADBEEF;int status;
    memcpy(before,ram,BYTES);memcpy(old_scratch,scratch,sizeof(scratch));status=dreadzone_guest_actor_allocate(q,&out);
    check(status==expected,message);check(out==0xDEADBEEF,"refusal preserves result");
    check(!memcmp(before,ram,BYTES),"refusal preserves all RAM");
    check(!memcmp(old_scratch,scratch,sizeof(scratch)),"refusal preserves all SPR");
}
int main(void){
    DreadzoneGuestActorAllocation q;uint32_t out=0xDEADBEEF;unsigned i;
    ram=malloc(BYTES);before=malloc(BYTES);if(!ram||!before){free(ram);free(before);return 2;}
    q=setup();check(dreadzone_guest_actor_allocate(&q,&out)==1&&out==ACTOR,"first free actor allocated");
    check(get(ACTOR+0xAC)==PAYLOAD,"payload link is guest address");
    check(get(0x222D10)==1&&get(0x222D2C)==ACTOR,"counter decremented; occupied boundary not supplied");
    check(get(PAYLOAD)==0&&ram[PAYLOAD-BASE+4]==0xA5,"three-byte clear rounds to word and preserves padding");
    check(ram[ACTOR-BASE+0x120]==0xFF,"FF preserves following free marker");
    q.payload_bytes=65;check(dreadzone_guest_actor_allocate(&q,&out)==1&&out==ACTOR+0x100,"allocated actor skipped on next call");
    check(get(out+0xAC)==PAYLOAD+64&&get(0x222D10)==0,"tail allocation and zero counter");
    for(i=0;i<68;i++)check(ram[PAYLOAD-BASE+64+i]==0,"word-rounded second payload clear");
    check(ram[PAYLOAD-BASE+132]==0xA5,"allocation padding stays intact");
    memcpy(before,ram,BYTES);memcpy(old_scratch,scratch,sizeof(scratch));out=0xDEADBEEF;
    check(dreadzone_guest_actor_allocate(&q,&out)==0&&out==0,"ordinary exhaustion returns zero");
    check(!memcmp(before,ram,BYTES)&&!memcmp(old_scratch,scratch,sizeof(scratch)),"ordinary exhaustion preserves memory");
    q=setup();q.object_class=0x10000;refuse(&q,-1,"invalid class refuses");
    q=setup();put(0x222D2C,0x222D00);put(0x222D30,0x222E00);refuse(&q,-3,"pool/control overlap refuses before class initialization");
    q=setup();put(0x222D44,BASE+BYTES-32);put(0x222D50,BASE+BYTES-32);put(0x222D54,BASE+BYTES);
    refuse(&q,-2,"missing descriptor lookahead word refuses");
    q=setup();put(0x222D2C,BASE+BYTES-256);put(0x222D30,BASE+BYTES);
    ram[BYTES-256+0x20]=0xFF;put(BASE+BYTES-256+0xA0,7);refuse(&q,-2,"FF following byte must be mapped");
    ram[BYTES-256+0x20]=0xFE;check(dreadzone_guest_actor_allocate(&q,&out)==1&&out==BASE+BYTES-256,"FE last actor needs no following byte");
    q=setup();memcpy(ram+ACTOR-BASE+0xC0,&q,sizeof(q));
    check(dreadzone_guest_actor_allocate((const DreadzoneGuestActorAllocation *)(ram+ACTOR-BASE+0xC0),&out)==1&&out==ACTOR,"borrowed actor request captured before filler");
    free(ram);free(before);printf("%u checks, %u failures\n",checks,failures);return failures?1:0;
}
