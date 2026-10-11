#include "dreadzone_actor_slope.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static unsigned checks,failures;
#define CHECK(x) do {++checks;if(!(x)){++failures;printf("line %d: %s\n",__LINE__,#x);}}while(0)
typedef struct Fixture {
    u8 record[0x3210],time[4],tables[0x80];
    DreadzoneActorInputMemory views[2];DreadzoneActorInputContext context;
} Fixture;
static void word(u8 *p,u32 v){memcpy(p,&v,4);}
static void floating(u8 *p,f32 v){memcpy(p,&v,4);}
static f32 get(const u8 *p){f32 v;memcpy(&v,p,4);return v;}
static u32 bits(const u8 *p){u32 v;memcpy(&v,p,4);return v;}
static void setup(Fixture *f) {
    static const f32 coefficients[8]={
        0.999999344348907470703125f,-0.33329856395721435546875f,
        0.1994653642177581787109375f,-0.13908533751964569091796875f,
        0.096420042216777801513671875f,-0.05590988695621490478515625f,
        0.0218612290918827056884765625f,-0.0040540578775107860565185546875f};
    static const f32 quadrants[16]={1.f,0.f,-1.f,1.57079637050628662109375f,
        -1.f,0.f,1.f,-1.57079637050628662109375f,
        -1.f,3.1415927410125732421875f,1.f,1.57079637050628662109375f,
        1.f,-3.1415927410125732421875f,-1.f,-1.57079637050628662109375f};
    memset(f,0,sizeof(*f));memset(f->record+0x31F0,0xCD,32);
    memcpy(f->tables,coefficients,sizeof(coefficients));memcpy(f->tables+0x40,quadrants,sizeof(quadrants));
    f->views[0]=(DreadzoneActorInputMemory){f->time,0x21DDF8,sizeof(f->time),0};
    f->views[1]=(DreadzoneActorInputMemory){f->tables,0x3B9540,sizeof(f->tables),0};
    f->context=(DreadzoneActorInputContext){f->record,0x368F60,0x31F0,f->views,2};
    f->record[0x2FE]=1;f->record[0x226]=1;floating(f->record+0x158,1.f);
    floating(f->record+0x1D0,1.f);floating(f->record+0x1D8,.5f);
    floating(f->record+0x130,-1.f);floating(f->record+0x138,.25f);
    word(f->record+0x13C,0x7FA12345);floating(f->record+0x25C4,.5f);floating(f->time,.125f);
}
static int execute(Fixture *f){return dreadzone_actor_slope_0067D4E8(&f->context);}
static void refusal(Fixture *f,int expected) {
    Fixture before=*f;
    CHECK(execute(f)==expected);CHECK(!memcmp(before.record,f->record,sizeof(f->record)));
    CHECK(!memcmp(before.time,f->time,sizeof(f->time)));CHECK(!memcmp(before.tables,f->tables,sizeof(f->tables)));
}
int main(void) {
    Fixture f;u8 before[0x3210],expected[0x3210];u32 i;
    setup(&f);memcpy(expected,f.record,sizeof(expected));floating(expected+0x130,0.f);
    floating(expected+0x138,-6.5f);CHECK(execute(&f)==1);CHECK(!memcmp(expected,f.record,sizeof(expected)));
    CHECK(bits(f.record+0x13C)==0x7FA12345);
    setup(&f);floating(f.record+0x1D0,67108864.f);floating(f.record+0x1D8,33554432.f);
    CHECK(execute(&f)==1);CHECK(get(f.record+0x130)==0.f);CHECK(get(f.record+0x138)==-6.5f);
    /* Each actual gate stops before reaching unusable later fields/views. */
    for(i=0;i<5;++i) {
        setup(&f);f.context.view_count=0;
        if(i==0){f.record[0x2FE]=0;word(f.record+0x2CC,0x7FA12345);}
        if(i==1){floating(f.record+0x2CC,.15f);word(f.record+0x158,0x7FA12345);}
        if(i==2){floating(f.record+0x158,-0.f);word(f.record+0x1D8,0x7FA12345);}
        if(i==3){f.record[0x226]=0;word(f.record+0x1D8,0x7FA12345);}
        if(i==4){floating(f.record+0x1D8,-0.f);word(f.record+0x1D0,0x7FA12345);}
        memcpy(before,f.record,sizeof(before));CHECK(execute(&f)==0);CHECK(!memcmp(before,f.record,sizeof(before)));
    }
    /* Below angle and outward velocity avoid the genuine time reload. */
    setup(&f);floating(f.record+0x1D8,2.f);f.views[0].guest_base=0x220000;
    memcpy(before,f.record,sizeof(before));CHECK(execute(&f)==1);CHECK(!memcmp(before,f.record,sizeof(before)));
    setup(&f);floating(f.record+0x130,1.f);f.views[0].guest_base=0x220000;
    word(f.record+0x138,0x7FA12345);memcpy(before,f.record,sizeof(before));
    CHECK(execute(&f)==1);CHECK(!memcmp(before,f.record,sizeof(before)));
    /* Plane zero path retains signed zeros and does not evaluate raw Z/W velocity. */
    setup(&f);floating(f.record+0x1D0,-0.f);floating(f.record+0x1D4,-0.f);
    word(f.record+0x138,0x7FA12345);f.views[0].guest_base=0x220000;
    memcpy(before,f.record,sizeof(before));CHECK(execute(&f)==1);CHECK(!memcmp(before,f.record,sizeof(before)));
    /* Genuine global alias sees the first corrected Z store. */
    setup(&f);f.context.record_guest=0x21DCC0;f.views[0]=f.views[1];f.context.view_count=1;
    CHECK(execute(&f)==1);CHECK(get(f.record+0x138)==-13.25f);CHECK(bits(f.record+0x13C)==0x7FA12345);
    setup(&f);f.views[0].guest_base=0x220000;refusal(&f,-2);
    setup(&f);f.context.view_count=1;refusal(&f,-2);
    setup(&f);floating(f.tables,1.f);refusal(&f,-5);
    setup(&f);floating(f.record+0x1D8,1.0001f);refusal(&f,-3);
    setup(&f);word(f.record+0x130,1);refusal(&f,-3);
    setup(&f);word(f.time,0x7F800000);refusal(&f,-3);
    setup(&f);f.context.record_guest=0xFFFFF000;refusal(&f,-1);
    setup(&f);f.views[0].storage=f.record;refusal(&f,-1);
    setup(&f);f.views[0].guest_base=f.context.record_guest;refusal(&f,-1);
    setup(&f);f.context.view_count=65;refusal(&f,-1);
    CHECK(dreadzone_actor_slope_0067D4E8(NULL)==-1);
    printf("%u checks, %u failures\n",checks,failures);return failures?1:0;
}
