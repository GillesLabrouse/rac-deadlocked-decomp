#include "dreadzone_animation_clock.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned checks;
static int failed;
#define CHECK(x) do { ++checks; if (!(x)) { \
    fprintf(stderr,"line %d: %s\n",__LINE__,#x); failed=1; } } while (0)
static void word(uint8_t *p, uint32_t v) {
    p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);
}
static void half(uint8_t *p, uint16_t v) { p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8); }
static uint32_t bits(float v) { uint32_t r;memcpy(&r,&v,4);return r; }
static float frame(const uint8_t *p) { float r;memcpy(&r,p+0x44,4);return r; }
static uint16_t duration(const uint8_t *p) { return (uint16_t)(p[0x4C]|(uint16_t)p[0x4D]<<8); }
static void setup(uint8_t *actor,uint8_t *definition,uint32_t count,
    float initial,float rate,uint8_t flags,const uint16_t *durations) {
    uint32_t i;
    memset(actor,0xA5,256);memset(definition,0x5A,0x420);
    definition[0x10]=(uint8_t)count;
    for(i=0;i<count;++i) { half(definition+0x20+i*4,durations[i]);half(definition+0x22+i*4,(uint16_t)(i*16)); }
    word(actor+0x40,0x500000);word(actor+0x44,bits(initial));word(actor+0x48,bits(rate));
    half(actor+0x4C,durations[(uint32_t)initial]);word(actor+0x50,0);actor[0x55]=flags;
}
static void check_untouched(const uint8_t *before,const uint8_t *after) {
    unsigned i;
    for(i=0;i<256;++i) if(!((i>=0x44&&i<0x48)||(i>=0x4C&&i<0x4E)||i==0x55)) CHECK(before[i]==after[i]);
}
static void refusal(DreadzoneAnimationClockRequest *request,uint8_t *actor,int expected) {
    uint8_t before[256];memcpy(before,actor,256);
    CHECK(dreadzone_animation_clock_advance(request)==expected);
    CHECK(memcmp(before,actor,256)==0);
}
int main(void) {
    uint8_t arena[288],definition_arena[0x440],before[256],definition_before[0x420];
    uint8_t *actor=arena+16,*definition=definition_arena+16;
    uint16_t durations[255];unsigned i;
    DreadzoneAnimationClockRequest request={actor,256,0x400000,definition,0x420,0x500000};
    memset(arena,0xC3,sizeof(arena));memset(definition_arena,0xD7,sizeof(definition_arena));
    for(i=0;i<255;++i) durations[i]=2;
    setup(actor,definition,30,0,1,0xCB,durations);
    memcpy(definition_before,definition,0x420);
    for(i=0;i<64;++i) {
        memcpy(before,actor,256);
        CHECK(dreadzone_animation_clock_advance(&request)==1);
        CHECK(frame(actor)==(float)((i+1)%60)*0.5f);
        CHECK(duration(actor)==2);
        CHECK((actor[0x55]&2)==(i==59 ? 2 : 0));
        CHECK((actor[0x55]&0xFD)==((0xCB&0xFD)|0x20));
        check_untouched(before,actor);
    }
    durations[0]=2;durations[1]=4;durations[2]=8;
    setup(actor,definition,3,0.5f,7,0,durations);
    CHECK(dreadzone_animation_clock_advance(&request)==1);
    CHECK(frame(actor)==2.25f && duration(actor)==8 && actor[0x55]==0x20);
    setup(actor,definition,3,2.5f,12,0,durations);
    CHECK(dreadzone_animation_clock_advance(&request)==1);
    CHECK(frame(actor)==2.25f && duration(actor)==8 && actor[0x55]==0x22);
    setup(actor,definition,3,0,-1,0,durations);
    CHECK(dreadzone_animation_clock_advance(&request)==1);
    CHECK(frame(actor)==2.875f && duration(actor)==8 && actor[0x55]==0x22);
    setup(actor,definition,3,1.25f,-2,0,durations);
    CHECK(dreadzone_animation_clock_advance(&request)==1);
    CHECK(frame(actor)==0.5f && duration(actor)==2 && actor[0x55]==0x20);
    setup(actor,definition,3,1,100,0x10,durations);
    CHECK(dreadzone_animation_clock_advance(&request)==1);
    CHECK(frame(actor)==2 && duration(actor)==8 && actor[0x55]==0x32);
    setup(actor,definition,3,0,-1,0x10,durations);
    CHECK(dreadzone_animation_clock_advance(&request)==1);
    CHECK(frame(actor)==0 && duration(actor)==2 && actor[0x55]==0x32);
    /* The original reverse clamp adds the old duration4 before reloading2. */
    setup(actor,definition,3,1,-1,0x10,durations);
    CHECK(dreadzone_animation_clock_advance(&request)==1);
    CHECK(frame(actor)==1.5f && duration(actor)==2 && actor[0x55]==0x30);
    setup(actor,definition,3,0.5f,0,0xFF,durations);
    memcpy(before,actor,256);
    CHECK(dreadzone_animation_clock_advance(&request)==1);
    CHECK(memcmp(before,actor,256)==0);
    word(actor+0x48,UINT32_C(0x80000000));
    CHECK(dreadzone_animation_clock_advance(&request)==1);
    CHECK(frame(actor)==0.5f && actor[0x55]==0xFD);
    setup(actor,definition,3,0,-0.5f,0,durations);
    refusal(&request,actor,-2);
    setup(actor,definition,3,2.5f,-0.5f,0,durations);
    CHECK(dreadzone_animation_clock_advance(&request)==1);
    CHECK(frame(actor)==2.4375f && duration(actor)==8 && actor[0x55]==0x20);
    setup(actor,definition,255,254.5f,1,0,durations);
    CHECK(dreadzone_animation_clock_advance(&request)==1);
    CHECK(frame(actor)==0 && actor[0x55]==0x22);
    setup(actor,definition,3,0,1,0,durations);
    word(actor+0x50,0x28EB40);refusal(&request,actor,-2);word(actor+0x50,0);
    word(actor+0x44,UINT32_C(0x7F800000));refusal(&request,actor,-2);word(actor+0x44,bits(0));
    word(actor+0x48,UINT32_C(0x7FC00001));refusal(&request,actor,-2);word(actor+0x48,bits(1));
    half(definition+0x24,0);refusal(&request,actor,-2);half(definition+0x24,4);
    request.definition_bytes=0x24;refusal(&request,actor,-1);request.definition_bytes=0x420;
    request.actor_guest=0xFFFFFFFC;refusal(&request,actor,-1);request.actor_guest=0x400000;
    request.definition_guest=0x400000;refusal(&request,actor,-1);request.definition_guest=0x500000;
    request.definition=actor;refusal(&request,actor,-1);request.definition=definition;
    request.actor_bytes=0x55;refusal(&request,actor,-1);request.actor_bytes=256;
    CHECK(dreadzone_animation_clock_advance(NULL)==-1);
    for(i=0;i<16;++i) {
        CHECK(arena[i]==0xC3&&arena[272+i]==0xC3);
        CHECK(definition_arena[i]==0xD7&&definition_arena[0x430+i]==0xD7);
    }
    /* Metadata storage was only modified by this fixture, never the entry. */
    setup(actor,definition,30,0,1,0,durations);memcpy(definition_before,definition,0x420);
    CHECK(dreadzone_animation_clock_advance(&request)==1);
    CHECK(memcmp(definition_before,definition,0x420)==0);
    /* Output may contain the borrowed descriptor; capture makes it safe. */
    setup(actor,definition,30,0,1,0,durations);
    memcpy(actor+0x58,&request,sizeof(request));
    CHECK(dreadzone_animation_clock_advance((const DreadzoneAnimationClockRequest *)(void *)(actor+0x58))==1);
    CHECK(frame(actor)==0.5f);
    printf("animation clock: %u checks\n",checks);
    return failed;
}
