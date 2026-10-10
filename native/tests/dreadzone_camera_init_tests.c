#include "dreadzone_camera_init.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static unsigned checks,failures;
#define CHECK(x) do { checks++;if(!(x)){failures++;printf("FAIL line%d: %s\n",__LINE__,#x);} } while(0)
static u8 arena[0x502],before[0x502],clock_data[4];
static DreadzoneCameraView views[2];
static DreadzoneCameraInit request;
static u32 bits(float value) { u32 b;memcpy(&b,&value,4);return b; }
static u32 read32(const u8 *p) { return p[0]|(u32)p[1]<<8|(u32)p[2]<<16|(u32)p[3]<<24; }
static void setup(void) {
    u32 i;
    for(i=0;i<sizeof(arena);i++) arena[i]=(u8)(i*37+91);
    clock_data[0]=0x12;clock_data[1]=0x34;clock_data[2]=0x56;clock_data[3]=0x78;
    views[0].guest_base=0x400000;views[0].bytes=0x500;views[0].storage=arena+1;
    views[1].guest_base=0x21E754;views[1].bytes=4;views[1].storage=clock_data;
    request.views=views;request.view_count=2;request.output=0x400000;
    request.position=0x400300;request.actor=0xFFFFFFFF;
    request.angle12=0;request.angle13=0;
}
static void refusal(int result) {
    memcpy(before,arena,sizeof(arena));
    CHECK(dreadzone_camera_init(&request)==result);
    CHECK(memcmp(before,arena,sizeof(arena))==0);
}
int main(void) {
    float pair[2]={13,17};u32 i;
    const u32 scalar_offsets[]={0x48,0x4C,0x50,0x54,0x70,0x74,0x78,0x7C,
        0x120,0x5C,0x68,0x6C,0xB4,0xB8};
    const u32 scalar_values[]={0x3DD67751,0x3D567751,0x3B64C389,0x3AFE2E98,
        0x3F19999A,0x3F2A7EFA,0x3F59999A,0x3EB2B8C2,0x42340000,0x3F000000,
        0x3D0EFA36,0x3ABEA2F2,0x3FBDE44E,0xBFBDE44E};
    CHECK(dreadzone_angle_pair(0,pair)==DREADZONE_ANGLE_APPROXIMATE);
    CHECK(bits(pair[0])==0 && bits(pair[1])==0x3F800000);
    CHECK(dreadzone_angle_pair(0.25f,pair)==DREADZONE_ANGLE_APPROXIMATE);
    CHECK(bits(pair[0])==0x3E7D5776 && bits(pair[1])==0x3F780AA5);
    CHECK(dreadzone_angle_pair(-0.5f,pair)==DREADZONE_ANGLE_APPROXIMATE);
    CHECK(bits(pair[0])==0xBEF57744 && bits(pair[1])==0x3F60A940);
    CHECK(dreadzone_angle_pair(0,pair)==DREADZONE_ANGLE_APPROXIMATE);
    CHECK(dreadzone_angle_pair(4,pair)==DREADZONE_ANGLE_UNSUPPORTED);
    CHECK(pair[0]==0 && fabsf(pair[1]-1.0f)<0.000001f);
    CHECK(dreadzone_angle_pair(NAN,pair)==DREADZONE_ANGLE_UNSUPPORTED);
    CHECK(dreadzone_angle_pair(INFINITY,pair)==DREADZONE_ANGLE_UNSUPPORTED);
    CHECK(dreadzone_angle_pair(FLT_MIN/2,pair)==DREADZONE_ANGLE_UNSUPPORTED);
    CHECK(dreadzone_angle_pair(1.0e-8f,pair)==DREADZONE_ANGLE_APPROXIMATE);
    CHECK(bits(pair[0])==bits(1.0e-8f) && bits(pair[1])==0x3F800000);
    CHECK(dreadzone_angle_pair(0,NULL)==DREADZONE_ANGLE_INVALID);
    CHECK(dreadzone_angle_pair(0,(float *)(uintptr_t)(UINTPTR_MAX-3))==DREADZONE_ANGLE_INVALID);
    setup();memcpy(before,arena,sizeof(arena));
    CHECK(dreadzone_camera_init(&request)==DREADZONE_CAMERA_APPROXIMATE);
    for(i=0;i<14;i++) CHECK(read32(arena+1+scalar_offsets[i])==scalar_values[i]);
    CHECK(read32(arena+1+0xBC)==0xFFFFFFFF);
    CHECK(memcmp(arena+1+0x30,before+1+0x300,16)==0);
    CHECK(memcmp(arena+1+0x140,before+1+0x300,16)==0);
    CHECK(memcmp(arena+1+0x134,clock_data,4)==0);
    CHECK(memcmp(arena+1,arena+1+0xF0,16)==0);
    CHECK(arena[0]==before[0] && arena[0x501]==before[0x501]);
    CHECK(arena[1+0x58]==before[1+0x58]);
    CHECK(arena[1+0x90]==before[1+0x90]);
    CHECK(arena[1+0x124]==before[1+0x124]);
    for(i=0;i<0x150;i+=16) {
        setup();memcpy(before,arena,sizeof(arena));request.position=request.output+i;
        CHECK(dreadzone_camera_init(&request)==DREADZONE_CAMERA_APPROXIMATE);
        CHECK(memcmp(arena+1+0x30,arena+1+0x140,16)==0);
        if(i!=0x130) CHECK(memcmp(arena+1+0x30,arena+1+i,16)==0);
        else CHECK(memcmp(arena+1+0x34,before+1+0x134,4)==0);
    }
    /* Actual late global reload observes output54's constant at21E754. */
    setup();views[0].guest_base=0x21E700;request.output=0x21E700;
    request.position=0x21EA00;request.view_count=1;
    CHECK(dreadzone_camera_init(&request)==DREADZONE_CAMERA_APPROXIMATE);
    CHECK(read32(arena+1+0x134)==0x3AFE2E98);
    setup();request.angle12=NAN;refusal(DREADZONE_CAMERA_UNSUPPORTED);
    setup();request.angle13=4;refusal(DREADZONE_CAMERA_UNSUPPORTED);
    setup();request.angle13=FLT_MIN/2;refusal(DREADZONE_CAMERA_UNSUPPORTED);
    setup();request.angle12=1.0e-8f;CHECK(dreadzone_camera_init(&request)==DREADZONE_CAMERA_APPROXIMATE);
    setup();request.output++;refusal(DREADZONE_CAMERA_INVALID);
    setup();request.position++;refusal(DREADZONE_CAMERA_INVALID);
    setup();request.view_count=0;refusal(DREADZONE_CAMERA_INVALID);
    setup();request.view_count=9;refusal(DREADZONE_CAMERA_INVALID);
    setup();views[0].bytes=0x14F;refusal(DREADZONE_CAMERA_UNMAPPED);
    setup();views[1].guest_base++;refusal(DREADZONE_CAMERA_UNMAPPED);
    setup();views[1].guest_base=0x400100;refusal(DREADZONE_CAMERA_INVALID);
    setup();views[1].storage=arena+17;refusal(DREADZONE_CAMERA_INVALID);
    setup();views[0].guest_base=0xFFFFFFF0;refusal(DREADZONE_CAMERA_INVALID);
    setup();views[0].storage=(u8 *)(uintptr_t)(UINTPTR_MAX-15);refusal(DREADZONE_CAMERA_INVALID);
    CHECK(dreadzone_camera_init(NULL)==DREADZONE_CAMERA_INVALID);
    CHECK(dreadzone_camera_init((DreadzoneCameraInit *)(uintptr_t)(UINTPTR_MAX-3))==DREADZONE_CAMERA_INVALID);
    /* An output-backed request/table is rejected before publishing a write. */
    setup();memcpy(arena+1,&request,sizeof(request));memcpy(before,arena,sizeof(arena));
    CHECK(dreadzone_camera_init((DreadzoneCameraInit *)(void *)(arena+1))==DREADZONE_CAMERA_INVALID);
    CHECK(memcmp(before,arena,sizeof(arena))==0);
    setup();memcpy(arena+1,views,sizeof(views));request.views=(DreadzoneCameraView *)(void *)(arena+1);
    refusal(DREADZONE_CAMERA_INVALID);
    CHECK(bits(0.0f)==0);
    printf("%u checks, %u failures\n",checks,failures);
    return failures ? 1 : 0;
}
