#include "dreadzone_actor_walk_parameters.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static unsigned checks,failures;
#define CHECK(x) do {++checks;if(!(x)){++failures;printf("line %d: %s\n",__LINE__,#x);}}while(0)
typedef struct Fixture {
    u8 record[0x3210],input[0x5C0],globals[12];
    DreadzoneActorInputMemory views[2];
    DreadzoneActorInputContext context;
} Fixture;
static void word(u8 *p,u32 value){memcpy(p,&value,4);}
static void floating(u8 *p,f32 value){memcpy(p,&value,4);}
static f32 get(const u8 *p){f32 value;memcpy(&value,p,4);return value;}
static void setup(Fixture *f) {
    memset(f,0,sizeof(*f));memset(f->record+0x31F0,0xCD,32);
    f->views[0]=(DreadzoneActorInputMemory){f->input,0x1EE600,sizeof(f->input),0};
    f->views[1]=(DreadzoneActorInputMemory){f->globals,0x21DDEC,sizeof(f->globals),0};
    f->context=(DreadzoneActorInputContext){f->record,0x368F60,0x31F0,f->views,2};
    word(f->record+0x25CC,2);word(f->record+0x2F00,0x1EE600);
    word(f->record+0x2314,2);word(f->record+0x2318,2);
    floating(f->record+0x25C4,1.f);floating(f->record+0x1B4,.25f);
    floating(f->globals,.5f);floating(f->globals+8,.1f);
    word(f->input+0x190,2);word(f->input+0x1E0+29*4,0x28);
}
static int untouched_tail(const Fixture *f){unsigned i;for(i=0;i<32;++i)if(f->record[0x31F0+i]!=0xCD)return 0;return 1;}
static void refused(Fixture *f,int expected) {
    u8 before[sizeof(f->record)],input[sizeof(f->input)],globals[sizeof(f->globals)];
    memcpy(before,f->record,sizeof(before));memcpy(input,f->input,sizeof(input));memcpy(globals,f->globals,sizeof(globals));
    CHECK(dreadzone_actor_walk_parameters_00681B30(&f->context)==expected);
    CHECK(!memcmp(before,f->record,sizeof(before)));
    CHECK(!memcmp(input,f->input,sizeof(input)));CHECK(!memcmp(globals,f->globals,sizeof(globals)));
}
int main(void) {
    Fixture f;u8 untouched[0x31F0];f32 expected;u32 i;
    DreadzoneActorInputContext *embedded;
    setup(&f);memcpy(untouched,f.record,sizeof(untouched));
    CHECK(dreadzone_actor_walk_parameters_00681B30(&f.context)==1);
    CHECK(get(f.record+0xA24)==.04f);CHECK(get(f.record+0xAD4)==.02f);
    CHECK(get(f.record+0xAD8)==.2f);CHECK(get(f.record+0x938)==0.f);
    CHECK(!memcmp(f.record+0x120,untouched+0x120,0x90));CHECK(untouched_tail(&f));
    setup(&f);word(f.record+0x25D0,1);floating(f.record+0x17A4,.02f);floating(f.record+0x17A8,.5f);
    CHECK(dreadzone_actor_walk_parameters_00681B30(&f.context)==1);
    expected=-(.02f*17.f)*.11f*(.25f/(.1f*5.7f));
    CHECK(fabsf(get(f.record+0x880)-expected)<.0000001f);
    CHECK(fabsf(get(f.record+0x938)-(.02f*17.f)*.31f)<.0000001f);
    CHECK(fabsf(get(f.record+0xA94)-(.02f*17.f)*-.15f)<.0000001f);
    setup(&f);word(f.record+0x25D0,1);f.record[0x1C8F]=1;
    floating(f.record+0x17A4,.02f);floating(f.record+0x17A8,.5f);
    CHECK(dreadzone_actor_walk_parameters_00681B30(&f.context)==1);
    CHECK(get(f.record+0x938)==0.f);CHECK(get(f.record+0x978)==.5f*.3f);
    CHECK(get(f.record+0x974)==.5f*.055f);CHECK(get(f.record+0xAD4)==.5f*.07f);
    setup(&f);f.record[0x5F4]=1;floating(f.record+0x2E08,.5f);
    floating(f.record+0x5F8,.5f);floating(f.record+0xB8,.125f);
    CHECK(dreadzone_actor_walk_parameters_00681B30(&f.context)==1);
    CHECK(get(f.record+0x888)==.375f);CHECK(get(f.record+0x938)==-.375f);
    CHECK(get(f.record+0x8C4)==.5f*.037f);CHECK(get(f.record+0xA24)==0.f);
    setup(&f);word(f.record+0x1AE0,1);floating(f.record+0x2E08,.5f);
    floating(f.record+0x5F8,-.5f);floating(f.record+0xB8,.125f);
    CHECK(dreadzone_actor_walk_parameters_00681B30(&f.context)==1);CHECK(get(f.record+0x888)==-.625f);
    setup(&f);word(f.record+0x1AE0,1);word(f.record+0x2318,1);floating(f.record+0x2E08,.5f);
    CHECK(dreadzone_actor_walk_parameters_00681B30(&f.context)==1);CHECK(get(f.record+0xA24)==.04f);
    setup(&f);f.context.view_count=0;f.context.views=0;
    CHECK(dreadzone_actor_walk_parameters_00681B30(&f.context)==1);
    setup(&f);f.context.record_guest=0x0021DDF4U-0x8C4U;f.context.view_count=1;word(f.record+0x25D0,1);
    floating(f.record+0x17A4,.02f);floating(f.record+0x17A8,.5f);floating(f.record+0x8C4,.5f);
    CHECK(dreadzone_actor_walk_parameters_00681B30(&f.context)==1);
    expected=-(.02f*17.f)*.11f; /* The real ratio exceeds one and clamps. */
    CHECK(fabsf(get(f.record+0x880)-expected)<.0000001f);
    for(i=0;i<4;++i) {
        setup(&f);word(f.record+0x25D0,1);
        floating(f.record+0x17A8,i&1?-0.f:0.f);floating(f.record+0x17A4,i&2?-0.f:0.f);
        CHECK(dreadzone_actor_walk_parameters_00681B30(&f.context)==1);
        CHECK(get(f.record+0x938)==0.f);CHECK(untouched_tail(&f));
    }
    setup(&f);word(f.record+0x25CC,8);refused(&f,-3);
    setup(&f);f.record[0x2659]=1;refused(&f,-3);
    setup(&f);word(f.record+0x25D0,1);floating(f.globals+8,0.f);refused(&f,-4);
    setup(&f);word(f.record+0x25D0,1);f.context.view_count=1;refused(&f,-2);
    setup(&f);word(f.record+0x17A8,0x7F800000U);refused(&f,-4);
    setup(&f);floating(f.record+0x2E08,.5f);word(f.record+0x370,0xFFFFFFFFU);refused(&f,-3);
    setup(&f);f.context.record_guest=0xFFFFFFF0U;refused(&f,-1);
    setup(&f);f.views[0].guest_base=f.context.record_guest;refused(&f,-1);
    setup(&f);f.views[0].storage=f.record;refused(&f,-1);
    setup(&f);f.context.views=(const DreadzoneActorInputMemory *)(f.record+0x900);refused(&f,-1);
    setup(&f);embedded=(DreadzoneActorInputContext *)(f.record+0x900);
    memcpy(embedded,&f.context,sizeof(f.context));memcpy(untouched,f.record,sizeof(untouched));
    CHECK(dreadzone_actor_walk_parameters_00681B30(embedded)==-1);
    CHECK(!memcmp(untouched,f.record,sizeof(untouched)));
    setup(&f);f.context.record=(u8 *)(UINTPTR_MAX-15U);
    refused(&f,-1);
    setup(&f);f.context.views=(const DreadzoneActorInputMemory *)(UINTPTR_MAX-15U);
    refused(&f,-1);
    printf("Walk parameter checks: %u, failures: %u\n",checks,failures);return failures?1:0;
}
