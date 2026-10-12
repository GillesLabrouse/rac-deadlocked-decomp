#include "dreadzone_model_transform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks,failures;
#define CHECK(x) do { checks++;if(!(x)){ failures++;printf("line %d: %s\n",__LINE__,#x);}}while(0)
static u8 ram[0x6000],prior[0x6000],spr[0x4000],spr_prior[0x4000],vu[0x1000];
static DreadzoneModelView view;
static DreadzoneFixedModelTransform request;
static void put32(u8 *p,u32 v) {p[0]=(u8)v;p[1]=(u8)(v>>8);p[2]=(u8)(v>>16);p[3]=(u8)(v>>24);}
static void put_float(u8 *p,float v) {memcpy(p,&v,4);}
static float load_float(const u8 *p) {float v;memcpy(&v,p,4);return v;}
static void setup(u32 count) {
    u32 i;
    memset(ram,0,sizeof(ram));memset(spr,0,sizeof(spr));memset(vu,0x53,sizeof(vu));
    put32(ram+0x24,0x400100);put32(ram+0x40,0x400140);put32(ram+0x58,0x403000);
    ram[0x55]=0xF3;ram[0x57]=(u8)count;ram[0x108]=(u8)count;
    put32(ram+0x118,0x400400);ram[0x150]=17;put32(ram+0x15C,0x400200);
    /* Generated fixed-only stream; no disc-derived bytes. */
    ram[0x200]=2;ram[0x201]=7;ram[0x203]=4;ram[0x204]=2;ram[0x207]=2;ram[0x208]=1;
    for(i=0;i<4;i++) {
        ram[0x216+i*16]=0xFF;ram[0x217+i*16]=0x7F;
        ram[0x21E+i*16]=0xFF;ram[0x21F+i*16]=0x7F;
    }
    for(i=0;i<16;i++)ram[0x250+i]=(u8)(128+(i%8)*4);
    for(i=0;i<count;i++) {
        u8 *s=ram+0x400+i*16;
        put_float(s,1.25f);put_float(s+4,-2.5f);put_float(s+8,.125f);
        put32(s+12,i?i-1:127);
    }
    view.guest_base=0x400000;view.bytes=sizeof(ram);view.storage=ram;
    request.model.views=&view;request.model.view_count=1;request.model.actor=0x400000;
    request.model.scratch=spr;request.model.scratch_bytes=sizeof(spr);
    request.vu_data=vu;request.vu_data_bytes=sizeof(vu);
}
static void variable(u32 quads,u32 opcode,u32 frames) {
    u32 i;
    setup(3);ram[0x202]=(u8)quads;ram[0x260]=(u8)opcode;ram[0x150]=(u8)frames;
    for(i=0;i<80;i++) {ram[0x270+i]=(u8)(i*3+7);ram[0x270+quads*16+i]=(u8)(i*5+19);}
}
static void refuse(int status) {
    memcpy(prior,ram,sizeof(ram));memcpy(spr_prior,spr,sizeof(spr));
    CHECK(dreadzone_model_transform_initial(&request)==status);
    CHECK(memcmp(prior,ram,sizeof(ram))==0);CHECK(memcmp(spr_prior,spr,sizeof(spr))==0);
}
static void refuse_frame(u32 frame_word) {
    put32(ram+0x44,frame_word);
    memcpy(prior,ram,sizeof(ram));memcpy(spr_prior,spr,sizeof(spr));
    CHECK(dreadzone_model_transform_frame(&request)==DREADZONE_TRANSFORM_UNSUPPORTED);
    CHECK(memcmp(prior,ram,sizeof(ram))==0);CHECK(memcmp(spr_prior,spr,sizeof(spr))==0);
}
int main(void) {
    u32 frames,i;u8 current[0x800],next[0x800];
    for(frames=1;frames<=2;frames++) {
        variable(5,0,frames);memcpy(prior,ram,sizeof(ram));
        memcpy(current,ram+0x270,80);memcpy(next,ram+0x2C0,80);
        CHECK(dreadzone_model_transform_initial(&request)==1);
        CHECK(ram[0x55]==0xD3 && memcmp(ram+0x3000,spr,3*64)==0);
        CHECK(memcmp(spr+0x3000,current,80)==0);
        CHECK(memcmp(spr+0x3800,frames==1?current:next,80)==0);
        for(i=0;i<sizeof(ram);i++)
            if(i!=0x55&&(i<0x3000||i>=0x3000+3*64))CHECK(ram[i]==prior[i]);
        for(i=0;i<sizeof(vu);i++)CHECK(vu[i]==0x53);
    }
    variable(128,0,2);memcpy(current,ram+0x270,sizeof(current));memcpy(next,ram+0xA70,sizeof(next));
    /* General matrices really replaces the final scratch quad block with the
     * parent127 identity matrix, after the complete next-bank transfer. */
    for(i=0;i<16;i++)put_float(next+sizeof(next)-64+i*4,i%5==0?1.f:0.f);
    CHECK(dreadzone_model_transform_initial(&request)==1);
    CHECK(memcmp(spr+0x3000,current,sizeof(current))==0);
    CHECK(memcmp(spr+0x3800,next,sizeof(next))==0);
    setup(3);CHECK(dreadzone_model_transform_initial(&request)==1 && load_float(ram+0x3004)!=0.f);
    variable(5,0,2);memcpy(prior,ram,sizeof(ram));memcpy(spr_prior,spr,sizeof(spr));
    CHECK(dreadzone_model_transform_fixed(&request)==DREADZONE_TRANSFORM_UNSUPPORTED);
    CHECK(!memcmp(prior,ram,sizeof(ram)) && !memcmp(spr_prior,spr,sizeof(spr)));
    variable(129,0,2);refuse(DREADZONE_TRANSFORM_UNSUPPORTED);
    variable(5,12,2);refuse(DREADZONE_TRANSFORM_UNSUPPORTED);
    /* This generated fixed descriptor layout cannot address the caches of
     * these variable groups. Real9207 opcode24/48 are checked separately. */
    variable(5,24,2);refuse(DREADZONE_TRANSFORM_UNSUPPORTED);
    variable(5,48,2);refuse(DREADZONE_TRANSFORM_UNSUPPORTED);
    variable(5,0,0);refuse(DREADZONE_TRANSFORM_UNSUPPORTED);
    variable(5,0,2);put32(ram+0x44,0x80000000U);refuse(DREADZONE_TRANSFORM_UNSUPPORTED);
    variable(5,0,2);request.model.actor++;refuse(DREADZONE_TRANSFORM_INVALID);
    variable(5,0,2);put32(ram+0x24,0x400101);refuse(DREADZONE_TRANSFORM_INVALID);
    variable(5,0,2);put32(ram+0x40,0x400141);refuse(DREADZONE_TRANSFORM_INVALID);
    variable(5,0,2);put32(ram+0x58,0x4002C0);refuse(DREADZONE_TRANSFORM_INVALID);
    variable(5,0,2);{
        DreadzoneModelView split[2]={{0x400000,0x300,ram},{0x400301,sizeof(ram)-0x301,ram+0x301}};
        request.model.views=split;request.model.view_count=2;
        refuse(DREADZONE_TRANSFORM_UNMAPPED);
    }
    variable(5,0,2);{
        DreadzoneInitialModelTransform *borrowed=(DreadzoneInitialModelTransform *)(void *)(spr+16);
        memcpy(borrowed,&request,sizeof(request));
        CHECK(dreadzone_model_transform_initial(borrowed)==1 && ram[0x55]==0xD3);
    }
    /* A generated fixed stream permits actual frame consumption without
     * importing game bytes. Frame-aware output must retain the initial pose. */
    setup(3);put_float(ram+0x44,.5f);
    CHECK(dreadzone_model_transform_frame(&request)==1 && ram[0x55]==0xD3);
    CHECK(memcmp(ram+0x3000,spr,3*64)==0);
    setup(3);put_float(ram+0x44,16.5f);
    CHECK(dreadzone_model_transform_frame(&request)==1);
    setup(3);refuse_frame(0x41880000U); /* actual frame-count limit17 */
    setup(3);refuse_frame(0xBF000000U);
    setup(3);refuse_frame(0x00000001U);
    setup(3);refuse_frame(0x7F800000U);
    setup(3);refuse_frame(0x7FC00000U);
    setup(3);put32(ram+0x44,0x80000000U);
    CHECK(dreadzone_model_transform_frame(&request)==1);
    printf("initial and current model checks: %u, failures: %u\n",checks,failures);
    return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
