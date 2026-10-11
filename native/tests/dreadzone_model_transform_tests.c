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
    u32 k;
    const u32 counts[]={1,2,3,107,127};
    /* Zero variable qwords do not imply an undecoded identity pose. These
     * previously successful domains must refuse without publishing anything. */
    for (k=0;k<sizeof(counts)/sizeof(counts[0]);k++) {
        setup(counts[k]); unchanged(DREADZONE_TRANSFORM_UNSUPPORTED);
    }
    setup(107); arena[0x201]=88;
    unchanged(DREADZONE_TRANSFORM_UNSUPPORTED);
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
    setup(3); put32(arena+0x58,0x400000); unchanged(-2);
    setup(3); put32(arena+0x58,0x403001); unchanged(-2);
    setup(3); view.bytes=0x3010; unchanged(-2);
    setup(3); view.bytes=0x5F; unchanged(-3);
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
        /* A borrowed descriptor remains unchanged on refusal. */
        DreadzoneModelTransform saved=request;
        DreadzoneModelTransform *inside=(DreadzoneModelTransform *)(void *)(scratch+16);
        memcpy(inside,&saved,sizeof(saved));
        memcpy(before,arena,sizeof(arena)); memcpy(scratch_before,scratch,sizeof(scratch));
        CHECK(dreadzone_model_transform_static(inside)==DREADZONE_TRANSFORM_UNSUPPORTED);
        CHECK(memcmp(before,arena,sizeof(arena))==0);
        CHECK(memcmp(scratch_before,scratch,sizeof(scratch))==0);
    }
    printf("%u checks, %u failures\n",checks,failures);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
