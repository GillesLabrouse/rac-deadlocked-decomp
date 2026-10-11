#include "dreadzone_fresh_class.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum {BASE=0x200000,SIZE=0x400000,ACT=0x300000,DEF=0x420000,ANIM=0x420140,
      STREAM=0x420200,SKEL=0x420400,DESC=0x380000,MATRIX=0x500000,STATE=0x222D78,CLS=0x23F7};
static unsigned checks,failures;
#define CHECK(x) do{checks++;if(!(x)){failures++;fprintf(stderr,"fresh class failed line %d: %s\n",__LINE__,#x);}}while(0)
static uint8_t *ram,*prior,spr[0x4000],spr_prior[0x4000],vu[0x1000];
static DreadzoneFreshClass q;
static void put(uint32_t a,uint32_t v){uint8_t *p=ram+a-BASE;p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static uint32_t get(uint32_t a){const uint8_t *p=ram+a-BASE;return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void f32(uint32_t a,float f){memcpy(ram+a-BASE,&f,4);}
static int equal_except_outputs(uint32_t count){
    uint32_t i;
    for(i=0;i<SIZE;i++){
        uint32_t a=BASE+i;
        if((a>=ACT&&a<ACT+256)||(count&&((a>=STATE&&a<STATE+40)||(a>=DESC&&a<DESC+8)||(a>=MATRIX&&a<MATRIX+count*64))))continue;
        if(ram[i]!=prior[i])return 0;
    }
    return 1;
}
static void setup(uint32_t count){
    uint32_t i;
    memset(ram,0xA5,SIZE);memset(spr,0,sizeof(spr));memset(vu,0x53,sizeof(vu));memset(ram+DEF-BASE,0,0x4C);
    ram[0x26B590+CLS-BASE]=7;ram[0x26B2B0+14-BASE]=(uint8_t)(CLS&255);ram[0x26B2B0+15-BASE]=(uint8_t)(CLS>>8);
    put(0x26DD90+28,0x12345678);put(0x26AE00+28,0x87654321);put(0x26AA40+28,DEF);put(0x222C50,0);
    put(ACT+0x58,0);ram[ACT+0x20-BASE]=0x42;ram[ACT+0x2D-BASE]=0xAB;
    ram[DEF+8-BASE]=(uint8_t)count;ram[DEF+6-BASE]=1;ram[DEF+0xF-BASE]=1;
    put(DEF+0x18,SKEL);put(DEF+0x48,ANIM);put(DEF+0x40,0x450100);
    memset(ram+ANIM-BASE,0,0x20);ram[ANIM+0x10-BASE]=17;put(ANIM+0x1C,STREAM);
    /* Synthetic compressed fixed-only stream and skeleton, no retail bytes. */
    memset(ram+STREAM-BASE,0,7*16);ram[STREAM-BASE]=2;ram[STREAM+1-BASE]=7;
    ram[STREAM+3-BASE]=4;ram[STREAM+4-BASE]=2;ram[STREAM+7-BASE]=2;ram[STREAM+8-BASE]=1;
    for(i=0;i<4;i++){
        ram[STREAM+0x16+i*16-BASE]=0xFF;ram[STREAM+0x17+i*16-BASE]=0x7F;
        ram[STREAM+0x1E+i*16-BASE]=0xFF;ram[STREAM+0x1F+i*16-BASE]=0x7F;
    }
    for(i=0;i<16;i++)ram[STREAM+0x50+i-BASE]=(uint8_t)(128+(i%8)*4);
    for(i=0;i<count&&i<128;i++){f32(SKEL+i*16,1.25f);f32(SKEL+i*16+4,-2.5f);f32(SKEL+i*16+8,.125f);put(SKEL+i*16+12,i?i-1:127);}
    put(STATE,DESC);put(STATE+4,MATRIX);put(STATE+8,MATRIX);put(STATE+12,0x3094);
    put(STATE+16,0);put(STATE+20,0);put(STATE+24,0);put(STATE+28,0x7FFFFFFF);put(STATE+32,0x13579BDF);put(STATE+36,0x2468ACE0);
    q.storage=ram;q.guest_base=BASE;q.bytes=SIZE;q.gp=0x22597F;q.actor=ACT;q.object_class=CLS;q.skip_cleanup=0;
    q.scratch=spr;q.scratch_bytes=sizeof(spr);q.vu_data=vu;q.vu_data_bytes=sizeof(vu);
}
static void refuse(int result){
    memcpy(prior,ram,SIZE);memcpy(spr_prior,spr,sizeof(spr));
    CHECK(dreadzone_fresh_class_initialize(&q)==result);CHECK(!memcmp(prior,ram,SIZE));CHECK(!memcmp(spr_prior,spr,sizeof(spr)));
}
int main(void){
    const uint32_t counts[]={0,1,2,107,127};uint32_t k,i;
    ram=(uint8_t *)malloc(SIZE);prior=(uint8_t *)malloc(SIZE);if(!ram||!prior){free(ram);free(prior);return 1;}
    for(k=0;k<sizeof(counts)/sizeof(counts[0]);k++){
        uint32_t count=counts[k];setup(count);memcpy(prior,ram,SIZE);memcpy(spr_prior,spr,sizeof(spr));
        CHECK(dreadzone_fresh_class_initialize(&q)==1);
        CHECK(ram[ACT+0x2D-BASE]==0xAB&&get(ACT+0x38)==0&&get(ACT+0x3C)==0x00404040);
        CHECK(get(ACT+0xA8)==0x87654321&&get(ACT+0x24)==DEF&&ram[ACT+0x22-BASE]==7);
        CHECK(get(ACT+0x60)==0x450100&&ram[ACT+0x66-BASE]==0x18&&ram[ACT+0x76-BASE]==0x18);
        CHECK(equal_except_outputs(count));
        if(count){CHECK(get(ACT+0x58)==MATRIX&&ram[ACT+0x55-BASE]==0x40);CHECK(get(STATE+20)==1&&get(STATE+12)==0x3094-count);CHECK(!memcmp(ram+MATRIX-BASE,spr,count*64));CHECK(get(ACT+0x48)==0x3F800000);}
        else{CHECK(get(ACT+0x58)==0&&get(ACT+0x48)==0&&(ram[ACT+0x34-BASE]&0x40));CHECK(!memcmp(spr,spr_prior,sizeof(spr)));}
        CHECK(get(STATE+32)==0x13579BDF&&get(STATE+36)==0x2468ACE0);
        for(i=0;i<sizeof(vu);i++)CHECK(vu[i]==0x53);
    }
    setup(2);put(DEF+0x1C,0x450000);put(0x450000,1);CHECK(dreadzone_fresh_class_initialize(&q)==1&&ram[ACT+0x55-BASE]==0);
    setup(0);ram[ACT+0x20-BASE]=0xFF;CHECK(dreadzone_fresh_class_initialize(&q)==1&&ram[ACT+0x2D-BASE]==0);
    setup(0);ram[0x26B2B0+14-BASE]=0;CHECK(dreadzone_fresh_class_initialize(&q)==1&&get(ACT+0x24)==0&&get(ACT+0xA8)==0x12345678);
    setup(2);put(ACT+0x58,1);refuse(-3);
    setup(128);refuse(-3);
    setup(2);ram[STREAM+2-BASE]=1;refuse(-3);
    setup(2);put(DEF+0x1C,BASE+SIZE);refuse(-2);
    setup(2);put(DEF+0x10,ACT);refuse(-3);
    setup(2);put(0x222C50,ACT);refuse(-3);
    setup(2);q.scratch_bytes=0x3FFF;refuse(-1);
    setup(2);q.vu_data=spr;refuse(-1);
    setup(2);q.actor=BASE+SIZE;refuse(-2);
    setup(2);q.object_class=0x10000;refuse(-1);
    setup(2);{
        DreadzoneFreshClass *alias=(DreadzoneFreshClass *)(ram+ACT-BASE);q.skip_cleanup=1;
        memcpy(alias,&q,sizeof(q));CHECK(dreadzone_fresh_class_initialize(alias)==1&&get(ACT+0x58)==MATRIX);
    }
    CHECK(dreadzone_fresh_class_initialize(NULL)==-1);
    free(prior);free(ram);printf("fresh class checks: %u, failures: %u\n",checks,failures);return failures?1:0;
}
