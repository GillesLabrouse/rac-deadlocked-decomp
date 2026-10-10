#include "dreadzone_actor_parameters.h"
#include <stdio.h>
#include <string.h>
static unsigned checks,failures;
#define CHECK(x) do { ++checks; if(!(x)) { ++failures; if(failures<8) printf("line%d: %s\n",__LINE__,#x); } } while(0)
static u8 backing[0x31F0+64],expected[sizeof(backing)];
static void word(u8 *p,u32 v) { p[0]=(u8)v; p[1]=(u8)(v>>8); p[2]=(u8)(v>>16); p[3]=(u8)(v>>24); }
static void half(u8 *p,u16 v) { p[0]=(u8)v; p[1]=(u8)(v>>8); }
static void oracle(s32 mode,u32 start) {
    /* Recovered numeric values are checked as IEEE bit patterns, independent
     * of the implementation's decimal literals and scalar stores. */
    static const u16 ids[18]={9,10,11,4,15,16,7,8,13,12,6,0,26,1,19,27,30,17};
    static const u32 first[18]={0,0,0,0,0x3DCCCCCD,0x3DCCCCCD,0x3DCCCCCD,0x3DCCCCCD,
        0,0,0,0,0x3DF5C28F,0x3DCCCCCD,0x3DCCCCCD,0x3C54FDF4,0x3C8B4396,0x3CF5C28F};
    static const u32 second[18]={0,0,0,0,0x3E99999A,0x3E99999A,0x3E99999A,0x3E99999A,
        0,0,0,0,0x3E99999A,0x3E99999A,0x3E99999A,0x3E800000,0x3E570A3D,0x3E4CCCCD};
    unsigned i;
    memcpy(expected,backing,sizeof(backing));
    for(i=0;i<18;i++) {
        u8 *p=expected+start+0x8C0+i*0xB0;
        half(p,(u16)(i==15 && mode==1?20:ids[i]));
        half(p+2,(u16)(mode==1 && (i==12 || i==13 || i==14 || i==16 || i==17)?0xFFFF:0));
        word(p+4,first[i]); word(p+8,second[i]); word(p+12,0x3F800000);
    }
}
int main(void) {
    static const s32 modes[]={0,1,2,-1,0x7FFFFFFF,(s32)0x80000000U,6,99};
    DreadzoneActorParameterView view;
    unsigned seed,i;
    for(seed=0;seed<256;seed++) {
        u32 start=16+seed%16;
        for(i=0;i<sizeof(backing);i++) backing[i]=(u8)(i*13+seed*47+(i>>8));
        view.record=backing+start; view.record_guest=0x368F60+(seed%4)*4;
        view.record_bytes=0x31F0; view.mode_0021E824=modes[seed%8];
        oracle(view.mode_0021E824,start);
        CHECK(dreadzone_actor_00669A20(&view)==0);
        for(i=0;i<sizeof(backing);i++) CHECK(backing[i]==expected[i]);
    }
    memcpy(expected,backing,sizeof(backing));
    view.record_bytes=0x31EF; CHECK(dreadzone_actor_00669A20(&view)==-1);
    view.record_bytes=0x31F0; view.record_guest=0x368F61; CHECK(dreadzone_actor_00669A20(&view)==-1);
    view.record_guest=0xFFFFD000; CHECK(dreadzone_actor_00669A20(&view)==-1);
    view.record_guest=0x368F60; view.record=0; CHECK(dreadzone_actor_00669A20(&view)==-1);
    CHECK(dreadzone_actor_00669A20(0)==-1);
    for(i=0;i<sizeof(backing);i++) CHECK(backing[i]==expected[i]);
    printf("Actor parameters: %u checks, %u failures. All288 written bytes and untouched record/guards verified.\n",checks,failures);
    return failures?1:0;
}
