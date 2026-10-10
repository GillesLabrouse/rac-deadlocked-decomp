#include "dreadzone_animation_layers.h"
#include <stdio.h>
#include <string.h>

enum { ACTOR,POOL,OCCUPANCY,METADATA,DEFINITIONS,REGIONS };
static u8 storage[REGIONS][0x1040],saved[REGIONS][0x1040];
static const u32 addresses[REGIONS]={0x222D40,0x28EB40,0x28EF40,0x292FF8,0x500000};
static const u32 sizes[REGIONS]={0x100,0x400,32,0x808,0x1000};
static u8 *region[REGIONS];
static DreadzoneAnimationMemory views[REGIONS];
static DreadzoneAnimationContext memory;
static DreadzoneAnimationLayersRequest request;
static unsigned checks,failures;
#define CHECK(x) do { ++checks;if(!(x)) { ++failures;fprintf(stderr,"line%d: %s\n",__LINE__,#x); } } while(0)
static void word(u8 *p,u32 v) { p[0]=(u8)v;p[1]=(u8)(v>>8);p[2]=(u8)(v>>16);p[3]=(u8)(v>>24); }
static void half(u8 *p,u16 v) { p[0]=(u8)v;p[1]=(u8)(v>>8); }
static u32 readword(const u8 *p) { return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24); }
static u32 bits(f32 value) { u32 result;memcpy(&result,&value,4);return result; }
static f32 number(const u8 *p) { u32 raw=readword(p);f32 result;memcpy(&result,&raw,4);return result; }
static void setup(unsigned nodes) {
    unsigned r,i;
    for(r=0;r<REGIONS;++r) {
        memset(storage[r],0xA5,sizeof(storage[r]));region[r]=storage[r]+16;
        views[r].storage=region[r];views[r].guest_base=addresses[r];views[r].bytes=sizes[r];
    }
    memory.record=0;memory.record_guest=0;memory.record_bytes=0;memory.views=views;memory.view_count=REGIONS;
    request.memory=&memory;request.actor_guest=addresses[ACTOR];
    request.selected_definition_guest=addresses[DEFINITIONS];request.effective_rate_bits=bits(1);
    region[ACTOR][0x20]=0;region[ACTOR][0x31]=1;
    word(region[ACTOR]+0x40,addresses[DEFINITIONS]);word(region[ACTOR]+0x44,0);
    word(region[ACTOR]+0x48,bits(1));half(region[ACTOR]+0x4C,2);
    word(region[ACTOR]+0x50,nodes?addresses[POOL]:0);region[ACTOR][0x55]=0;
    for(i=0;i<33;++i) {
        u8 *definition=region[DEFINITIONS]+i*64;
        definition[0x10]=3;half(definition+0x20,2);half(definition+0x24,4);half(definition+0x28,8);
    }
    for(i=0;i<nodes;++i) {
        u8 *node=region[POOL]+i*32;
        word(node,addresses[DEFINITIONS]+(i+1)*64);word(node+4,0);word(node+8,bits(1));half(node+0xC,2);
        half(node+0xE,(u16)i);word(node+0x10,i+1<nodes?addresses[POOL]+(i+1)*32:0);
        node[0x15]=0;word(node+0x18,bits(1));word(node+0x1C,0);
    }
}
static void save(void) { unsigned r;for(r=0;r<REGIONS;++r) memcpy(saved[r],storage[r],sizeof(storage[r])); }
static void unchanged(void) { unsigned r;for(r=0;r<REGIONS;++r) CHECK(memcmp(saved[r],storage[r],sizeof(storage[r]))==0); }
static void guards(void) {
    unsigned r,i;
    for(r=0;r<REGIONS;++r) for(i=0;i<16;++i) CHECK(storage[r][i]==0xA5&&storage[r][sizes[r]+16+i]==0xA5);
}
static void refuse(int status) { save();CHECK(dreadzone_animation_layers_advance(&request)==status);unchanged(); }
int main(void) {
    unsigned n,i;
    for(n=0;n<=32;++n) {
        setup(n);save();CHECK(dreadzone_animation_layers_advance(&request)==1);
        CHECK(number(region[ACTOR]+0x44)==0.5f);
        CHECK(readword(region[ACTOR]+0x50)==(n?addresses[POOL]:0));
        for(i=0;i<n;++i) {
            u8 *node=region[POOL]+i*32;
            CHECK(number(node+4)==0.5f&&node[0x15]==0x20);
            CHECK(readword(node+0x10)==(i+1<n?addresses[POOL]+(i+1)*32:0));
            CHECK(number(node+0x18)==1&&readword(node+0x1C)==0);
        }
        CHECK(memcmp(saved[OCCUPANCY],storage[OCCUPANCY],sizeof(storage[OCCUPANCY]))==0);
        CHECK(memcmp(saved[DEFINITIONS],storage[DEFINITIONS],sizeof(storage[DEFINITIONS]))==0);
        guards();
    }
    setup(2);region[POOL][0x15]=4;word(region[POOL]+0x18,bits(.25f));word(region[POOL]+0x1C,bits(.5f));
    word(region[POOL]+32+0x18,bits(.75f));word(region[POOL]+32+0x1C,bits(-.5f));
    CHECK(dreadzone_animation_layers_advance(&request)==1);
    CHECK(readword(region[ACTOR]+0x50)==addresses[POOL]+32);
    for(i=0;i<32;++i) CHECK(region[POOL][i]==0);
    CHECK(region[OCCUPANCY][0]==0&&region[METADATA][12]==0);
    CHECK(number(region[POOL]+32+0x18)==1&&readword(region[POOL]+32+0x1C)==0);
    setup(3);region[ACTOR][0x31]=0;
    CHECK(dreadzone_animation_layers_advance(&request)==1);
    CHECK(readword(region[ACTOR]+0x50)==0&&number(region[ACTOR]+0x44)==.5f);
    for(i=0;i<96;++i) CHECK(region[POOL][i]==0);
    setup(2);region[ACTOR][0x20]=0xFF;
    CHECK(dreadzone_animation_layers_advance(&request)==1);CHECK(readword(region[ACTOR]+0x50)==0);
    /* Metadata release changes a future decay BEFORE its weight update. */
    setup(2);word(region[POOL]+8,0);word(region[POOL]+32+8,0);
    region[POOL][0x15]=4;half(region[POOL]+0xE,(u16)((s32)(addresses[POOL]+32+0x1C-0x293004U)/8));
    word(region[POOL]+0x18,bits(1));word(region[POOL]+0x1C,bits(1));
    word(region[POOL]+32+0x18,0x3F800001);word(region[POOL]+32+0x1C,0x3F800001);
    CHECK(dreadzone_animation_layers_advance(&request)==1);
    CHECK(readword(region[ACTOR]+0x50)==addresses[POOL]+32);
    CHECK(readword(region[POOL]+32+0x1C)==0x3F800000);
    CHECK(number(region[POOL]+32+0x18)==1.1920928955078125e-7f);
    setup(1);word(region[POOL]+0x10,addresses[POOL]);refuse(-2);
    setup(1);region[ACTOR][0x31]=0;word(region[POOL]+0x10,addresses[POOL]);
    CHECK(dreadzone_animation_layers_advance(&request)==1);CHECK(readword(region[ACTOR]+0x50)==0);
    setup(2);word(region[POOL]+32+8,bits(-.5f));refuse(-2);
    setup(2);region[POOL][32+0x15]=4;half(region[POOL]+32+0xE,0x7FFF);
    word(region[POOL]+32+0x18,0);word(region[POOL]+32+0x1C,bits(1));refuse(-1);
    setup(1);word(region[POOL]+0x18,0x7FC00001);refuse(-2);
    setup(1);request.selected_definition_guest++;refuse(-1);
    setup(1);request.effective_rate_bits=0;refuse(-1);
    setup(1);word(region[POOL],addresses[POOL]-0x20);refuse(-1);
    setup(1);views[POOL].storage=(u8 *)(UINTPTR_MAX-15);refuse(-1);
    setup(1);views[DEFINITIONS].storage=(u8 *)(uintptr_t)0x1000;
    views[DEFINITIONS].bytes=32U*1024U*1024U+1U;refuse(-2);
    setup(1);memory.view_count=65;refuse(-1);
    CHECK(dreadzone_animation_layers_advance(
        (const DreadzoneAnimationLayersRequest *)(UINTPTR_MAX-7))==-1);
    /* A legitimate guest metadata write can erase the borrowed request after
     * capture; no post-publication descriptor read may follow it. */
    setup(1);region[POOL][0x15]=4;
    half(region[POOL]+0xE,(u16)((s32)(addresses[METADATA]+0x104-0x293004U)/8));
    word(region[POOL]+0x18,bits(1));word(region[POOL]+0x1C,bits(1));
    memcpy(region[METADATA]+0x100,&request,sizeof(request));
    CHECK(dreadzone_animation_layers_advance(
        (const DreadzoneAnimationLayersRequest *)(const void *)(region[METADATA]+0x100))==1);
    CHECK(region[METADATA][0x104]==0&&readword(region[ACTOR]+0x50)==0);
    CHECK(dreadzone_animation_layers_advance(0)==-1);
    printf("Animation layers: %u checks, %u failures.\n",checks,failures);
    return failures?1:0;
}
