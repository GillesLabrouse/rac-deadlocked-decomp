#include "dreadzone_player_record_transform.h"
#include "dreadzone_player_volume.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { BASE=0x200000, BYTES=0x100000, RECORD=0x2A0000, ACTOR=0x2C0000, HEADER=0x2B0000, DEF=0x2B1000 };
static uint8_t *ram,*before,spr[0x4000],old_spr[0x4000],vu[0x1000];
static unsigned checks,failures;
#define CHECK(x) do{checks++;if(!(x)){failures++;fprintf(stderr,"FAIL line%d: %s\n",__LINE__,#x);}}while(0)
static void put(uint32_t a,uint32_t x){memcpy(ram+a-BASE,&x,4);}
static uint32_t get(uint32_t a){uint32_t x;memcpy(&x,ram+a-BASE,4);return x;}
static void number(uint32_t a,float f){memcpy(ram+a-BASE,&f,4);}
static float value(uint32_t a){float f;memcpy(&f,ram+a-BASE,4);return f;}
static DreadzonePlayerRecordTransform setup(void){
    DreadzonePlayerRecordTransform q={ram,BASE,BYTES,RECORD,8,spr,sizeof(spr),vu,sizeof(vu)};
    memset(ram,0,BYTES);memset(spr,0x93,sizeof(spr));memset(vu,0,sizeof(vu));
    put(RECORD+0x2EF0,ACTOR);put(ACTOR+0x98,HEADER);put(HEADER+4,64);put(ACTOR+0x24,DEF);ram[DEF+6-BASE]=1;
    put(ACTOR+0x34,0x100);ram[ACTOR+0x55-BASE]=0xA0;number(ACTOR+0x70,1);
    number(RECORD+0x20,1);number(RECORD+0x34,1);number(RECORD+0x48,1);
    number(RECORD+0xA0,1);number(RECORD+0xA4,2);number(RECORD+0xA8,3);
    number(RECORD+0x1F0,2);number(RECORD+0x1F4,.25f);number(RECORD+0x204,.5f);
    return q;
}
static void refuse(DreadzonePlayerRecordTransform *q,int expected){
    memcpy(before,ram,BYTES);memcpy(old_spr,spr,sizeof(spr));
    CHECK(dreadzone_player_record_transform(q)==expected);CHECK(!memcmp(before,ram,BYTES));CHECK(!memcmp(old_spr,spr,sizeof(spr)));
}
int main(void){
    DreadzonePlayerRecordTransform q;DreadzonePlayerVolume v;
    ram=malloc(BYTES);before=malloc(BYTES);if(!ram||!before){free(ram);free(before);return 2;}
    q=setup();CHECK(dreadzone_player_record_transform(&q)==1);
    CHECK(ram[ACTOR+0x2C-BASE]==0xFF&&ram[ACTOR+0x66-BASE]==0x10&&ram[ACTOR+0x55-BASE]==0);
    CHECK(value(ACTOR)==1024&&value(ACTOR+4)==2048&&value(ACTOR+8)==3072);
    CHECK(value(HEADER+0x28)==256&&value(HEADER+0x2C)==512&&value(HEADER+0x14)==1792);
    CHECK(get(HEADER+4)==64&&value(HEADER+0x4C)==512);
    q=setup();put(RECORD+0x25D4,17);CHECK(dreadzone_player_record_transform(&q)==1);CHECK(value(HEADER+0x48)==256&&value(HEADER+0x34)==1792);
    q=setup();put(RECORD+0x25CC,129);CHECK(dreadzone_player_record_transform(&q)==1);CHECK(value(HEADER+0x48)==256);
    q=setup();put(RECORD+0x52C,0xFFFF);number(RECORD+0x520,.375f);CHECK(dreadzone_player_record_transform(&q)==1);CHECK(value(ACTOR+0xF4)==.375f);
    q=setup();put(RECORD+0x52C,1);put(RECORD+0x4D0,1);number(RECORD+0x520,.375f);CHECK(dreadzone_player_record_transform(&q)==1);CHECK(value(ACTOR+0xF4)==0);
    q=setup();put(RECORD+0x3A0,1);CHECK(dreadzone_player_record_transform(&q)==1);CHECK(value(HEADER+0x28)==0&&ram[ACTOR+0x66-BASE]==0x10);
    q=setup();put(ACTOR+0x98,0);CHECK(dreadzone_player_record_transform(&q)==1);
    q=setup();q.record++;refuse(&q,-3);
    q=setup();q.record=BASE+BYTES-0x31E0;refuse(&q,-2);
    q=setup();put(RECORD+0x2EF0,BASE+BYTES);refuse(&q,-2);
    q=setup();put(RECORD+0x25CC,0x7E);refuse(&q,-3);
    q=setup();put(RECORD+0x25CC,0x99);put(RECORD+0x25D8,0x7E);refuse(&q,-3);
    q=setup();ram[RECORD+0x2659-BASE]=1;refuse(&q,-3);
    q=setup();put(RECORD+0x204,0x7FC00000);refuse(&q,-3);
    q=setup();put(ACTOR+0x70,0x80000000);refuse(&q,-3);
    q=setup();put(ACTOR+0x98,BASE+BYTES);refuse(&q,-3);
    q=setup();q.scratch=ram;refuse(&q,-1);
    q=setup();q.vu_data=spr;refuse(&q,-1);
    q=setup();q.actor_pool_count=0;refuse(&q,-1);
    q=setup();memcpy(ram+ACTOR+0xC0-BASE,&q,sizeof(q));CHECK(dreadzone_player_record_transform((DreadzonePlayerRecordTransform *)(ram+ACTOR+0xC0-BASE))==1);
    CHECK(dreadzone_player_record_transform(NULL)==-1);
    q=setup();v.storage=ram;v.guest_base=BASE;v.bytes=BYTES;v.actor=ACTOR;v.index=0;
    v.center[0]=1;v.center[1]=2;v.center[2]=3;v.center[3]=1;v.radius=.5f;v.height=2;
    number(ACTOR+0xC0,1);number(ACTOR+0xCC,2);number(ACTOR+0xD4,1);number(ACTOR+0xE8,1);
    CHECK(dreadzone_player_volume_update(&v)==1);CHECK(value(HEADER+0x20)==3072&&value(HEADER+0x24)==2048&&value(HEADER+0x28)==3072&&value(HEADER+0x2C)==512);
    put(HEADER+4,0xFFFFFFFF);v.center[0]=0;memcpy(before,ram,BYTES);CHECK(dreadzone_player_volume_update(&v)==0&&!memcmp(before,ram,BYTES));
    v.index=128;CHECK(dreadzone_player_volume_update(&v)==-3);CHECK(!memcmp(before,ram,BYTES));
    CHECK(dreadzone_player_volume_update(NULL)==-1);
    free(before);free(ram);printf("player transform/volume: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
