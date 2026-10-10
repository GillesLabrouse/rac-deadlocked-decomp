#include "dreadzone_model_transform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks, failures;
#define CHECK(x) do { checks++; if (!(x)) { failures++; \
    printf("FAIL line%d: %s\n", __LINE__, #x); } } while (0)
static void put32(u8 *p, u32 value) {
    p[0]=(u8)value; p[1]=(u8)(value>>8); p[2]=(u8)(value>>16); p[3]=(u8)(value>>24);
}
static void put_float(u8 *p, float value) { memcpy(p,&value,4); }
static float get_float(const u8 *p) { float value; memcpy(&value,p,4); return value; }

static u8 arena[0x6000], scratch[0x4002], before[0x6000], scratch_before[0x4002];
static DreadzoneModelView view;
static DreadzoneModelTransform request;
static void setup(u32 count) {
    u32 i;
    for (i=0;i<sizeof(arena);i++) arena[i]=(u8)(i*17+91);
    for (i=0;i<sizeof(scratch);i++) scratch[i]=(u8)(i*13+27);
    memset(arena,0,0x60);
    put32(arena+0x24,0x400100); put32(arena+0x40,0x400140);
    put32(arena+0x58,0x403000); arena[0x55]=0xF3; arena[0x57]=(u8)count;
    arena[0x108]=(u8)count; put32(arena+0x118,0x400400);
    arena[0x150]=17; put32(arena+0x15C,0x400200);
    arena[0x201]=4; arena[0x202]=0;
    for (i=0;i<count;i++) {
        u8 *s=arena+0x400+i*16;
        put_float(s,1.25f); put_float(s+4,-2.5f); put_float(s+8,0.125f);
        put32(s+12,i==0 ? 0xAABBCCFF : 0xAABBCC00U|((i-1)&127)|(i&1 ? 128 : 0));
    }
    view.guest_base=0x400000; view.bytes=sizeof(arena); view.storage=arena;
    request.views=&view; request.view_count=1; request.actor=0x400000;
    request.scratch=scratch+1; request.scratch_bytes=0x4000;
}
static void unchanged(int expected) {
    memcpy(before,arena,sizeof(arena)); memcpy(scratch_before,scratch,sizeof(scratch));
    CHECK(dreadzone_model_transform_static(&request)==expected);
    CHECK(memcmp(before,arena,sizeof(arena))==0);
    CHECK(memcmp(scratch_before,scratch,sizeof(scratch))==0);
}
int main(void) {
    u32 count,i,j,k;
    const u32 counts[]={1,2,3,107,127};
    for (k=0;k<sizeof(counts)/sizeof(counts[0]);k++) {
        count=counts[k]; setup(count);
        memcpy(before,arena,sizeof(arena)); memcpy(scratch_before,scratch,sizeof(scratch));
        CHECK(dreadzone_model_transform_static(&request)==DREADZONE_TRANSFORM_APPROXIMATE);
        CHECK(arena[0x55]==0xD3);
        for (i=0;i<sizeof(arena);i++)
            if (i!=0x55 && (i<0x3000 || i>=0x3000+count*64)) CHECK(arena[i]==before[i]);
        for (i=0;i<count;i++) {
            u8 *matrix=arena+0x3000+i*64;
            for (j=0;j<16;j++) {
                float expected=(j%5==0 ? 1.0f : 0.0f);
                if (j==12) expected=1.25f*(float)(i+1);
                if (j==13) expected=-2.5f*(float)(i+1);
                if (j==14) expected=0.125f*(float)(i+1);
                CHECK(get_float(matrix+j*4)==expected);
            }
            CHECK(memcmp(matrix,scratch+1+i*64,64)==0);
            CHECK(memcmp(matrix,scratch+1+0x2000+i*64,64)==0);
        }
        for (i=0;i<16;i++) CHECK(get_float(scratch+1+0x3FC0+i*4)==(i%5==0 ? 1.0f : 0.0f));
        CHECK(scratch[0]==scratch_before[0]); CHECK(scratch[0x4001]==scratch_before[0x4001]);
        for (i=0;i<0x4000;i++)
            if (i>=count*64 && !(i>=0x2000 && i<0x2000+count*64) && i<0x3FC0)
                CHECK(scratch[1+i]==scratch_before[1+i]);
    }
    setup(3); arena[0x410+12]=0xFF;
    CHECK(dreadzone_model_transform_static(&request)==1);
    CHECK(get_float(arena+0x3000+64+48)==1.25f);
    CHECK(get_float(arena+0x3000+128+48)==2.5f);
    setup(1); arena[0x201]=88;
    memcpy(before,arena,sizeof(arena));
    CHECK(dreadzone_model_transform_static(&request)==1);
    for (i=64;i<88*16;i++) CHECK(scratch[1+0x2000+i]==before[0x200+i]);
    setup(3); arena[0x55]|=4; unchanged(-2);
    setup(3); put32(arena+0x50,0x12345678); unchanged(-2);
    setup(3); put32(arena+0x5C,0x12345678); unchanged(-2);
    setup(3); arena[0x202]=1; unchanged(-2);
    setup(3); arena[0x57]=2; unchanged(-2);
    setup(3); arena[0x400+12]=0x80; unchanged(-2);
    setup(3); arena[0x410+12]=0x82; unchanged(-2);
    setup(3); put32(arena+0x400,0x7F800000); unchanged(-2);
    setup(3); put32(arena+0x400,1); unchanged(-2);
    setup(3); put32(arena+0x400,0x7F000000); unchanged(-2);
    setup(127); put32(arena+0x400+126*16,0x7F800000); unchanged(-2);
    setup(3); put32(arena+0x58,0x400000); unchanged(-1);
    setup(3); put32(arena+0x58,0x403001); unchanged(-1);
    setup(3); view.bytes=0x3010; unchanged(-3);
    setup(3); request.scratch_bytes=0x3FFF; unchanged(-1);
    setup(3); request.scratch=arena; unchanged(-1);
    setup(3); view.guest_base=0xFFFFFFF0; unchanged(-1);
    setup(3); request.views=(const DreadzoneModelView *)(UINTPTR_MAX-3); unchanged(-1);
    setup(3); {
        memcpy(before,arena,sizeof(arena)); memcpy(scratch_before,scratch,sizeof(scratch));
        CHECK(dreadzone_model_transform_static((const DreadzoneModelTransform *)(UINTPTR_MAX-3))==-1);
        CHECK(memcmp(before,arena,sizeof(arena))==0);
        CHECK(memcmp(scratch_before,scratch,sizeof(scratch))==0);
    }
    setup(3); put32(arena+0x58,0); unchanged(0);
    setup(3); request.actor=0; unchanged(0);
    setup(3); {
        DreadzoneModelView duplicated[2]={view,view};
        request.views=duplicated; request.view_count=2; unchanged(-1);
    }
    setup(3); {
        /* A borrowed descriptor in scratch is captured before publication. */
        DreadzoneModelTransform saved=request;
        DreadzoneModelTransform *inside=(DreadzoneModelTransform *)(void *)(scratch+16);
        memcpy(inside,&saved,sizeof(saved));
        CHECK(dreadzone_model_transform_static(inside)==1);
        CHECK(get_float(arena+0x3000+128+48)==3.75f);
    }
    printf("%u checks, %u failures\n",checks,failures);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
