#include "dreadzone_bind_pose.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks, failures;
#define CHECK(x) do { checks++; if (!(x)) { failures++; \
    printf("FAIL line%d: %s\n", __LINE__, #x); } } while(0)
static u8 poses[127*64],bindings[127*48],scratch[0x4002],original[0x4002];
static DreadzoneModelView views[2];
static DreadzoneBindPose request;
static void put_float(u8 *p,float v) { memcpy(p,&v,4); }
static float get_float(const u8 *p) { float v;memcpy(&v,p,4);return v; }
static void setup(s32 count) {
    u32 i,j;
    for(i=0;i<sizeof(scratch);i++) scratch[i]=(u8)(i*73+61);
    for(i=0;i<127;i++) {
        for(j=0;j<16;j++) put_float(poses+i*64+j*4,j%5==0 ? 1.0f : 0.0f);
        put_float(poses+i*64+48,(float)i+3);put_float(poses+i*64+52,-(float)i-5);
        put_float(poses+i*64+56,(float)i+7);
        for(j=0;j<12;j++) put_float(bindings+i*48+j*4,0);
        put_float(bindings+i*48,2);put_float(bindings+i*48+20,3);put_float(bindings+i*48+40,4);
        put_float(bindings+i*48+12,-(float)i);put_float(bindings+i*48+28,(float)i);
        put_float(bindings+i*48+44,11);
    }
    views[0].guest_base=0x400000;views[0].bytes=sizeof(poses);views[0].storage=poses;
    views[1].guest_base=0x500000;views[1].bytes=sizeof(bindings);views[1].storage=bindings;
    request.views=views;request.view_count=2;request.pose_guest=0x400000;request.bind_guest=0x500000;
    request.count=count;request.scratch=scratch+1;request.scratch_bytes=0x4000;
}
static void refused(int expected) {
    memcpy(original,scratch,sizeof(scratch));
    CHECK(dreadzone_bind_pose_prepare(&request)==expected);
    CHECK(memcmp(original,scratch,sizeof(scratch))==0);
}
int main(void) {
    const s32 counts[]={1,2,3,107,127};u32 k,i,j;
    setup(0);request.views=NULL;request.view_count=0;request.pose_guest=1;request.bind_guest=1;
    memcpy(original,scratch,sizeof(scratch));
    CHECK(dreadzone_bind_pose_prepare(&request)==1);
    for(i=0;i<16;i++) CHECK(get_float(scratch+1+i*4)==(i%5==0 ? 1.0f : 0.0f));
    for(i=65;i<sizeof(scratch);i++) CHECK(scratch[i]==original[i]);
    CHECK(scratch[0]==original[0]);
    for(k=0;k<sizeof(counts)/sizeof(counts[0]);k++) {
        u32 count=(u32)counts[k];
        setup((s32)count);memcpy(original,scratch,sizeof(scratch));
        CHECK(dreadzone_bind_pose_prepare(&request)==1);
        for(i=0;i<count;i++) {
            for(j=0;j<16;j++) {
                float expected=0;
                if(j==0) expected=2;if(j==5) expected=3;if(j==10) expected=4;
                if(j==12) expected=3;if(j==13) expected=-5;if(j==14) expected=(float)i+18;
                if(j==15) expected=1;
                CHECK(get_float(scratch+1+i*64+j*4)==expected);
            }
        }
        CHECK(memcmp(scratch+1+count*64,bindings,count*48)==0);
        for(i=1+count*112;i<sizeof(scratch);i++) CHECK(scratch[i]==original[i]);
        CHECK(scratch[0]==original[0]);
        setup((s32)count);memcpy(scratch+1,poses,count*64);request.pose_guest=0x70000000;
        CHECK(dreadzone_bind_pose_prepare(&request)==1);
        CHECK(get_float(scratch+1+(count-1)*64+56)==(float)count+17);
    }
    setup(1);
    /* A nonsymmetric pose/bind checks multiplication direction and layout. */
    for(i=0;i<16;i++) put_float(poses+i*4,(float)i-4);
    for(i=0;i<12;i++) put_float(bindings+i*4,((float)i-3)*0.5f);
    CHECK(dreadzone_bind_pose_prepare(&request)==1);
    for(i=0;i<16;i++) {
        u32 column=i/4,row=i%4;
        double expected=0;
        for(j=0;j<3;j++) expected+=(double)get_float(poses+j*16+row*4)*get_float(bindings+j*16+column*4);
        if(column==3) expected+=get_float(poses+48+row*4);
        CHECK(get_float(scratch+1+i*4)==(float)expected);
    }
    setup(-1);refused(-3);setup(128);refused(-3);
    setup(3);request.pose_guest=0x400001;refused(-1);
    setup(3);request.bind_guest=0x500001;refused(-1);
    setup(3);views[0].bytes=3*64-1;refused(-2);
    setup(3);views[1].bytes=3*48-1;refused(-2);
    setup(3);request.scratch_bytes=0x3FFF;refused(-1);
    setup(3);views[1]=views[0];refused(-1);
    setup(3);views[0].guest_base=0xFFFFFFF0;refused(-1);
    setup(3);views[0].storage=scratch+1;refused(-1);
    setup(3);request.views=(const DreadzoneModelView *)(UINTPTR_MAX-8);refused(-1);
    setup(3);request.scratch=(u8 *)(UINTPTR_MAX-8);
    CHECK(dreadzone_bind_pose_prepare(&request)==-1);
    setup(3);put_float(bindings+2*48,INFINITY);refused(-3);
    setup(3);put_float(poses+2*64,NAN);refused(-3);
    setup(3);{u32 bits=1;memcpy(bindings,&bits,4);}refused(-3);
    setup(3);put_float(bindings,3.0e38f);put_float(poses,3.0e38f);refused(-3);
    setup(3);{
        DreadzoneBindPose saved=request;
        DreadzoneBindPose *inside=(DreadzoneBindPose *)(void *)(scratch+16);
        memcpy(inside,&saved,sizeof(saved));
        CHECK(dreadzone_bind_pose_prepare(inside)==1);
        CHECK(get_float(scratch+1+128+56)==20);
    }
    setup(3);{
        DreadzoneModelView *inside=(DreadzoneModelView *)(void *)(scratch+32);
        memcpy(inside,views,sizeof(views));request.views=inside;
        CHECK(dreadzone_bind_pose_prepare(&request)==1);
        CHECK(get_float(scratch+1+128+56)==20);
    }
    setup(3);memcpy(original,scratch,sizeof(scratch));
    CHECK(dreadzone_bind_pose_prepare(NULL)==-1);
    CHECK(memcmp(original,scratch,sizeof(scratch))==0);
    CHECK(dreadzone_bind_pose_prepare((const DreadzoneBindPose *)(UINTPTR_MAX-8))==-1);
    CHECK(memcmp(original,scratch,sizeof(scratch))==0);
    printf("%u checks, %u failures\n",checks,failures);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
