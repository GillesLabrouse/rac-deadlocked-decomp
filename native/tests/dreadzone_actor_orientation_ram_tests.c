#include "dreadzone_actor_orientation_ram.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BASE 0x21D000U
#define RECORD 0x368F60U
#define BYTES (0x3B95C0U-BASE)
#define GP 0x22597FU
static u32 checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"orientation RAM check failed line%d\n",__LINE__);return 1;}}while(0)
static void word(u8 *b,u32 a,u32 v){memcpy(b+a-BASE,&v,4);}
static void real(u8 *b,u32 a,float v){memcpy(b+a-BASE,&v,4);}
static u32 read(const u8 *b,u32 a){u32 v;memcpy(&v,b+a-BASE,4);return v;}
static void fixture(u8 *b){
    static const float coefficients[8]={0.999999344348907470703125f,-0.33329856395721435546875f,
        0.1994653642177581787109375f,-0.13908533751964569091796875f,
        0.096420042216777801513671875f,-0.05590988695621490478515625f,
        0.0218612290918827056884765625f,-0.0040540578775107860565185546875f};
    static const float quadrants[16]={1,0,-1,1.57079637050628662109375f,
        -1,0,1,-1.57079637050628662109375f,-1,3.1415927410125732421875f,1,1.57079637050628662109375f,
        1,-3.1415927410125732421875f,-1,-1.57079637050628662109375f};
    /* Synthetic component fixture; this does not initialize a retail frame. */
    memset(b,0,BYTES);real(b,0x21DDEC,1.f);real(b,0x21DDF4,1.f/60.f);
    real(b,RECORD+0x25C4,1.f);real(b,RECORD+0xB0,.15f);
    real(b,RECORD+0xB4,-.125f);real(b,RECORD+0xB8,.25f);
    real(b,RECORD+0xC0,.075f);real(b,RECORD+0xC4,.05f);
    word(b,RECORD+0xBC,0x7FC0BEEF);b[RECORD+0x266D-BASE]=1;
    word(b,RECORD+0x25CC,2);word(b,RECORD+0x25D4,2);
    memcpy(b+0x3B9540-BASE,coefficients,sizeof(coefficients));
    memcpy(b+0x3B9580-BASE,quadrants,sizeof(quadrants));
}
int main(void){
    u8 *b=(u8 *)malloc(BYTES),*before=(u8 *)malloc(BYTES);u32 i;
    DreadzoneActorOrientationRAMRequest q={b,BASE,BYTES,RECORD,GP};
    DreadzoneActorOrientationRAMInfo info,old;CHECK(b&&before);
    fixture(b);CHECK(dreadzone_actor_orientation_ram_00683310(&q,&info)==1);
    /* Frozen actual-word finite reference results for the explicit input. */
    CHECK(read(b,RECORD+0xB0)==0x3E4C73CC&&read(b,RECORD+0xB4)==0xBDB55310);
    CHECK(read(b,RECORD+0xB8)==0x3E82D4F4&&read(b,RECORD+0xC0)==0x3D4DD2F2);
    CHECK(read(b,RECORD+0xC4)==0x3D170A3E&&read(b,RECORD+0xBC)==0x7FC0BEEF);
    CHECK(info.turns==2&&info.compositions==1&&info.extractions==1&&!info.skips);
    for(i=0;i<8;++i)CHECK(dreadzone_actor_orientation_ram_00683310(&q,&info)==1&&info.turns==2);
    fixture(b);word(b,0x3B9540,0);memset(&info,0xA5,sizeof(info));old=info;memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_ram_00683310(&q,&info)==-2);
    CHECK(!memcmp(before,b,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    fixture(b);real(b,RECORD+0xB0,4.f);memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_ram_00683310(&q,&info)==-3&&!memcmp(before,b,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    fixture(b);word(b,RECORD+0xC4,1);memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_ram_00683310(&q,&info)==-3&&!memcmp(before,b,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    fixture(b);b[RECORD+0x2659-BASE]=1;memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_ram_00683310(&q,&info)==-2&&!memcmp(before,b,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    fixture(b);word(b,RECORD+0x25CC,36);word(b,RECORD+0xB0,0x7FC01234);memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_ram_00683310(&q,&info)==1&&info.skips==1&&!info.turns&&!memcmp(before,b,BYTES));
    fixture(b);b[RECORD+0x2659-BASE]=3;word(b,RECORD+0xB0,0x7FC01234);memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_ram_00683310(&q,&info)==1&&info.skips==1&&!memcmp(before,b,BYTES));
    fixture(b);b[RECORD+0x266D-BASE]=0;b[0x220649-BASE]=1;word(b,RECORD+0x2F10,2);word(b,GP-0x7BC7,1);word(b,RECORD+0xB0,0x7FC01234);memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_ram_00683310(&q,&info)==1&&info.skips==1&&!memcmp(before,b,BYTES));
    fixture(b);word(b,RECORD+0xB0,0);word(b,RECORD+0xB4,0);word(b,RECORD+0xC0,0);word(b,RECORD+0xC4,0);word(b,0x3B9540,0);
    CHECK(dreadzone_actor_orientation_ram_00683310(&q,&info)==1&&info.turns==2&&!info.compositions&&!info.extractions);
    fixture(b);q.gp_guest=GP+0x100;real(b,q.gp_guest-0x7B93,16.f);memset(&info,0xA5,sizeof(info));old=info;memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_ram_00683310(&q,&info)==-3&&!memcmp(before,b,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    q.gp_guest=GP;fixture(b);q.record_guest=RECORD+4;memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_ram_00683310(&q,&info)==-1&&!memcmp(before,b,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    q.record_guest=RECORD;
    CHECK(dreadzone_actor_orientation_ram_00683310(NULL,&info)==-1);
    CHECK(dreadzone_actor_orientation_ram_00683310(&q,NULL)==-1);
    CHECK(dreadzone_actor_orientation_ram_00683310(&q,(DreadzoneActorOrientationRAMInfo *)b)==-1);
    fixture(b);memcpy(b+0x200,&q,sizeof(q));
    CHECK(dreadzone_actor_orientation_ram_00683310((DreadzoneActorOrientationRAMRequest *)(b+0x200),&info)==1&&info.compositions==1);
    free(before);free(b);printf("orientation RAM checks: %u\n",checks);return 0;
}
