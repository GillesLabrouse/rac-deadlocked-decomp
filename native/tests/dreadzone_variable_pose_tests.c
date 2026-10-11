#include "dreadzone_variable_pose.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
static unsigned checks,failures;
#define CHECK(x) do { checks++;if(!(x)) { failures++;printf("line %d: %s\n",__LINE__,#x); } } while(0)
static void make_stream(u8 *scratch) {
    u8 *h=scratch+0x2000;
    unsigned i;
    memset(scratch,0xA5,0x4000);memset(h,0,0x80);
    h[0]=2;h[1]=7;h[2]=1;h[3]=4;h[4]=2;h[7]=2;h[8]=1;
    for(i=0;i<4;i++) {
        scratch[0x2016+i*16]=0xFF;scratch[0x2017+i*16]=0x7F;
        scratch[0x201E+i*16]=0xFF;scratch[0x201F+i*16]=0x7F;
    }
    for(i=0;i<16;i++) scratch[0x2050+i]=(u8)(128+(i%8)*4);
}
static float load_float(const u8 *p) { float v;memcpy(&v,p,4);return v; }
int main(void) {
    u8 scratch[0x4020],saved[0x4020],vu[0x1000];
    DreadzoneVariablePose request={scratch,0x4000,0.f,vu,sizeof(vu)};
    unsigned i,opcode;
    memset(vu,0x53,sizeof(vu));make_stream(scratch);memset(scratch+0x4000,0x91,32);
    memcpy(saved,scratch,sizeof(scratch));
    CHECK(dreadzone_variable_pose_decode(&request)==DREADZONE_VARIABLE_POSE_APPROXIMATE);
    CHECK(load_float(scratch)==128.f);CHECK(load_float(scratch+4)==132.f);CHECK(load_float(scratch+8)==136.f);
    CHECK(load_float(scratch+12)==(float)0x7FFF008CU);
    CHECK(load_float(scratch+64)==144.f);CHECK(load_float(scratch+68)==148.f);CHECK(load_float(scratch+72)==152.f);
    for(i=0;i<sizeof(scratch);i++) if(i>=80 || (i>=16 && i<64)) CHECK(scratch[i]==saved[i]);
    for(i=0;i<sizeof(vu);i++) CHECK(vu[i]==0x53);
    make_stream(scratch);request.fraction=.5f;
    CHECK(dreadzone_variable_pose_decode(&request)==DREADZONE_VARIABLE_POSE_APPROXIMATE);
    CHECK(load_float(scratch)==64.f);CHECK(load_float(scratch+4)==66.f);
    make_stream(scratch);scratch[0x2060]=12;memcpy(saved,scratch,sizeof(scratch));
    CHECK(dreadzone_variable_pose_decode(&request)==DREADZONE_VARIABLE_POSE_UNSUPPORTED);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    make_stream(scratch);scratch[0x2001]=1;memcpy(saved,scratch,sizeof(scratch));
    CHECK(dreadzone_variable_pose_decode(&request)==DREADZONE_VARIABLE_POSE_UNSUPPORTED);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    request.fraction=NAN;
    CHECK(dreadzone_variable_pose_decode(&request)==DREADZONE_VARIABLE_POSE_UNSUPPORTED);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    request.fraction=.5f;request.vu_data=NULL;
    CHECK(dreadzone_variable_pose_decode(&request)==DREADZONE_VARIABLE_POSE_INVALID);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    request.vu_data=vu;request.bytes=0x3FFF;
    CHECK(dreadzone_variable_pose_decode(&request)==DREADZONE_VARIABLE_POSE_INVALID);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    request.bytes=0x4000;request.fraction=0;make_stream(scratch);
    scratch[0x2050]=255;memcpy(saved,scratch,sizeof(scratch));
    CHECK(dreadzone_variable_pose_decode(&request)==DREADZONE_VARIABLE_POSE_UNSUPPORTED);
    CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    /* An inherited low VU quad is a real input, including visible lanes. */
    make_stream(scratch);scratch[0x2050]=4;
    { float inherited=43.5f;memcpy(vu+4*16,&inherited,4); }
    CHECK(dreadzone_variable_pose_decode(&request)==DREADZONE_VARIABLE_POSE_APPROXIMATE);
    CHECK(load_float(scratch)==43.5f);
    /* Capture the descriptor before its own storage becomes a quaternion. */
    request.bytes=0x4000;request.fraction=0;make_stream(scratch);
    memcpy(scratch,&request,sizeof(request));
    CHECK(dreadzone_variable_pose_decode((const DreadzoneVariablePose *)(const void *)scratch)==DREADZONE_VARIABLE_POSE_APPROXIMATE);
    CHECK(load_float(scratch)==128.f);
    /* Fixed-only streams still execute the compressed decoder. */
    make_stream(scratch);scratch[0x2002]=0;request.fraction=0;
    memcpy(saved,scratch,sizeof(scratch));
    CHECK(dreadzone_variable_pose_decode(&request)==DREADZONE_VARIABLE_POSE_APPROXIMATE);
    CHECK(load_float(scratch)==128.f);CHECK(load_float(scratch+64)==144.f);
    CHECK(memcmp(scratch+0x2000,saved+0x2000,sizeof(scratch)-0x2000)==0);
    for(opcode=24;opcode<=48;opcode+=24) {
        make_stream(scratch);scratch[0x2002]=0;scratch[0x2060]=(u8)opcode;
        memcpy(saved,scratch,sizeof(scratch));
        CHECK(dreadzone_variable_pose_decode(&request)==DREADZONE_VARIABLE_POSE_UNSUPPORTED);
        CHECK(memcmp(saved,scratch,sizeof(scratch))==0);
    }
    printf("%u checks, %u failures\n",checks,failures);return failures!=0;
}
