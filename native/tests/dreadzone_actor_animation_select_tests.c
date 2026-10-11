#include "dreadzone_actor_animation_select.h"
#include <stdio.h>
#include <string.h>
enum { PLAYER,ACTOR,POOL,OCCUPANCY,OWNERS,CLASS,DEFINITIONS,WEAPONS,BLENDS,REGIONS };
static const u32 bases[REGIONS]={0x368F60,0x400000,0x28EB40,0x28EF40,0x28EF80,0x500000,0x510000,0x520000,0x220798};
static const u32 sizes[REGIONS]={0x31F0,256,1024,32,128,0x248,0x100,0x200,64};
static u8 storage[REGIONS][0x3230],saved[REGIONS][0x3230],*region[REGIONS];
static DreadzoneAnimationMemory views[REGIONS];
static DreadzoneAnimationContext memory;
static DreadzoneAnimationSelectRequest request;
static unsigned checks,failures;
#define CHECK(x) do { ++checks;if(!(x)) { ++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x); } } while(0)
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
    memset(&memory,0,sizeof(memory));memory.views=views;memory.view_count=REGIONS;
    request.memory=&memory;request.player_guest=bases[PLAYER];request.clip=5;request.frame=16;request.flags=0;request.blend_duration_bits=bits(8);
    word(region[PLAYER]+0x2EF0,bases[ACTOR]);word(region[PLAYER]+0x2404,2);
    word(region[PLAYER]+0x23E0,bases[WEAPONS]);word(region[PLAYER]+0x23E4,bases[WEAPONS]+256);
    word(region[PLAYER]+0x25CC,2);word(region[PLAYER]+0x2DD4,0);
    word(region[PLAYER]+0x7E8,0);word(region[PLAYER]+0x7EC,0);
    region[PLAYER][0x2658]=0;region[PLAYER][0x265A]=0;half(region[PLAYER]+0x2E2C,0);
    region[WEAPONS][0x54]=1;region[WEAPONS][256+0x54]=1;
    region[ACTOR][0x31]=1;region[ACTOR][0x20]=0;region[ACTOR][0x54]=0;region[ACTOR][0x55]=0x93;
    word(region[ACTOR]+0x24,bases[CLASS]);word(region[ACTOR]+0x40,0x530000);
    word(region[ACTOR]+0x44,bits(.5f));half(region[ACTOR]+0x4C,2);word(region[ACTOR]+0x50,0x28EFA0);
    memset(region[OCCUPANCY],0,sizes[OCCUPANCY]);region[CLASS][0xC]=128;
    for(i=0;i<128;++i) word(region[CLASS]+0x48+i*4,bases[DEFINITIONS]);
    region[DEFINITIONS][0x10]=50;region[DEFINITIONS][0x11]=255;region[DEFINITIONS][0x12]=0;
    for(i=0;i<50;++i) half(region[DEFINITIONS]+0x20+i*4,2);
    word(region[PLAYER]+0x650,3);word(region[BLENDS]+12,12);
}
static void save(void) { unsigned i;for(i=0;i<REGIONS;++i) memcpy(saved[i],storage[i],sizeof(storage[i])); }
static void guards(void) { unsigned i,j;for(i=0;i<REGIONS;++i) for(j=0;j<16;++j) CHECK(storage[i][j]==0xA5&&storage[i][sizes[i]+16+j]==0xA5); }
static void refuse(int status) {
    unsigned i;save();CHECK(dreadzone_actor_animation_select(&request)==status);
    for(i=0;i<REGIONS;++i) CHECK(memcmp(saved[i],storage[i],sizeof(storage[i]))==0);guards();
}
int main(void) {
    unsigned slot,i;
    for(slot=0;slot<32;++slot) {
        u8 *node;
        setup();memset(region[OCCUPANCY],9,slot);save();node=region[POOL]+slot*32;
        CHECK(dreadzone_actor_animation_select(&request)==1);
        CHECK(readword(region[ACTOR]+0x50)==bases[POOL]+slot*32);
        CHECK(region[OCCUPANCY][slot]==1&&readword(region[OWNERS]+slot*4)==bases[ACTOR]);
        CHECK(readword(node)==0x530000&&readword(node+4)==bits(.5f)&&readword(node+8)==0);
        CHECK(readword(node+0x10)==0x28EFA0&&node[0x14]==0&&node[0x15]==0x93);
        CHECK(node[0xE]==0xA5&&node[0xF]==0xA5&&node[0x16]==255&&node[0x17]==255);
        CHECK(readword(node+0x18)==bits(.875f)&&readword(node+0x1C)==bits(.125f));
        CHECK(region[ACTOR][0x54]==5&&readword(region[ACTOR]+0x44)==bits(16));
        CHECK(region[ACTOR][0x55]==0x83&&region[ACTOR][0x6D]==255&&region[ACTOR][0x6C]==0);
        CHECK(readword(region[PLAYER]+0x640)==bits(1)&&readword(region[PLAYER]+0x644)==2);
        CHECK(readword(region[PLAYER]+0x7F4)==0&&region[PLAYER][0x265A]==0);
        CHECK(memcmp(saved[WEAPONS],storage[WEAPONS],sizeof(storage[WEAPONS]))==0);
        for(i=0;i<32;++i) if(i!=slot) {
            CHECK(memcmp(saved[POOL]+16+i*32,region[POOL]+i*32,32)==0);
            CHECK(region[OCCUPANCY][i]==saved[OCCUPANCY][16+i]);
            CHECK(memcmp(saved[OWNERS]+16+i*4,region[OWNERS]+i*4,4)==0);
        }
        guards();
    }
    setup();memset(region[OCCUPANCY],1,32);request.blend_duration_bits=0;save();
    CHECK(dreadzone_actor_animation_select(&request)==1);CHECK(readword(region[ACTOR]+0x50)==0x28EFA0);
    CHECK(memcmp(saved[POOL],storage[POOL],sizeof(storage[POOL]))==0);
    setup();region[ACTOR][0x31]=0;request.blend_duration_bits=0;CHECK(dreadzone_actor_animation_select(&request)==1);
    setup();region[ACTOR][0x20]=0x80;request.blend_duration_bits=0;CHECK(dreadzone_actor_animation_select(&request)==1);
    setup();request.blend_duration_bits=0;refuse(DREADZONE_ANIMATION_SELECT_UNSUPPORTED);
    setup();request.clip=6;request.frame=15;CHECK(dreadzone_actor_animation_select(&request)==1);CHECK(readword(region[ACTOR]+0x44)==bits(15));
    setup();request.clip=9;request.frame=12;region[DEFINITIONS][0x11]=18;region[DEFINITIONS][0x12]=2;
    CHECK(dreadzone_actor_animation_select(&request)==1);CHECK(region[ACTOR][0x6D]==18&&region[ACTOR][0x6C]==2);
    CHECK(region[ACTOR][0x55]&0x10);
    setup();request.blend_duration_bits=bits(-1);CHECK(dreadzone_actor_animation_select(&request)==1);
    CHECK(readword(region[POOL]+0x1C)==0x3DAAAAAA);
    setup();request.frame=2;refuse(DREADZONE_ANIMATION_SELECT_NEEDS_5A36B0);
    setup();request.frame=2;request.flags=8;CHECK(dreadzone_actor_animation_select(&request)==1);
    setup();region[WEAPONS][0x54]=2;refuse(DREADZONE_ANIMATION_SELECT_NEEDS_5919B8);
    setup();region[WEAPONS][256+0x54]=2;refuse(DREADZONE_ANIMATION_SELECT_NEEDS_5919B8);
    setup();word(region[PLAYER]+0x7EC,bases[POOL]+64);refuse(DREADZONE_ANIMATION_SELECT_NEEDS_58AA90);
    setup();region[PLAYER][0x265A]=1;word(region[PLAYER]+0x7EC,0xFFFFFFFF);
    CHECK(dreadzone_actor_animation_select(&request)==1);CHECK(readword(region[PLAYER]+0x7EC)==0xFFFFFFFF);
    setup();word(region[PLAYER]+0x7E8,bases[POOL]+64);region[PLAYER][0x265A]=1;refuse(DREADZONE_ANIMATION_SELECT_NEEDS_58AA90);
    setup();region[CLASS][0xC]=5;refuse(DREADZONE_ANIMATION_SELECT_UNSUPPORTED);
    setup();request.frame=50;refuse(DREADZONE_ANIMATION_SELECT_UNSUPPORTED);
    setup();word(region[ACTOR]+0x24,bases[CLASS]+1);refuse(DREADZONE_ANIMATION_SELECT_INVALID);
    setup();word(region[CLASS]+0x48+5*4,bases[DEFINITIONS]+1);refuse(DREADZONE_ANIMATION_SELECT_INVALID);
    setup();views[OWNERS].bytes=3;refuse(DREADZONE_ANIMATION_SELECT_INVALID);
    setup();views[DEFINITIONS].bytes=0x60;refuse(DREADZONE_ANIMATION_SELECT_INVALID);
    setup();word(region[PLAYER]+0x23E4,0xFFFFFFF0);refuse(DREADZONE_ANIMATION_SELECT_INVALID);
    setup();views[PLAYER].guest_base=0xFFFFFF80;request.player_guest=0xFFFFFF80;refuse(DREADZONE_ANIMATION_SELECT_INVALID);
    setup();request.blend_duration_bits=0x7FC00000;refuse(DREADZONE_ANIMATION_SELECT_UNSUPPORTED);
    /* Aligned class entry aliases actor6C..6F. The first metadata store at
     *6D changes that entry's second byte; retail reloads the pointer before
     *reading event count. This fixture must read the second real definition. */
    setup();word(region[ACTOR]+0x24,bases[ACTOR]+0x10);region[ACTOR][0x1C]=128;
    word(region[ACTOR]+0x6C,bases[DEFINITIONS]);views[DEFINITIONS].bytes=0x200;
    region[DEFINITIONS][0x11]=1;region[DEFINITIONS][0x112]=7;
    CHECK(dreadzone_actor_animation_select(&request)==1);
    CHECK(region[ACTOR][0x6D]==1&&region[ACTOR][0x6C]==7);
    CHECK(readword(region[ACTOR]+0x40)==bases[DEFINITIONS]);
    /* The same intermediate alias remains uncommitted on later refusal. */
    setup();word(region[ACTOR]+0x24,bases[ACTOR]+0x10);region[ACTOR][0x1C]=128;
    word(region[ACTOR]+0x6C,bases[DEFINITIONS]);views[DEFINITIONS].bytes=0x200;
    region[DEFINITIONS][0x11]=1;region[DEFINITIONS][0x112]=7;region[WEAPONS][0x54]=2;
    save();CHECK(dreadzone_actor_animation_select(&request)==DREADZONE_ANIMATION_SELECT_NEEDS_5919B8);
    for(i=0;i<REGIONS;++i) CHECK(memcmp(saved[i],storage[i],sizeof(storage[i]))==0);
    printf("%u animation selection checks, %u failures\n",checks,failures);return failures?1:0;
}
