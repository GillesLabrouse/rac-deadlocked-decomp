#include "dreadzone_player_history.h"
#include <fenv.h>
#include <stdio.h>
#include <string.h>

static unsigned char memory[0x2C82],before[0x2C80],expected[0x2C80];
static unsigned checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"history line %d failed\n",__LINE__);return 1;}}while(0)
static void word(unsigned offset,uint32_t value){memcpy(memory+1+offset,&value,4);}
static uint32_t read_word(unsigned offset){uint32_t v;memcpy(&v,memory+1+offset,4);return v;}
static DreadzonePlayerHistoryRequest setup(void) {
    DreadzonePlayerHistoryRequest r={memory+1,0x368F60,0x2C80,0x368F60};
    static const uint32_t raw[4]={0x80000000,0x7FC12345,0xFFC23456,0xDEADBEEF};
    memset(memory,0xD3,sizeof(memory));memcpy(memory+1+0xA0,raw,16);
    word(0xB8,0x7FC34567);word(0x2C70,31);word(0x2C74,31);
    word(0x2C78,31);word(0x2C7C,32);return r;
}
int main(void) {
    DreadzonePlayerHistoryRequest q=setup();DreadzonePlayerHistoryInfo info,old;
    uint32_t angle=read_word(0xB8);int rounding=fegetround();
    memcpy(expected,memory+1,sizeof(expected));
    memcpy(expected+0x26F0+31*16,memory+1+0xA0,16);
    memcpy(expected+0x28F0+31*4,&angle,4);
    {uint32_t zero=0,count=32;memcpy(expected+0x2C70,&zero,4);memcpy(expected+0x2C74,&count,4);memcpy(expected+0x2C78,&zero,4);}
    CHECK(dreadzone_player_history_before_basis(&q,&info)==1);
    CHECK(!memcmp(expected,memory+1,sizeof(expected)));
    CHECK(info.angle_slot==31 && info.position_slot==31 && info.angle_written==1 && info.position_written==1);
    CHECK(memory[0]==0xD3 && memory[sizeof(memory)-1]==0xD3);
    /* A late position refusal must roll back the already staged angle ring. */
    q=setup();word(0x2C7C,33);memcpy(before,memory+1,sizeof(before));
    memset(&info,0xAA,sizeof(info));old=info;
    CHECK(dreadzone_player_history_before_basis(&q,&info)==-2);
    CHECK(!memcmp(before,memory+1,sizeof(before)) && !memcmp(&info,&old,sizeof(info)));
    q=setup();word(0x2C70,32);memcpy(before,memory+1,sizeof(before));
    CHECK(dreadzone_player_history_before_basis(&q,&info)==-2);
    CHECK(!memcmp(before,memory+1,sizeof(before)) && !memcmp(&info,&old,sizeof(info)));
    q=setup();memcpy(before,memory+1,sizeof(before));q.record_guest++;
    CHECK(dreadzone_player_position_history_0068EFE8(&q,&info)==-1);
    CHECK(!memcmp(before,memory+1,sizeof(before)) && !memcmp(&info,&old,sizeof(info)));
    q=setup();CHECK(dreadzone_player_history_before_basis(&q,(DreadzonePlayerHistoryInfo *)(memory+1))==-1);
    CHECK(!memcmp(before,memory+1,sizeof(before)));
    /* Raw transport is valid under another host rounding mode. */
    q=setup();if(fesetround(FE_DOWNWARD)==0) {
        CHECK(dreadzone_player_history_before_basis(&q,&info)==1);
        CHECK(!memcmp(expected,memory+1,sizeof(expected)));
        CHECK(fesetround(rounding)==0);
    }
    printf("history checks: %u\n",checks);return 0;
}
