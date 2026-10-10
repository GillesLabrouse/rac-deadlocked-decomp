#include "dreadzone_actor_animation.h"
#include <stdio.h>
#include <string.h>
#include <fenv.h>
static unsigned checks,failures;
#define CHECK(x) do { ++checks; if(!(x)) { ++failures; if(failures<9) printf("line%d: %s\n",__LINE__,#x); } } while(0)
enum { RECORD=0,ACTOR,POOL,OCCUPANCY,METADATA,DEFINITION,FRAMES,FAR_FRAME,REGIONS };
static u8 storage[REGIONS][0x3230],expected[REGIONS][0x3230];
static const u32 bytes[REGIONS]={0x31F0,0x100,0x400,32,0x808,0x500,0x100,2};
static const u32 addresses[REGIONS]={0x368F60,0x222D40,0x28EB40,0x28EF40,0x292FF8,0x260000,0x270000,0x4270020};
static u8 *region[REGIONS];
static u32 padding;
static DreadzoneAnimationMemory views[REGIONS];
static DreadzoneAnimationContext context;
static u32 readword(const u8 *p) { return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24); }
static void word(u8 *p,u32 n) { p[0]=(u8)n;p[1]=(u8)(n>>8);p[2]=(u8)(n>>16);p[3]=(u8)(n>>24); }
static void half(u8 *p,u16 n) { p[0]=(u8)n;p[1]=(u8)(n>>8); }
static u32 exact_float(u32 value) {
    u32 top=0,bit=value,significand;
    if(!value) return 0;
    while(bit>1) { bit>>=1;top++; }
    significand=top>23?value>>(top-23):value<<(23-top);
    return ((top+127)<<23)|(significand&0x7FFFFF);
}
static void setup(unsigned seed,s32 animation,s32 frame) {
    unsigned r,i;
    u32 slot=seed%32,head=seed&1?addresses[POOL]+slot*32:0;
    s32 metadata_index=seed%17==0?-1:(s32)(seed%256);
    padding=16+seed%16;
    for(r=0;r<REGIONS;r++) {
        for(i=0;i<bytes[r]+64;i++) storage[r][i]=(u8)(seed*31+i*13+r*7+(i>>8));
        region[r]=storage[r]+padding;
        views[r].storage=region[r];views[r].guest_base=addresses[r];views[r].bytes=bytes[r];
    }
    word(region[RECORD]+0x2EF0,addresses[ACTOR]); word(region[ACTOR]+0x24,addresses[DEFINITION]);
    word(region[ACTOR]+0x50,head);
    region[POOL][slot*32+0x15]=(u8)(seed&2?4:0);
    half(region[POOL]+slot*32+0xE,(u16)metadata_index);
    word(region[POOL]+slot*32+0x10,(seed&4)?head:0xFFFFFFFF);
    word(region[DEFINITION]+0x48+(u32)animation*4,addresses[FRAMES]);
    if(frame==16777216) half(region[FAR_FRAME],(u16)(0xB000+seed));
    else half(region[FRAMES]+0x20+(u32)frame*4,(u16)(0xB000+seed));
    context.record=region[RECORD];context.record_guest=addresses[RECORD];context.record_bytes=bytes[RECORD];
    context.views=views+1;context.view_count=REGIONS-1;
}
static void save(void) {
    unsigned r;
    for(r=0;r<REGIONS;r++) memcpy(expected[r],storage[r],bytes[r]+64);
}
static void oracle(s32 animation,s32 frame) {
    u8 *actor=expected[ACTOR]+padding;
    u32 head=readword(region[ACTOR]+0x50);
    if(head) {
        u32 slot=(head-addresses[POOL])/32;
        const u8 *node=region[POOL]+slot*32;
        if(node[0x15]&4) {
            s32 index=(s32)(((u32)node[0xE]|((u32)node[0xF]<<8))^0x8000U)-0x8000;
            expected[METADATA][padding+12+index*8]=0;
        }
        expected[OCCUPANCY][padding+slot]=0;
        memset(expected[POOL]+padding+slot*32,0,32);
    }
    actor[0x54]=(u8)animation;word(actor+0x50,0);word(actor+0x44,exact_float((u32)frame));
    word(actor+0x48,0x3F800000);word(actor+0x40,addresses[FRAMES]);
    if(frame==16777216) memcpy(actor+0x4C,region[FAR_FRAME],2);
    else memcpy(actor+0x4C,region[FRAMES]+0x20+(u32)frame*4,2);
}
static void verify(void) {
    unsigned r,i;
    for(r=0;r<REGIONS;r++) for(i=0;i<bytes[r]+64;i++) CHECK(storage[r][i]==expected[r][i]);
}
static void rejected(s32 animation,s32 frame,int status) {
    save();CHECK(dreadzone_animation_00688FE0(&context,animation,frame)==status);verify();
}
static void rejected_storage(const DreadzoneAnimationContext *host) {
    save();
    CHECK(dreadzone_animation_00688FE0(host,0,0)==DREADZONE_ANIMATION_INVALID_STORAGE);
    CHECK(dreadzone_animation_0058A8D8(host,addresses[POOL])==DREADZONE_ANIMATION_INVALID_STORAGE);
    CHECK(dreadzone_animation_0058B6B0(host,addresses[POOL])==DREADZONE_ANIMATION_INVALID_STORAGE);
    CHECK(dreadzone_animation_validate(host)==0);
    CHECK(dreadzone_animation_translate(host,addresses[ACTOR],1)==0);
    verify();
}
int main(void) {
    unsigned seed;
    static const s32 animations[]={0,1,10,255};
    static const s32 frames[]={0,1,7,31,16777216};
    for(seed=0;seed<640;seed++) {
        s32 animation=animations[(seed/8)%4],frame=frames[(seed/32)%5];
        setup(seed,animation,frame);save();oracle(animation,frame);
        CHECK(dreadzone_animation_00688FE0(&context,animation,frame)==0);verify();
    }
    /* Signed metadata index and direct release helpers execute real writes. */
    setup(17,0,0);save();expected[METADATA][padding+4]=0;
    CHECK(dreadzone_animation_0058B6B0(&context,addresses[POOL]+17*32)==0);verify();
    setup(19,0,0);save();memset(expected[POOL]+padding+19*32,0,32);
    expected[OCCUPANCY][padding+19]=0;expected[METADATA][padding+12+19*8]=0;
    CHECK(dreadzone_animation_0058A8D8(&context,addresses[POOL]+19*32)==0);verify();
    setup(1,0,0);rejected(-1,0,DREADZONE_ANIMATION_UNSUPPORTED_ARGUMENT);
    setup(1,0,0);rejected(256,0,DREADZONE_ANIMATION_UNSUPPORTED_ARGUMENT);
    setup(1,0,0);rejected(0,-1,DREADZONE_ANIMATION_UNSUPPORTED_ARGUMENT);
    setup(1,0,0);rejected(0,16777217,DREADZONE_ANIMATION_UNSUPPORTED_ARGUMENT);
    setup(1,0,0);context.record_bytes--;rejected(0,0,DREADZONE_ANIMATION_INVALID_STORAGE);
    setup(1,0,0);context.record_guest=0xFFFFD000;rejected(0,0,DREADZONE_ANIMATION_INVALID_STORAGE);
    setup(1,0,0);word(region[RECORD]+0x2EF0,0);rejected(0,0,DREADZONE_ANIMATION_NEEDS_ACTOR);
    setup(1,0,0);word(region[RECORD]+0x2EF0,addresses[ACTOR]+1);rejected(0,0,DREADZONE_ANIMATION_NEEDS_ACTOR);
    setup(1,0,0);views[ACTOR].bytes--;rejected(0,0,DREADZONE_ANIMATION_NEEDS_ACTOR);
    setup(1,0,0);word(region[ACTOR]+0x50,addresses[POOL]+1);rejected(0,0,DREADZONE_ANIMATION_NEEDS_NODE_POOL);
    setup(1,0,0);word(region[ACTOR]+0x50,addresses[POOL]+0x400);rejected(0,0,DREADZONE_ANIMATION_NEEDS_NODE_POOL);
    setup(1,0,0);views[OCCUPANCY].guest_base+=32;rejected(0,0,DREADZONE_ANIMATION_NEEDS_NODE_POOL);
    setup(3,0,0);views[METADATA].guest_base+=0x1000;rejected(0,0,DREADZONE_ANIMATION_NEEDS_METADATA);
    setup(1,0,0);word(region[ACTOR]+0x24,0);rejected(0,0,DREADZONE_ANIMATION_NEEDS_DEFINITION);
    setup(1,0,0);word(region[ACTOR]+0x24,0xFFFFFFC0);rejected(0,0,DREADZONE_ANIMATION_NEEDS_DEFINITION);
    setup(1,0,0);word(region[DEFINITION]+0x48,0xFFFFFFE0);rejected(0,0,DREADZONE_ANIMATION_NEEDS_FRAME_DATA);
    setup(1,0,0);word(region[DEFINITION]+0x48,0);rejected(0,0,DREADZONE_ANIMATION_NEEDS_FRAME_DATA);
    setup(1,0,0);views[FRAMES].bytes=0x21;rejected(0,0,DREADZONE_ANIMATION_NEEDS_FRAME_DATA);
    setup(1,0,0);views[DEFINITION].storage=region[ACTOR];rejected(0,0,DREADZONE_ANIMATION_INVALID_STORAGE);
    setup(1,0,0);views[DEFINITION].guest_base=addresses[ACTOR];rejected(0,0,DREADZONE_ANIMATION_INVALID_STORAGE);
    setup(1,0,0);word(region[ACTOR]+0x24,addresses[ACTOR]+0xC);
    word(region[ACTOR]+0x54,addresses[FRAMES]+4);rejected(0,0,DREADZONE_ANIMATION_INVALID_STORAGE);
    setup(1,0,0);word(region[ACTOR]+0x24,addresses[POOL]+32-0x2C);
    word(region[POOL]+32+0x1C,addresses[FRAMES]);rejected(0,0,DREADZONE_ANIMATION_INVALID_STORAGE);
    setup(1,0,0);word(region[DEFINITION]+0x48,addresses[POOL]+32-0x1C);
    rejected(0,0,DREADZONE_ANIMATION_INVALID_STORAGE);
    setup(1,0,0);context.view_count=65;rejected(0,0,DREADZONE_ANIMATION_INVALID_STORAGE);
    /* Numeric wrapping ranges are rejected without dereferencing fake host
     * addresses. Every entry point remains atomic on legitimate allocations. */
    setup(3,0,0);views[POOL].storage=(u8 *)(UINTPTR_MAX-15);rejected_storage(&context);
    setup(3,0,0);context.record=(const u8 *)(UINTPTR_MAX-15);rejected_storage(&context);
    setup(3,0,0);context.views=(const DreadzoneAnimationMemory *)(UINTPTR_MAX-7);
    rejected_storage(&context);
    setup(3,0,0);rejected_storage((const DreadzoneAnimationContext *)(UINTPTR_MAX-7));
    /* Descriptors are real host objects, independent from guest byte layouts.
     * A mapped clear must not erase the caller's live context or view array. */
    {
        union { DreadzoneAnimationContext context;u8 bytes[0x400]; } alias_context;
        union { DreadzoneAnimationMemory views[REGIONS];u8 bytes[0x400]; } alias_views;
        u8 before[0x400];
        memset(&alias_context,0xA6,sizeof(alias_context));
        memset(&alias_views,0xB8,sizeof(alias_views));
        setup(1,0,0);word(region[ACTOR]+0x50,addresses[POOL]);
        alias_context.context=context;views[POOL].storage=alias_context.bytes;
        memcpy(before,alias_context.bytes,sizeof(before));save();
        CHECK(dreadzone_animation_00688FE0(&alias_context.context,0,0)==DREADZONE_ANIMATION_INVALID_STORAGE);
        CHECK(dreadzone_animation_0058A8D8(&alias_context.context,addresses[POOL])==DREADZONE_ANIMATION_INVALID_STORAGE);
        CHECK(dreadzone_animation_0058B6B0(&alias_context.context,addresses[POOL])==DREADZONE_ANIMATION_INVALID_STORAGE);
        CHECK(memcmp(before,alias_context.bytes,sizeof(before))==0);verify();
        setup(1,0,0);memcpy(alias_views.views,views,sizeof(views));alias_views.views[POOL].storage=alias_views.bytes;
        context.views=alias_views.views+1;
        memcpy(before,alias_views.bytes,sizeof(before));save();
        CHECK(dreadzone_animation_00688FE0(&context,0,0)==DREADZONE_ANIMATION_INVALID_STORAGE);
        CHECK(dreadzone_animation_0058A8D8(&context,addresses[POOL]+32)==DREADZONE_ANIMATION_INVALID_STORAGE);
        CHECK(dreadzone_animation_0058B6B0(&context,addresses[POOL]+32)==DREADZONE_ANIMATION_INVALID_STORAGE);
        CHECK(memcmp(before,alias_views.bytes,sizeof(before))==0);verify();
    }
    {
        static const int rounding[]={FE_TONEAREST,FE_DOWNWARD,FE_UPWARD,FE_TOWARDZERO};
        unsigned r,f;
        int saved_round=fegetround();
        for(r=0;r<4;r++) {
            CHECK(fesetround(rounding[r])==0);
            for(f=0;f<5;f++) {
                setup(19,0,frames[f]);save();oracle(0,frames[f]);
                CHECK(dreadzone_animation_00688FE0(&context,0,frames[f])==0);verify();
            }
        }
        CHECK(fesetround(saved_round)==0);
    }
    CHECK(dreadzone_animation_00688FE0(0,0,0)==DREADZONE_ANIMATION_INVALID_STORAGE);
    printf("Actor animation: %u checks, %u failures. Real head-only release and exact finite integer frames verified.\n",checks,failures);
    return failures?1:0;
}
