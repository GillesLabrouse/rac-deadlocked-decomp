#include "dreadzone_player_state_entry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { PLAYER=0x368F60,BODY=0x400000,CLASS=0x500000,DEFINITION=0x530000 };
typedef struct Fixture {
    u8 player[0x31F0],body[256],class_header[0x208],definition[0x80];
    u8 input[0x5C0],input_table[0x4000],globals[0x3000],blends[0x300];
    u8 config[2],client[0x470],blend_pool[0x4C0],scheduler[63*128];
    DreadzoneAnimationMemory views[12];
    DreadzoneAnimationContext memory;
    DreadzonePlayerStateEntryRequest request;
} Fixture;
static u32 checks,failures;
#define CHECK(c) do { ++checks;if(!(c)) { ++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#c); } } while(0)
static void put(u8 *p,u32 o,u32 v) {p[o]=(u8)v;p[o+1]=(u8)(v>>8);p[o+2]=(u8)(v>>16);p[o+3]=(u8)(v>>24);}
static u32 word(const u8 *p,u32 o) {return (u32)p[o]|((u32)p[o+1]<<8)|((u32)p[o+2]<<16)|((u32)p[o+3]<<24);}
static void floating(u8 *p,u32 o,f32 v) {memcpy(p+o,&v,4);}
static void fixture(Fixture *f) {
    u32 i;
    memset(f,0,sizeof(*f));
    put(f->player,0x2EF0,BODY);put(f->player,0x2EFC,0x230280);put(f->player,0x2F00,0x1EE600);
    put(f->player,0x14,0x3B10C0);put(f->player,0x650,0);
    floating(f->player,0x2E20,50.f);floating(f->player,0x2F24,50.f);floating(f->player,0x2E08,.25f);
    /* Explicit authored input fixture, not a claimed full constructor image. */
    for(i=0;i<9;++i) put(f->player,0x2D30+i*4,0xFFFFFFFFU);
    put(f->body,0x24,CLASS);put(f->body,0x40,DEFINITION);floating(f->body,0x48,1.f);
    f->body[0x31]=1;f->body[0x57]=107;f->body[0x4C]=10;
    floating(f->body,0xC0,1.f);floating(f->body,0xD4,1.f);floating(f->body,0xE8,1.f);
    floating(f->player,0x2CF0,1.f);floating(f->player,0x2D04,1.f);floating(f->player,0x2D18,1.f);
    f->class_header[0xC]=112;
    for(i=0;i<112;++i) put(f->class_header,0x48+i*4,DEFINITION);
    f->definition[0x10]=20;f->definition[0x11]=0;f->definition[0x12]=0;
    for(i=0;i<20;++i) put(f->definition,0x20+i*4,10);
    memset(f->input_table,0xA5,sizeof(f->input_table));
    put(f->globals,0xC80,12345);put(f->globals,0x2710,0x1D24C0);
    put(f->blends,0x60,15);put(f->blends,0x64,16);put(f->blends,0x198,8);
    put(f->client,0x460,0xCAFEBABE);
    f->views[0]=(DreadzoneAnimationMemory){f->player,PLAYER,sizeof(f->player)};
    f->views[1]=(DreadzoneAnimationMemory){f->body,BODY,sizeof(f->body)};
    f->views[2]=(DreadzoneAnimationMemory){f->class_header,CLASS,sizeof(f->class_header)};
    f->views[3]=(DreadzoneAnimationMemory){f->definition,DEFINITION,sizeof(f->definition)};
    f->views[4]=(DreadzoneAnimationMemory){f->input,0x1EE600,sizeof(f->input)};
    f->views[5]=(DreadzoneAnimationMemory){f->input_table,0x3B5050,sizeof(f->input_table)};
    f->views[6]=(DreadzoneAnimationMemory){f->globals,0x21D000,sizeof(f->globals)};
    f->views[7]=(DreadzoneAnimationMemory){f->blends,0x220600,sizeof(f->blends)};
    f->views[8]=(DreadzoneAnimationMemory){f->config,0x171DE0,sizeof(f->config)};
    f->views[9]=(DreadzoneAnimationMemory){f->client,0x230280,sizeof(f->client)};
    f->views[10]=(DreadzoneAnimationMemory){f->blend_pool,0x28EB40,sizeof(f->blend_pool)};
    f->views[11]=(DreadzoneAnimationMemory){f->scheduler,0x1D24C0,sizeof(f->scheduler)};
    f->memory=(DreadzoneAnimationContext){NULL,0,0,f->views,12};
    f->request=(DreadzonePlayerStateEntryRequest){&f->memory,PLAYER,0,1,0,1};
}
static int invoke(Fixture *f,s32 state,DreadzonePlayerStateEntryOutput *output) {
    f->request.target_state=state;return dreadzone_player_state_entry(&f->request,output);
}
static void continuity(void) {
    Fixture *f=(Fixture *)malloc(sizeof(Fixture));
    DreadzonePlayerStateEntryOutput out={99};
    u32 i;
    CHECK(f!=NULL);if(!f) return;
    fixture(f);put(f->player,0x2F54,0x435678);put(f->player,0x2EC,0x777700);
    f->player[0x266C]=7;put(f->player,0x2F0,0);
    CHECK(invoke(f,0,&out)==DREADZONE_PLAYER_ENTRY_APPROXIMATE && out.entry_return==1);
    CHECK(f->body[0x54]==0 && word(f->player,0x25D4)==0 && word(f->player,0x264C)==1);
    CHECK(f->player[0x266C]==7);CHECK(word(f->client,0x460)==0);
    CHECK(word(f->player,0x2F54)==0x435678);
    for(i=0;i<sizeof(f->input_table);++i) CHECK(f->input_table[i]==((i<=0x70 && i%16==0)?0:0xA5));
    CHECK(invoke(f,1,&out)==DREADZONE_PLAYER_ENTRY_APPROXIMATE && out.entry_return==1);
    CHECK(f->body[0x54]==0 && word(f->player,0x2DDC)==4);
    CHECK(word(f->player,0x25D8)==0 && word(f->player,0x264C)==2);
    CHECK(invoke(f,2,&out)==DREADZONE_PLAYER_ENTRY_APPROXIMATE && out.entry_return==1);
    CHECK(f->body[0x54]==5 && word(f->body,0x44)==0x41800000);
    CHECK(word(f->player,0x25D0)==0xFFFFFFFFU && word(f->player,0x25D4)==1);
    CHECK(word(f->player,0x25D8)==1 && word(f->player,0x25E4)==0);
    CHECK(word(f->player,0x2F54)==0x435678);
    CHECK(invoke(f,0,&out)==DREADZONE_PLAYER_ENTRY_APPROXIMATE);
    floating(f->player,0x2E08,0.f);put(f->player,0x2F0,0x446688);f->player[0x266C]=7;
    CHECK(invoke(f,1,&out)==DREADZONE_PLAYER_ENTRY_APPROXIMATE);
    CHECK(f->player[0x266C]==0);
    CHECK(invoke(f,2,&out)==DREADZONE_PLAYER_ENTRY_APPROXIMATE);
    CHECK(f->body[0x54]==6 && word(f->body,0x44)==0x41700000);
    CHECK(word(f->player,0x264C)==6 && word(f->player,0x2F54)==0x435678);
    free(f);
}
static void refusal_and_guard(void) {
    Fixture *f=(Fixture *)malloc(sizeof(Fixture));
    Fixture *before=(Fixture *)malloc(sizeof(Fixture));
    DreadzonePlayerStateEntryOutput out;
    int status;
    u32 kind;
    CHECK(f && before);if(!f || !before) {free(f);free(before);return;}
    for(kind=0;kind<7;++kind) {
        fixture(f);f->request.target_state=2;
        if(kind==0) {put(f->player,0x217C,1);put(f->player,0x2178,2);put(f->player,0x2154,0x800000);}
        if(kind==1) {put(f->player,0x264C,9);}
        if(kind==2) {f->player[0x438]=1;f->memory.view_count=12;}
        if(kind==3) {f->player[0x2659]=1;}
        if(kind==4) {f->views[5].bytes=0x40;}
        if(kind==5) {put(f->player,0x2F10,0x10000000);}
        if(kind==6) {f->request.target_state=20;}
        memcpy(before,f,sizeof(*f));out.entry_return=123;
        status=dreadzone_player_state_entry(&f->request,&out);CHECK(status<0);CHECK(out.entry_return==123);
        /* Compare every backing array, not descriptors borrowed by the fixture. */
        CHECK(memcmp(f,before,offsetof(Fixture,views))==0);
    }
    for(kind=3;kind<154;++kind) {
        fixture(f);f->request.target_state=(s32)kind;f->request.force_option=1;
        memcpy(before,f,sizeof(*f));out.entry_return=123;
        CHECK(dreadzone_player_state_entry(&f->request,&out)==DREADZONE_PLAYER_ENTRY_UNSUPPORTED);
        CHECK(out.entry_return==123 && memcmp(f,before,offsetof(Fixture,views))==0);
    }
    fixture(f);put(f->player,0x25D4,20);f->request.target_state=2;out.entry_return=99;
    memcpy(before,f,sizeof(*f));
    CHECK(dreadzone_player_state_entry(&f->request,&out)==DREADZONE_PLAYER_ENTRY_REJECTED_EXACT);
    CHECK(out.entry_return==0);CHECK(memcmp(f,before,offsetof(Fixture,views))==0);
    fixture(f);f->request.selection_option=0;f->player[0x2658]=1;
    CHECK(invoke(f,1,&out)==DREADZONE_PLAYER_ENTRY_APPROXIMATE);
    CHECK(f->body[0x54]==0 && word(f->body,0x44)==0);
    fixture(f);CHECK(dreadzone_player_state_entry(&f->request,(DreadzonePlayerStateEntryOutput *)(void *)(f->player+0x50))<0);
    free(before);free(f);
}
static void aliases_and_boundaries(void) {
    Fixture *f=(Fixture *)malloc(sizeof(Fixture));
    Fixture *before=(Fixture *)malloc(sizeof(Fixture));
    DreadzonePlayerStateEntryOutput out={123};
    const DreadzoneAnimationContext *retained;
    u32 kind;
    CHECK(f && before);if(!f || !before) {free(f);free(before);return;}
    fixture(f);put(f->player,0x25D4,1);put(f->player,0x2EFC,PLAYER+0x25DC-0x460);
    put(f->player,0x2D30,0);put(f->scheduler,0x1C,BODY);f->scheduler[0x10]=7;
    put(f->player,0x37C,0xAABBCCDD);
    CHECK(invoke(f,0,&out)==DREADZONE_PLAYER_ENTRY_APPROXIMATE);
    /* The client store clears previous-base before the release comparison. */
    CHECK(word(f->player,0x25DC)==0 && f->scheduler[0x10]==7);
    CHECK(word(f->player,0x2D30)==0 && word(f->player,0x37C)==0);
    fixture(f);put(f->player,0x25D4,1);put(f->player,0x2EFC,PLAYER+0x2D30-0x460);
    put(f->player,0x2D30,1);put(f->scheduler,0x1C,BODY);f->scheduler[0x10]=7;
    put(f->scheduler,128+0x1C,BODY);f->scheduler[128+0x10]=7;
    CHECK(invoke(f,0,&out)==DREADZONE_PLAYER_ENTRY_APPROXIMATE);
    /* Clearing the handle changes the actually queried record from1 to0. */
    CHECK(f->scheduler[0x10]==0 && f->scheduler[128+0x10]==7);
    CHECK(word(f->player,0x2D30)==0xFFFFFFFFU);
    for(kind=0;kind<7;++kind) {
        fixture(f);memcpy(before,f,sizeof(*f));out.entry_return=123;
        if(kind==0) f->views[0].guest_base=0xFFFFFFF0U;
        if(kind==1) f->views[0].storage=(u8 *)(uintptr_t)(UINTPTR_MAX-1);
        if(kind==2) f->memory.views=(const DreadzoneAnimationMemory *)(uintptr_t)(UINTPTR_MAX-1);
        if(kind==3) f->request.memory=(const DreadzoneAnimationContext *)(uintptr_t)(UINTPTR_MAX-1);
        if(kind==4) f->views[1].guest_base=PLAYER+16;
        if(kind==5) f->views[1].storage=f->player+16;
        if(kind==6) {f->views[0].bytes=0x31EF;}
        CHECK(dreadzone_player_state_entry(&f->request,&out)<0);
        CHECK(out.entry_return==123 && memcmp(f,before,offsetof(Fixture,views))==0);
    }
    fixture(f);retained=f->request.memory;
    CHECK(dreadzone_player_state_entry((const DreadzonePlayerStateEntryRequest *)(uintptr_t)(UINTPTR_MAX-1),&out)<0);
    CHECK(dreadzone_player_state_entry(&f->request,(DreadzonePlayerStateEntryOutput *)(uintptr_t)(UINTPTR_MAX-1))<0);
    CHECK(dreadzone_player_state_entry(&f->request,(DreadzonePlayerStateEntryOutput *)(void *)&f->memory)<0);
    CHECK(dreadzone_player_state_entry(&f->request,(DreadzonePlayerStateEntryOutput *)(void *)f->views)<0);
    CHECK(f->request.memory==retained);
    fixture(f);f->views[0].guest_base=0x21D000;f->request.player_guest=0x21D000;
    /* The fixed globals are now backed inside the relocated player view. */
    memmove(f->views+6,f->views+7,5*sizeof(f->views[0]));f->memory.view_count=11;
    memcpy(before,f,sizeof(*f));out.entry_return=123;
    CHECK(dreadzone_player_state_entry(&f->request,&out)==DREADZONE_PLAYER_ENTRY_UNSUPPORTED);
    CHECK(out.entry_return==123 && memcmp(f,before,offsetof(Fixture,views))==0);
    /* Request is captured before the result publication; this alias is safe. */
    fixture(f);CHECK(dreadzone_player_state_entry(&f->request,(DreadzonePlayerStateEntryOutput *)(void *)&f->request.target_state)==DREADZONE_PLAYER_ENTRY_APPROXIMATE);
    CHECK(f->request.target_state==1);
    free(before);free(f);
}
int main(void) {
    continuity();refusal_and_guard();aliases_and_boundaries();
    printf("%u checks, %u failures\n",checks,failures);return failures?1:0;
}
