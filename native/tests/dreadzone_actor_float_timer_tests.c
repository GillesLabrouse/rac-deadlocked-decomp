#include "dreadzone_actor_float_timer.h"
#include <stdio.h>
#include <string.h>
#define BASE 0x21D000U
#define GP 0x22597FU
#define CLOCK (GP-0x7B7BU)
#define TIMER 0x222100U
static u32 checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"float timer check failed line%d\n",__LINE__);return 1;}}while(0)
static void word(u8 *b,u32 a,u32 value){memcpy(b+a-BASE,&value,4);}
static u32 read_word(u8 *b,u32 a){u32 value;memcpy(&value,b+a-BASE,4);return value;}
int main(void){
    u8 b[0x10000],before[0x10000];DreadzoneActorFloatTimerRequest q={b,BASE,sizeof(b),TIMER,GP};
    DreadzoneActorFloatTimerInfo info,old;u32 i;
    const u32 inputs[]={0,0x80000000U,0xBF800000U,0x3C888888U,0x3C888889U,0x3C88888AU};
    const s32 returns[]={-1,-1,-1,-2,-2,0};
    for(i=0;i<sizeof(inputs)/sizeof(inputs[0]);++i){
        memset(b,0xA5,sizeof(b));word(b,CLOCK,0x3C888889);word(b,TIMER,inputs[i]);
        CHECK(dreadzone_actor_float_timer_0055D240(&q,&info)==1);
        CHECK(info.returned==returns[i]&&info.stored_word==read_word(b,TIMER));
        CHECK((i==5&&info.stored_word==0x31000000U)||(i<5&&info.stored_word==0));
    }
    word(b,CLOCK,0x3C888889);q.timer_guest=CLOCK;
    CHECK(dreadzone_actor_float_timer_0055D240(&q,&info)==1);
    CHECK(info.returned==-2&&!read_word(b,CLOCK));
    q.timer_guest=TIMER;word(b,CLOCK,0x3C888889);word(b,TIMER,0x7FC0BEEF);
    memset(&info,0xA5,sizeof(info));old=info;memcpy(before,b,sizeof(b));
    CHECK(dreadzone_actor_float_timer_0055D240(&q,&info)==-2);
    CHECK(!memcmp(before,b,sizeof(b))&&!memcmp(&old,&info,sizeof(info)));
    word(b,TIMER,0x3F800000);q.gp_guest=0x500000;memcpy(before,b,sizeof(b));
    CHECK(dreadzone_actor_float_timer_0055D240(&q,&info)==-1);
    CHECK(!memcmp(before,b,sizeof(b))&&!memcmp(&old,&info,sizeof(info)));
    CHECK(dreadzone_actor_float_timer_0055D240(NULL,&info)==-1);
    CHECK(dreadzone_actor_float_timer_0055D240(&q,NULL)==-1);
    CHECK(dreadzone_actor_float_timer_0055D240(&q,(DreadzoneActorFloatTimerInfo *)b)==-1);
    CHECK(!memcmp(before,b,sizeof(b)));
    q.gp_guest=GP;word(b,TIMER,0x3C888889);memcpy(b+0x8000,&q,sizeof(q));
    CHECK(dreadzone_actor_float_timer_0055D240((DreadzoneActorFloatTimerRequest *)(b+0x8000),&info)==1);
    CHECK(info.returned==-2&&!read_word(b,TIMER));
    printf("float timer checks: %u\n",checks);return 0;
}
