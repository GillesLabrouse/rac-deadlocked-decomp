#include "dreadzone_general_matrices.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
static unsigned checks,failures;
#define CHECK(x) do { checks++;if(!(x)) { failures++;printf("line %d: %s\n",__LINE__,#x); } } while(0)
static void put(u8 *p,float v) { memcpy(p,&v,4); }
static float get(const u8 *p) { float v;memcpy(&v,p,4);return v; }
static void make_records(u8 *scratch,u8 inherit) {
    unsigned i;
    memset(scratch,0xA5,0x4000);memset(scratch,0,128);
    for(i=0;i<2;i++) {
        put(scratch+i*64+12,1.f);
        put(scratch+i*64+16,268435456.f*(i==0 ? 2.f : 1.f));
        put(scratch+i*64+20,268435456.f*(i==0 ? 3.f : 1.f));
        put(scratch+i*64+24,268435456.f*(i==0 ? 4.f : 1.f));
    }
    put(scratch+32,65536.f);put(scratch+64+32,65536.f);
    scratch[0x3C]=(u8)(127|inherit);scratch[64+0x3C]=0;
}
static void initialization_tests(void) {
    u8 skeleton[127*16],scratch[0x4020],saved[0x4020];
    DreadzoneLocalPoseInit request={skeleton,sizeof(skeleton),scratch,0x4000,127};
    unsigned joint,lane;
    memset(scratch,0x96,sizeof(scratch));
    for(joint=0;joint<127;joint++) {
        for(lane=0;lane<3;lane++) put(skeleton+joint*16+lane*4,(float)((int)joint-(int)lane));
        skeleton[joint*16+12]=(u8)(joint|128);skeleton[joint*16+13]=0xFF;
        skeleton[joint*16+14]=0xCF;skeleton[joint*16+15]=0x7F;
    }
    memcpy(saved,scratch,sizeof(scratch));
    CHECK(dreadzone_local_pose_initialize(&request)==DREADZONE_GENERAL_MATRICES_APPROXIMATE);
    for(joint=0;joint<127;joint++) {
        for(lane=0;lane<4;lane++) {
            CHECK(get(scratch+joint*64+lane*4)==(lane==3 ? 1.f : 0.f));
            CHECK(get(scratch+joint*64+16+lane*4)==(lane==3 ? 0.f : 268435456.f));
        }
        for(lane=0;lane<3;lane++) CHECK(get(scratch+joint*64+32+lane*4)==(float)((int)joint-(int)lane)*65536.f);
        CHECK(memcmp(scratch+joint*64+44,skeleton+joint*16+12,4)==0);
        CHECK(memcmp(scratch+joint*64+32,scratch+joint*64+48,16)==0);
    }
    for(joint=127*64;joint<sizeof(scratch);joint++) CHECK(scratch[joint]==saved[joint]);
    /* Genuine source at2000 overlaps the same scratch allocation. */
    memset(scratch,0x96,sizeof(scratch));memcpy(scratch+0x2000,skeleton,sizeof(skeleton));
    request.skeleton=scratch+0x2000;memcpy(saved,scratch,sizeof(scratch));
    CHECK(dreadzone_local_pose_initialize(&request)==DREADZONE_GENERAL_MATRICES_APPROXIMATE);
    CHECK(memcmp(scratch+0x2000,skeleton,sizeof(skeleton))==0);
    CHECK(memcmp(scratch+127*64,saved+127*64,sizeof(scratch)-127*64)==0);
    /* Source overlapping the output is explicitly snapshotted too. */
    memset(scratch,0x96,sizeof(scratch));memcpy(scratch,skeleton,sizeof(skeleton));request.skeleton=scratch;
    CHECK(dreadzone_local_pose_initialize(&request)==DREADZONE_GENERAL_MATRICES_APPROXIMATE);
    CHECK(get(scratch+126*64+32)==126.f*65536.f);
    request.skeleton=skeleton;put(skeleton+126*16,INFINITY);memcpy(saved,scratch,sizeof(scratch));
    CHECK(dreadzone_local_pose_initialize(&request)==DREADZONE_GENERAL_MATRICES_UNSUPPORTED);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    request.joints=0;
    CHECK(dreadzone_local_pose_initialize(&request)==DREADZONE_GENERAL_MATRICES_INVALID);
    request.joints=128;
    CHECK(dreadzone_local_pose_initialize(&request)==DREADZONE_GENERAL_MATRICES_INVALID);
    request.joints=127;request.skeleton_bytes--;
    CHECK(dreadzone_local_pose_initialize(&request)==DREADZONE_GENERAL_MATRICES_INVALID);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
}
int main(void) {
    u8 scratch[0x4020],saved[0x4020];
    DreadzoneGeneralMatrices request={scratch,0x4000,2};
    unsigned i;
    initialization_tests();
    memset(scratch,0x93,sizeof(scratch));memcpy(saved,scratch,sizeof(scratch));request.joints=0;
    CHECK(dreadzone_general_matrices_prepare(&request)==DREADZONE_GENERAL_MATRICES_APPROXIMATE);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);request.joints=2;
    CHECK(dreadzone_general_matrices_prepare(NULL)==DREADZONE_GENERAL_MATRICES_INVALID);
    CHECK(dreadzone_local_pose_initialize(NULL)==DREADZONE_GENERAL_MATRICES_INVALID);
    make_records(scratch,0);memset(scratch+0x4000,0x91,32);memcpy(saved,scratch,sizeof(scratch));
    CHECK(dreadzone_general_matrices_prepare(&request)==DREADZONE_GENERAL_MATRICES_APPROXIMATE);
    CHECK(get(scratch)==2.f);CHECK(get(scratch+20)==3.f);CHECK(get(scratch+40)==4.f);
    CHECK(get(scratch+48)==1.f);CHECK(get(scratch+64+48)==2.f);
    CHECK(get(scratch+0x2000)==1.f);CHECK(get(scratch+0x3FC0)==1.f);
    for(i=128;i<sizeof(scratch);i++) if(i<0x2000 || (i>=0x2080 && i<0x3FC0) || i>=0x4000) CHECK(scratch[i]==saved[i]);
    make_records(scratch,128);
    CHECK(dreadzone_general_matrices_prepare(&request)==DREADZONE_GENERAL_MATRICES_APPROXIMATE);
    CHECK(get(scratch+0x2000)==2.f);CHECK(get(scratch+64+48)==3.f);CHECK(get(scratch+64)==2.f);
    /* Metadata in ignored scale/translation lanesw cannot affect the result. */
    make_records(scratch,0);put(scratch+28,NAN);put(scratch+44,NAN);put(scratch+92,NAN);put(scratch+108,NAN);
    CHECK(dreadzone_general_matrices_prepare(&request)==DREADZONE_GENERAL_MATRICES_APPROXIMATE);
    CHECK(get(scratch+64+48)==2.f);
    make_records(scratch,0);scratch[64+0x3C]=1;memcpy(saved,scratch,sizeof(scratch));
    CHECK(dreadzone_general_matrices_prepare(&request)==DREADZONE_GENERAL_MATRICES_UNSUPPORTED);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    make_records(scratch,0);put(scratch+12,0.f);memcpy(saved,scratch,sizeof(scratch));
    CHECK(dreadzone_general_matrices_prepare(&request)==DREADZONE_GENERAL_MATRICES_UNSUPPORTED);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    make_records(scratch,0);put(scratch,INFINITY);memcpy(saved,scratch,sizeof(scratch));
    CHECK(dreadzone_general_matrices_prepare(&request)==DREADZONE_GENERAL_MATRICES_UNSUPPORTED);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    request.bytes=0x3FFF;
    CHECK(dreadzone_general_matrices_prepare(&request)==DREADZONE_GENERAL_MATRICES_INVALID);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    request.bytes=0x4000;request.joints=128;
    CHECK(dreadzone_general_matrices_prepare(&request)==DREADZONE_GENERAL_MATRICES_UNSUPPORTED);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    /* Capture the descriptor before its quaternion output overwrites it. */
    make_records(scratch,0);request.joints=2;memcpy(scratch+0x2000,&request,sizeof(request));
    CHECK(dreadzone_general_matrices_prepare((const DreadzoneGeneralMatrices *)(const void *)(scratch+0x2000))==DREADZONE_GENERAL_MATRICES_APPROXIMATE);
    CHECK(get(scratch)==2.f);
    printf("%u checks, %u failures\n",checks,failures);return failures!=0;
}
