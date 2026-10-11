#include "dreadzone_actor_orientation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BASE 0x21D000U
#define RECORD 0x368F60U
#define BYTES (RECORD+0x31F0U-BASE)
#define GP 0x22597FU
static u32 checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"actor orientation check failed line%d\n",__LINE__);return 1;}}while(0)
static void word(u8 *b,u32 a,u32 v){memcpy(b+a-BASE,&v,4);}
static void real(u8 *b,u32 a,float v){memcpy(b+a-BASE,&v,4);}
static u32 read(u8 *b,u32 a){u32 v;memcpy(&v,b+a-BASE,4);return v;}
static void fixture(u8 *b){
    memset(b,0,BYTES);real(b,0x21DDEC,1.f);real(b,0x21DDF4,1.f/60.f);
    real(b,RECORD+0x25C4,1.f);real(b,RECORD+0xB8,.125f);
    real(b,RECORD+0xC0,.075f);real(b,RECORD+0xC4,-.05f);
    word(b,RECORD+0xBC,0x7FC0BEEF);b[RECORD+0x266D-BASE]=1;
}
int main(void){
    u8 *b=(u8 *)malloc(BYTES),*before=(u8 *)malloc(BYTES);
    DreadzoneActorOrientationRequest q={b,BASE,BYTES,RECORD,GP,{0x7FC12345,0xFFFFFFFF,0x80000000,0x3F000000,1},{0xDEADBEEF,0x01234567}};
    DreadzoneActorOrientationInfo info,old;u32 i;CHECK(b&&before);
    fixture(b);CHECK(dreadzone_actor_orientation_00683310(&q,&info)==1);
    CHECK(read(b,RECORD+0xC0)==0&&read(b,RECORD+0xC4)==0);
    CHECK(read(b,RECORD+0xB8)==0x3E000000&&read(b,RECORD+0xBC)==0x7FC0BEEF);
    CHECK(info.turns==2&&!info.skips&&!info.compositions&&!info.extractions);
    CHECK(!memcmp(info.outgoing_vf25_29_w,q.incoming_vf25_29_w,20)&&!memcmp(info.outgoing_a0_high64,q.incoming_a0_high64,8));
    for(i=0;i<16;++i)CHECK(dreadzone_actor_orientation_00683310(&q,&info)==1&&info.turns==2);
    CHECK(read(b,RECORD+0xB8)==0x3E000000&&read(b,RECORD+0xC0)==0&&read(b,RECORD+0xC4)==0);
    fixture(b);q.gp_guest=GP+0x100;real(b,q.gp_guest-0x7B93,16.f);memset(&info,0xA5,sizeof(info));old=info;memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_00683310(&q,&info)==-3);
    CHECK(!memcmp(before,b,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    word(b,q.gp_guest-0x7B93,0);CHECK(dreadzone_actor_orientation_00683310(&q,&info)==1&&info.turns==2);q.gp_guest=GP;
    fixture(b);word(b,RECORD+0xB0,0x80000000);word(b,RECORD+0xB4,0x80000000);word(b,RECORD+0xB8,0x80000000);
    CHECK(dreadzone_actor_orientation_00683310(&q,&info)==1);
    /* The actual wrapped sum adds positive zero: yaw -0 becomes +0. */
    CHECK(read(b,RECORD+0xB8)==0&&read(b,RECORD+0xB0)==0x80000000&&read(b,RECORD+0xB4)==0x80000000);
    fixture(b);word(b,RECORD+0x25CC,36);word(b,RECORD+0xB0,0x7FC01234);memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_00683310(&q,&info)==1&&info.skips==1&&!info.turns);
    CHECK(!memcmp(before,b,BYTES)&&!memcmp(info.outgoing_a0_high64,q.incoming_a0_high64,8));
    fixture(b);b[RECORD+0x2659-BASE]=3;word(b,RECORD+0xB0,0x7FC01234);memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_00683310(&q,&info)==1&&info.skips==1&&!memcmp(before,b,BYTES));
    fixture(b);b[RECORD+0x266D-BASE]=0;b[0x220649-BASE]=1;word(b,RECORD+0x2F10,2);word(b,GP-0x7BC7,1);word(b,RECORD+0xB0,0x7FC01234);memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_00683310(&q,&info)==1&&info.skips==1&&!memcmp(before,b,BYTES));
    q.gp_guest=GP+0x100;word(b,q.gp_guest-0x7BC7,2);
    CHECK(dreadzone_actor_orientation_00683310(&q,&info)==-3);
    q.gp_guest=GP;fixture(b);real(b,RECORD+0xB0,.3f);memset(&info,0xA5,sizeof(info));old=info;memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_00683310(&q,&info)==-2);
    CHECK(!memcmp(before,b,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    fixture(b);b[RECORD+0x2659-BASE]=1;memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_00683310(&q,&info)==-2&&!memcmp(before,b,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    fixture(b);word(b,RECORD+0xC0,1);memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_00683310(&q,&info)==-3&&!memcmp(before,b,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    fixture(b);q.gp_guest=0x500000;memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_00683310(&q,&info)==-1&&!memcmp(before,b,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    q.gp_guest=GP;fixture(b);b[RECORD+0x266D-BASE]=0;b[0x220649-BASE]=0;q.storage=b+0xDC0;q.guest_base=0x21DDC0;q.storage_bytes=BYTES-0xDC0;memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_orientation_00683310(&q,&info)==-1&&!memcmp(before,b,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    q.storage=b;q.guest_base=BASE;q.storage_bytes=BYTES;fixture(b);
    CHECK(dreadzone_actor_orientation_00683310(NULL,&info)==-1);
    CHECK(dreadzone_actor_orientation_00683310(&q,NULL)==-1);
    CHECK(dreadzone_actor_orientation_00683310(&q,(DreadzoneActorOrientationInfo *)b)==-1);
    memcpy(b+0x200,&q,sizeof(q));
    CHECK(dreadzone_actor_orientation_00683310((DreadzoneActorOrientationRequest *)(b+0x200),&info)==1&&info.turns==2);
    free(before);free(b);printf("actor orientation checks: %u\n",checks);return 0;
}
