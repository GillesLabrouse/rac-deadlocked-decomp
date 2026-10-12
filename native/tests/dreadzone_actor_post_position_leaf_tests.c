#include "dreadzone_actor_post_position_leaf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {BASE=0x200000,RECORD=0x250000,BYTES=0x100000};
static unsigned char memory[BYTES+1],before[BYTES];
static int failures;
static void check(int good,const char *message) {
    if(!good){fprintf(stderr,"post-position leaf: %s\n",message);++failures;}
}
static void word(unsigned address,uint32_t value) {memcpy(memory+1+address-BASE,&value,4);}
static void real(unsigned address,float value) {memcpy(memory+1+address-BASE,&value,4);}
static void half(unsigned address,uint16_t value) {memcpy(memory+1+address-BASE,&value,2);}
static uint32_t load(unsigned address) {uint32_t v;memcpy(&v,memory+1+address-BASE,4);return v;}
static DreadzonePostPositionLeafRequest setup(void) {
    DreadzonePostPositionLeafRequest r={memory+1,BASE,BYTES,RECORD,0x22597F};
    memset(memory,0,sizeof(memory));real(0x21DDF4,1.f/60.f);
    real(RECORD+0xA0,558.f);real(RECORD+0xA4,330.f);real(RECORD+0xA8,188.f);
    real(RECORD+0x1E0,557.f);real(RECORD+0x1E4,330.f);real(RECORD+0x1E8,188.f);
    real(RECORD+0x204,.1f);return r;
}
static void refusal(DreadzonePostPositionLeafRequest *r,int expected) {
    DreadzonePostPositionLeafInfo info={0xAAAAAAAA,0xBBBBBBBB},old=info;
    memcpy(before,memory+1,BYTES);
    check(dreadzone_actor_post_position_leaf_00686420(r,&info)==expected,"refusal status");
    check(!memcmp(memory+1,before,BYTES)&&!memcmp(&info,&old,sizeof(info)),"atomic refusal");
}
int main(void) {
    DreadzonePostPositionLeafRequest r;DreadzonePostPositionLeafInfo info;
    r=setup();r.gp_guest=0;word(RECORD+0x214,0x7FC12345);
    check(dreadzone_actor_post_position_leaf_00686420(&r,&info)==1,"zero-timer return");
    check(load(RECORD+0x214)==0 && info.original_low_word==0 && info.stores==1,"real clear and zero return");

    r=setup();memory[1+RECORD-BASE+0x226]=1;real(RECORD+0x2CC,0.f);
    word(0x21DDF4,0x7FC12345);
    check(dreadzone_actor_post_position_leaf_00686420(&r,&info)==1,"unconsumed delay NaN");
    r.gp_guest=0;refusal(&r,-1);

    r=setup();memory[1+RECORD-BASE+0x226]=1;real(RECORD+0x2CC,1.f);
    real(RECORD+0x25C4,1.f);real(RECORD+0x138,-.015f);
    check(dreadzone_actor_post_position_leaf_00686420(&r,&info)==1,"genuine length return");
    check(load(RECORD+0x3CC)==5 && info.original_low_word==0x3F800000 && info.stores==1,"mark and float return bits");

    r=setup();half(RECORD+0x3CC,1);word(RECORD+0x1EC,0x7FC12345);
    word(RECORD+0x214,0x12345678);
    check(dreadzone_actor_post_position_leaf_00686420(&r,&info)==1,"second source W unconsumed");
    check(info.original_low_word==0x3F800000 && info.stores==0 && load(RECORD+0x214)==0x12345678,"unchanged second-length return");
    word(RECORD+0xAC,0x7FC12345);refusal(&r,-2);

    r=setup();memory[1+RECORD-BASE+0x226]=1;real(RECORD+0x2CC,1.f);
    real(RECORD+0x25C4,1.f);real(RECORD+0x138,-.015f);
    real(RECORD+0x1E0,557.99f);refusal(&r,-2); /* Mark remains unpublished on reached angle. */
    r=setup();++r.record_guest;refusal(&r,-1);
    r=setup();r.storage_bytes=0;refusal(&r,-1);
    r=setup();r.guest_base=0x70000000;r.record_guest=0x70001000;refusal(&r,-1);
    check(failures==0,"complete regression set");return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
