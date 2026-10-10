#include "dreadzone_animation_replacement.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static unsigned checks,failures;
#define CHECK(x) do { checks++;if(!(x)) { failures++;printf("line %d: %s\n",__LINE__,#x); } } while(0)
static void put(u8 *p,u32 v,u32 n) { u32 i;for(i=0;i<n;i++) p[i]=(u8)(v>>(i*8)); }
static u32 get(const u8 *p,u32 n) { u32 i,v=0;for(i=0;i<n;i++) v|=(u32)p[i]<<(i*8);return v; }
static u32 random_value(u32 *state) { *state=*state*1664525U+1013904223U;return *state; }

/* Separate byte-level expected effects, including ordered source/destination
 * aliases. The private oracle additionally compares actual retail instructions. */
static void reference(u8 *memory,u32 destination,u32 source,u32 begin,u32 end) {
    u32 i,flag=memory[destination+0x47]&8,actor,flags;
    static const u32 offsets[]={0xC,0xD,0x10,0x1C,0x28,0x44};
    static const u32 widths[]={1,1,4,4,4,2};
    for(i=0;i<6;i++) put(memory+destination+offsets[i],get(memory+source+offsets[i],widths[i]),widths[i]);
    memory[destination+0x47]=(u8)((memory[source+0x47]&0xF7)|flag);
    memory[destination+0x46]=memory[source+0x46];
    for(i=0;i<memory[source+0xC];i++) put(memory+destination+0x48+i*4,get(memory+source+0x48+i*4,4),4);
    for(actor=begin;actor<end;actor+=256) {
        if(!get(memory+actor+0x24,4) || memory[actor+0x20]>=0xFD || get(memory+actor+0x24,4)!=0x400000+destination) continue;
        flags=get(memory+actor+0x36,2);
        if(flags&8) { if(!(memory[destination+0x47]&8)) put(memory+actor+0x36,flags&0xFFF7,2); }
        else if(memory[destination+0x47]&8) put(memory+actor+0x36,flags|8,2);
    }
}
int main(void) {
    u8 memory[0x3000],expected[0x3000],saved[0x3000];
    DreadzoneAnimationReplacementView v={memory,0x400000,sizeof(memory)};
    DreadzoneAnimationReplacement request={&v,1,0x400800,0x400100,0x402000,0x402800};
    u32 case_index,i,seed=0x575A18,source,destination;
    for(case_index=0;case_index<256;case_index++) {
        for(i=0;i<sizeof(memory);i++) memory[i]=(u8)(random_value(&seed)>>24);
        static const s32 offsets[]={0,4,-4,0x700,-0x3C,0x3C,-0x44,-0x40};
        source=0x100;destination=(u32)((s32)source+offsets[case_index%8]);
        memory[source+0xC]=(u8)case_index;
        request.source=0x400000+source;request.destination=0x400000+destination;
        for(i=0;i<8;i++) {
            put(memory+0x2024+i*256,i%3==0 ? 0 : i%3==1 ? request.destination : request.source,4);
            memory[0x2020+i*256]=(u8)(0xFA+i);
        }
        memcpy(expected,memory,sizeof(memory));reference(expected,destination,source,0x2000,0x2800);
        CHECK(dreadzone_animation_replace(&request)==DREADZONE_ANIMATION_REPLACED);
        for(i=0;i<sizeof(memory);i++) CHECK(memory[i]==expected[i]);
    }
    memcpy(saved,memory,sizeof(memory));
    v.bytes=0x1000;
    CHECK(dreadzone_animation_replace(&request)==DREADZONE_ANIMATION_REPLACEMENT_INVALID);
    CHECK(memcmp(memory,saved,sizeof(memory))==0);
    v.bytes=sizeof(memory);request.actors_end++;
    CHECK(dreadzone_animation_replace(&request)==DREADZONE_ANIMATION_REPLACEMENT_INVALID);
    CHECK(memcmp(memory,saved,sizeof(memory))==0);
    request.actors_end--;request.destination=UINT32_MAX-100;
    CHECK(dreadzone_animation_replace(&request)==DREADZONE_ANIMATION_REPLACEMENT_INVALID);
    CHECK(memcmp(memory,saved,sizeof(memory))==0);
    request.destination=0;
    CHECK(dreadzone_animation_replace(&request)==DREADZONE_ANIMATION_REPLACED);
    CHECK(memcmp(memory,saved,sizeof(memory))==0);
    /* Borrowed request/view descriptors can live inside mutated storage. */
    request.destination=0x400800;request.source=0x400100;request.actors_end=request.actors_begin;
    memory[0x10C]=32;
    memcpy(memory+0x840,&v,sizeof(v));
    request.views=(const DreadzoneAnimationReplacementView *)(void *)(memory+0x840);
    memcpy(memory+0x800,&request,sizeof(request));
    memcpy(expected,memory,sizeof(memory));reference(expected,0x800,0x100,0,0);
    CHECK(dreadzone_animation_replace((const DreadzoneAnimationReplacement *)(const void *)(memory+0x800))==DREADZONE_ANIMATION_REPLACED);
    CHECK(memcmp(memory,expected,sizeof(memory))==0);
    printf("%u checks, %u failures\n",checks,failures);return failures!=0;
}
