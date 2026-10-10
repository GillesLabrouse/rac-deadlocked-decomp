#include "dreadzone_animation_update.h"
#include <stdio.h>
#include <string.h>

enum { ACTOR, RECORDS, DEFINITION, EVENTS, MODE, PHASE, POINTER, NODES, OCCUPANCY, METADATA, REGIONS };
static u8 storage[REGIONS][0x2040], saved[REGIONS][0x2040];
static const u32 bases[REGIONS]={0x400000,0x1D24C0,0x500000,0x510000,0x21E824,0x21DDB8,0x21F710,0x28EB40,0x28EF40,0x292FF8};
static const u32 sizes[REGIONS]={256,63*128,0x420,16,4,4,4,1024,32,0x808};
static u8 *region[REGIONS];
static DreadzoneAnimationMemory views[REGIONS];
static DreadzoneAnimationContext context;
static DreadzoneAnimationUpdateRequest request;
static unsigned checks, failures;
#define CHECK(x) do { ++checks; if(!(x)) { ++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x); } } while(0)
static void word(u8 *p,u32 v) { p[0]=(u8)v;p[1]=(u8)(v>>8);p[2]=(u8)(v>>16);p[3]=(u8)(v>>24); }
static void half(u8 *p,u16 v) { p[0]=(u8)v;p[1]=(u8)(v>>8); }
static u32 readword(const u8 *p) { return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24); }
static u32 bits(f32 value) { u32 raw;memcpy(&raw,&value,4);return raw; }
static void setup(void) {
    unsigned i;
    for(i=0;i<REGIONS;++i) {
        memset(storage[i],0xA5,sizeof(storage[i]));region[i]=storage[i]+16;
        views[i].storage=region[i];views[i].guest_base=bases[i];views[i].bytes=sizes[i];
    }
    memset(&context,0,sizeof(context));context.views=views;context.view_count=REGIONS;
    request.memory=&context;request.actor_guest=bases[ACTOR];
    word(region[ACTOR]+0x40,bases[DEFINITION]);word(region[ACTOR]+0x44,0);
    word(region[ACTOR]+0x48,bits(1));half(region[ACTOR]+0x4C,2);word(region[ACTOR]+0x50,0);
    half(region[ACTOR]+0x34,0);half(region[ACTOR]+0x36,0);half(region[ACTOR]+0x74,0);
    region[ACTOR][0x55]=3;region[ACTOR][0x57]=107;region[ACTOR][0x6C]=0;region[ACTOR][0x6D]=255;
    half(region[ACTOR]+0x6E,0xFFFF);region[DEFINITION][0x10]=30;
    for(i=0;i<30;++i) half(region[DEFINITION]+0x20+i*4,2);
    word(region[DEFINITION]+0x14,bases[EVENTS]);word(region[MODE],0);word(region[PHASE],0);word(region[POINTER],bases[RECORDS]);
}
static void save(void) { unsigned i;for(i=0;i<REGIONS;++i) memcpy(saved[i],storage[i],sizeof(storage[i])); }
static void unchanged(void) { unsigned i;for(i=0;i<REGIONS;++i) CHECK(memcmp(saved[i],storage[i],sizeof(storage[i]))==0); }
static void guards(void) {
    unsigned i,j;for(i=0;i<REGIONS;++i) for(j=0;j<16;++j)
        CHECK(storage[i][j]==0xA5 && storage[i][sizes[i]+16+j]==0xA5);
}
static void refuse(int expected) { save();CHECK(dreadzone_animation_update(&request)==expected);unchanged();guards(); }
static void active(unsigned index,unsigned state,unsigned selector,unsigned generation) {
    u8 *record=region[RECORDS]+index*128;
    half(region[ACTOR]+0x6E,(u16)((generation<<6)|index));
    word(record+0x1C,bases[ACTOR]);record[0x10]=(u8)state;
    half(record+0xA,(u16)selector);half(record+0x58,(u16)generation);
}
int main(void) {
    unsigned phase,index,state;
    for(phase=0;phase<4;++phase) {
        setup();word(region[PHASE],phase);save();CHECK(dreadzone_animation_update(&request)==1);
        CHECK(readword(region[ACTOR]+0x44)==bits(.5f));CHECK(region[ACTOR][0x55]==0x20);
        CHECK(memcmp(saved[RECORDS],storage[RECORDS],sizeof(storage[RECORDS]))==0);guards();
        setup();region[ACTOR][0x57]=0;word(region[ACTOR]+0x40,0xFFFFFFFF);word(region[ACTOR]+0x44,0xFFFFFFFF);
        word(region[PHASE],phase);CHECK(dreadzone_animation_update(&request)==1);
        CHECK(readword(region[ACTOR]+0x44)==0xFFFFFFFF && region[ACTOR][0x55]==0);guards();
    }
    for(index=0;index<63;++index) for(state=0;state<8;++state) {
        u8 *record;
        setup();active(index,state,18,23);save();record=region[RECORDS]+index*128;
        CHECK(dreadzone_animation_update(&request)==1);
        CHECK(region[ACTOR][0x6E]==(state?255:(u8)((23<<6)|index)));
        CHECK(record[0x10]==(state==7?0:(state && state!=6?4:state)));
        CHECK(readword(record+0x1C)==(state==7?0:bases[ACTOR]));
        CHECK(readword(record+0x40)==(state==7?0:0xA5A5A5A5));
        if(index) CHECK(memcmp(saved[RECORDS]+16,region[RECORDS],index*128)==0);
        CHECK(memcmp(saved[RECORDS]+16+(index+1)*128,record+128,(62-index)*128)==0);guards();
    }
    setup();active(5,7,255,1023);CHECK(dreadzone_animation_update(&request)==1);
    CHECK(readword(region[RECORDS]+5*128+0x1C)==bases[ACTOR]);CHECK(region[ACTOR][0x6E]==0xC5);
    setup();active(5,7,255,23);half(region[RECORDS]+5*128+0x58,0xFFFF);
    CHECK(dreadzone_animation_update(&request)==1);CHECK(region[RECORDS][5*128+0x10]==7);
    setup();active(5,7,255,23);half(region[ACTOR]+0x34,0x40);CHECK(dreadzone_animation_update(&request)==1);
    CHECK(region[RECORDS][5*128+0x10]==0);
    setup();active(5,7,255,23);half(region[ACTOR]+0x74,0x8000);CHECK(dreadzone_animation_update(&request)==1);
    CHECK(region[RECORDS][5*128+0x10]==0);
    setup();region[ACTOR][0x6D]=18;refuse(DREADZONE_ANIMATION_UPDATE_NEEDS_626D50);
    setup();region[ACTOR][0x6D]=18;half(region[ACTOR]+0x36,4);CHECK(dreadzone_animation_update(&request)==1);
    setup();word(region[MODE],1);refuse(DREADZONE_ANIMATION_UPDATE_NEEDS_665A28);
    setup();word(region[MODE],1);word(region[PHASE],1);CHECK(dreadzone_animation_update(&request)==1);
    setup();word(region[MODE],1);word(region[PHASE],1);active(2,0,18,23);refuse(DREADZONE_ANIMATION_UPDATE_NEEDS_665A28);
    setup();region[ACTOR][0x6C]=2;half(region[EVENTS],77);half(region[EVENTS]+2,8);
    half(region[EVENTS]+4,78);half(region[EVENTS]+6,9);refuse(DREADZONE_ANIMATION_UPDATE_NEEDS_626D50);
    setup();region[ACTOR][0x6C]=2;half(region[EVENTS],77);half(region[EVENTS]+2,0);
    half(region[EVENTS]+4,78);half(region[EVENTS]+6,9);CHECK(dreadzone_animation_update(&request)==1);
    setup();region[ACTOR][0x6C]=1;half(region[EVENTS]+2,8);word(region[ACTOR]+0x44,bits(.5f));
    CHECK(dreadzone_animation_update(&request)==1); /* Equality at old time does not dispatch. */
    setup();region[ACTOR][0x6C]=255;word(region[DEFINITION]+0x14,0xFFFFFFFF);word(region[ACTOR]+0x48,0);
    CHECK(dreadzone_animation_update(&request)==1); /* Zero rate never reads event data. */
    setup();region[ACTOR][0x6C]=255;word(region[DEFINITION]+0x14,0xFFFFFFFF);
    word(region[ACTOR]+0x44,bits(.5f));word(region[ACTOR]+0x48,bits(-2));CHECK(dreadzone_animation_update(&request)==1);
    setup();region[ACTOR][0x6C]=1;word(region[ACTOR]+0x44,bits(29.5f));half(region[EVENTS]+2,472);
    CHECK(dreadzone_animation_update(&request)==1);CHECK(readword(region[ACTOR]+0x44)==0); /* Original wrap does not synthesize events. */
    setup();views[PHASE].bytes=0;refuse(DREADZONE_ANIMATION_UPDATE_INVALID);
    setup();views[MODE].guest_base=0x510100;refuse(DREADZONE_ANIMATION_UPDATE_INVALID);
    setup();word(region[POINTER],0);refuse(DREADZONE_ANIMATION_UPDATE_INVALID);
    setup();active(62,7,18,23);views[RECORDS].bytes=62*128;refuse(DREADZONE_ANIMATION_UPDATE_INVALID);
    setup();region[ACTOR][0x6C]=2;half(region[EVENTS]+2,9);views[EVENTS].bytes=4;refuse(DREADZONE_ANIMATION_UPDATE_INVALID);
    setup();views[ACTOR].guest_base=0xFFFFFF80;request.actor_guest=0xFFFFFF80;refuse(DREADZONE_ANIMATION_UPDATE_INVALID);
    setup();request.actor_guest=0x400001;refuse(DREADZONE_ANIMATION_UPDATE_INVALID);
    setup();word(region[ACTOR]+0x44,0x7FC00000);refuse(DREADZONE_ANIMATION_UPDATE_UNSUPPORTED);
    /* Actual release metadata aliases event count. The wrapper must reload
     * actor6C after the complete layer advance, rather than dispatch an event
     * using the input count captured before the release. */
    setup();views[ACTOR].guest_base=0x2A0000;request.actor_guest=0x2A0000;
    region[ACTOR][0x20]=0;region[ACTOR][0x31]=1;region[ACTOR][0x6C]=1;
    word(region[ACTOR]+0x50,bases[NODES]);half(region[EVENTS]+2,8);
    word(region[NODES],bases[DEFINITION]);word(region[NODES]+4,0);word(region[NODES]+8,bits(1));
    half(region[NODES]+0xC,2);half(region[NODES]+0xE,(u16)((0x2A006C-0x293004)/8));
    word(region[NODES]+0x10,0);region[NODES][0x15]=4;
    word(region[NODES]+0x18,bits(.25f));word(region[NODES]+0x1C,bits(.5f));
    CHECK(dreadzone_animation_update(&request)==1);CHECK(region[ACTOR][0x6C]==0);
    CHECK(readword(region[ACTOR]+0x50)==0 && region[OCCUPANCY][0]==0);
    for(index=0;index<32;++index) CHECK(region[NODES][index]==0);guards();
    /* A later failure rolls back that same genuine clear and metadata write. */
    setup();views[ACTOR].guest_base=0x2A0000;request.actor_guest=0x2A0000;
    region[ACTOR][0x20]=0;region[ACTOR][0x31]=1;region[ACTOR][0x6C]=1;
    word(region[ACTOR]+0x50,bases[NODES]);word(region[MODE],1);half(region[EVENTS]+2,8);
    word(region[NODES],bases[DEFINITION]);word(region[NODES]+4,0);word(region[NODES]+8,bits(1));
    half(region[NODES]+0xC,2);half(region[NODES]+0xE,(u16)((0x2A006C-0x293004)/8));
    word(region[NODES]+0x10,0);region[NODES][0x15]=4;
    word(region[NODES]+0x18,bits(.25f));word(region[NODES]+0x1C,bits(.5f));
    refuse(DREADZONE_ANIMATION_UPDATE_NEEDS_665A28);
    printf("%u animation update checks, %u failures\n",checks,failures);return failures?1:0;
}
