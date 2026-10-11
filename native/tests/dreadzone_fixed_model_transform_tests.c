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
static void refuse(int status) {
    memcpy(prior,ram,sizeof(ram));memcpy(spr_prior,spr,sizeof(spr));
    CHECK(dreadzone_model_transform_fixed(&request)==status);
    CHECK(memcmp(prior,ram,sizeof(ram))==0);CHECK(memcmp(spr_prior,spr,sizeof(spr))==0);
}
int main(void) {
    const u32 counts[]={1,2,3,107,127};u32 k,i;
    for(k=0;k<sizeof(counts)/sizeof(counts[0]);k++) {
        u32 count=counts[k];setup(count);memcpy(prior,ram,sizeof(ram));
        CHECK(dreadzone_model_transform_fixed(&request)==1);
        CHECK(ram[0x55]==0xD3);
        CHECK(load_float(ram+0x3000+4)!=0.f);
        for(i=0;i<sizeof(ram);i++)
            if(i!=0x55&&(i<0x3000||i>=0x3000+count*64))CHECK(ram[i]==prior[i]);
        CHECK(memcmp(ram+0x3000,spr,count*64)==0);
        for(i=0;i<sizeof(vu);i++)CHECK(vu[i]==0x53);
    }
    setup(3);put32(ram+0x44,0x3F800000);refuse(-2);
    setup(3);ram[0x202]=1;refuse(-2);
    setup(3);request.model.actor++;refuse(-1);
    setup(3);put32(ram+0x24,0x400101);refuse(-1);
    setup(3);put32(ram+0x40,0x400141);refuse(-1);
    setup(3);ram[0x260]=24;refuse(-2);
    setup(3);ram[0x260]=48;refuse(-2);
    setup(3);put32(ram+0x58,0x400000);refuse(-1);
    setup(3);request.vu_data=spr;refuse(-1);
    setup(3);request.vu_data=ram;refuse(-1);
    setup(3);request.vu_data_bytes=0xFFF;refuse(-1);
    setup(3);view.bytes=0x3010;refuse(-3);
    setup(3);put32(ram+0x58,0);refuse(0);
    setup(3);request.model.actor=0;refuse(0);
    setup(3);{
        DreadzoneFixedModelTransform saved=request;
        DreadzoneFixedModelTransform *borrowed=(DreadzoneFixedModelTransform *)(void *)(spr+16);
        memcpy(borrowed,&saved,sizeof(saved));
        CHECK(dreadzone_model_transform_fixed(borrowed)==1);
        CHECK(ram[0x55]==0xD3);
    }
    printf("%u checks, %u failures\n",checks,failures);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
