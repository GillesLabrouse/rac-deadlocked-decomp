#include "dreadzone_actor_post_position.h"
#include <fenv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { BASE=0x100000,BYTES=0x300000,RECORD=0x250000,BODY=0x280000,
       DEFINITION=0x281000,INPUT=0x282000 };
static unsigned char memory[BYTES+1],before[BYTES];
static int failures;
static void check(int good,const char *message) {
    if(!good){fprintf(stderr,"post-position: %s\n",message);++failures;}
}
static void word(unsigned address,uint32_t value){memcpy(memory+1+address-BASE,&value,4);}
static void real(unsigned address,float value){memcpy(memory+1+address-BASE,&value,4);}
static void half(unsigned address,uint16_t value){memcpy(memory+1+address-BASE,&value,2);}
static uint32_t load(unsigned address){uint32_t value;memcpy(&value,memory+1+address-BASE,4);return value;}
static DreadzonePostPositionRequest setup(void) {
    DreadzonePostPositionRequest r={memory+1,BASE,BYTES,RECORD,0x22597F};unsigned i;
    memset(memory,0,sizeof(memory));
    memory[1+RECORD-BASE+0x2672]=1; /* Explicitly select the original orientation-skip gate. */
    word(RECORD+0x2EF0,BODY);word(BODY+0x24,DEFINITION);real(DEFINITION+0x24,1.f);
    word(RECORD+0x2F00,INPUT);real(RECORD+0x25C4,1.f);real(0x21DE04,1.f/60.f);
    for(i=0;i<4;++i)real(RECORD+0x20+i*16+i*4,1.f);
    real(RECORD+0xA0,6.f);real(RECORD+0xA4,8.f);real(RECORD+0xA8,10.f);
    real(RECORD+0x280,6.f);real(RECORD+0x284,8.f);real(RECORD+0x288,9.5f);
    word(RECORD+0x28C,0x7FC00001);return r;
}
static void refusal(DreadzonePostPositionRequest *r,int expected) {
    DreadzonePostPositionInfo info,old;memset(&info,0xAA,sizeof(info));old=info;
    memcpy(before,memory+1,BYTES);
    check(dreadzone_actor_post_position_00684EF0(r,&info)==expected,"refusal status");
    check(!memcmp(before,memory+1,BYTES)&&!memcmp(&old,&info,sizeof(info)),"atomic RAM/info refusal");
}
int main(void) {
    DreadzonePostPositionRequest r=setup();DreadzonePostPositionInfo info;int original_round=fegetround();
    check(dreadzone_actor_post_position_00684EF0(&r,&info)==1,"complete selected return");
    check(info.orientation==0 && info.visibility==1 && info.half_timers==42 && info.word_timers==9 &&
          info.float_timers==1 && info.queue_calls==1 && info.input_calls==1 && info.leaf_calls==1 &&
          info.height_calls==1,"actual top-level calls");
    check(load(RECORD+0x370)==1 && load(RECORD+0x3B4)==1 && load(RECORD+0x214)==0,"original counters and clear");
    check(load(RECORD+0xE0)==0x40C00000 && load(RECORD+0xE4)==0x41000000 &&
          load(RECORD+0xE8)==0x4121999A && load(RECORD+0xEC)==0x7FC00001,"aliased height and raw W transport");
    check(load(RECORD+0xF0)==0x40C00000 && load(RECORD+0xF4)==0x41000000 &&
          load(RECORD+0xF8)==0x412B3333,"matrix projection fixture");
    r=setup();half(RECORD+0x408,5);word(RECORD+0x2FB4,0x12345678);
    check(dreadzone_actor_post_position_00684EF0(&r,&info)==1 && load(RECORD+0x2FB4)==0x12345678,"live timer leaves state intact");
    r=setup();half(RECORD+0x408,1);word(RECORD+0x2FB4,0x12345678);
    check(dreadzone_actor_post_position_00684EF0(&r,&info)==1 && load(RECORD+0x2FB4)==0,"expiry clears state");
    r=setup();real(RECORD+0x25C4,.5f);refusal(&r,-2);
    r=setup();word(RECORD+0x2EE0,1);refusal(&r,-2);
    r=setup();real(RECORD+0xA8,.5f);refusal(&r,-2);
    r=setup();memory[1+RECORD-BASE+0x2659]=1;refusal(&r,-2);
    r=setup();++r.record_guest;refusal(&r,-1);
    r=setup();if(fesetround(FE_DOWNWARD)==0){refusal(&r,-2);check(fesetround(original_round)==0,"restore host rounding");}
    check(failures==0,"complete regression set");return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
