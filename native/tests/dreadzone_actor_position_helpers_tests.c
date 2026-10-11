#include "dreadzone_actor_position_helpers.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static unsigned checks,failures;
#define CHECK(x) do {++checks;if(!(x)){++failures;printf("line %d: %s\n",__LINE__,#x);}}while(0)
typedef struct Fixture {
    u8 record[0x3210],body[0x200],metadata[0x40],globals[16];
    DreadzoneActorInputMemory views[3];
    DreadzoneActorInputContext context;
    DreadzonePositionHelperRequest request;
    f32 returned;
} Fixture;
static void word(u8 *p,u32 v){memcpy(p,&v,4);}
static void floating(u8 *p,f32 v){memcpy(p,&v,4);}
static f32 get(const u8 *p){f32 v;memcpy(&v,p,4);return v;}
static u32 bits(const u8 *p){u32 v;memcpy(&v,p,4);return v;}
static void setup(Fixture *f,u32 operation) {
    memset(f,0,sizeof(*f));memset(f->record+0x31F0,0xCD,32);
    f->views[0]=(DreadzoneActorInputMemory){f->body,0x230000,sizeof(f->body),1};
    f->views[1]=(DreadzoneActorInputMemory){f->metadata,0x800000,sizeof(f->metadata),0};
    f->views[2]=(DreadzoneActorInputMemory){f->globals,0x21DDE8,sizeof(f->globals),0};
    f->context=(DreadzoneActorInputContext){f->record,0x368F60,0x31F0,f->views,3};
    f->request.operation=operation;f->returned=123.5f;
    f->request.value=0x369760;f->request.target=0x369780;f->request.rate=0x369860;
    f->request.scalar[0]=.125f;f->request.scalar[1]=.25f;f->request.scalar[2]=1.f;
    word(f->record+0x2EF0,0x230000);word(f->record+0x2EEC,0x800000);
    word(f->record+0x25CC,2);floating(f->record+0x25C4,1.f);
    floating(f->globals,1.f);floating(f->globals+4,.5f);floating(f->globals+12,.1f);
    floating(f->metadata+0x10,.5f);floating(f->metadata+0x14,.75f);
    floating(f->metadata+0x18,1.f);floating(f->metadata+0x1C,1.25f);
}
static int execute(Fixture *f){return dreadzone_actor_position_helper(&f->context,&f->request,&f->returned);}
static void refusal(Fixture *f,int expected) {
    u8 record[sizeof(f->record)],body[sizeof(f->body)],meta[sizeof(f->metadata)],globals[sizeof(f->globals)];
    f32 returned=f->returned;
    memcpy(record,f->record,sizeof(record));memcpy(body,f->body,sizeof(body));
    memcpy(meta,f->metadata,sizeof(meta));memcpy(globals,f->globals,sizeof(globals));
    CHECK(execute(f)==expected);CHECK(!memcmp(record,f->record,sizeof(record)));
    CHECK(!memcmp(body,f->body,sizeof(body)));CHECK(!memcmp(meta,f->metadata,sizeof(meta)));
    CHECK(!memcmp(globals,f->globals,sizeof(globals)));CHECK(f->returned==returned);
}
int main(void) {
    Fixture f;u8 expected[0x3210];u32 i;clock_t started=clock();
    /* Raw projection transports all other lanes without interpreting them. */
    setup(&f,DREADZONE_POSITION_PROJECT);word(f.record+0x820,0x7FC00001);
    word(f.record+0x824,0x80000000);word(f.record+0x828,0x7F800000);word(f.record+0x82C,0xDEADBEEF);
    memcpy(expected,f.record,sizeof(expected));memcpy(expected+0x800,expected+0x820,16);word(expected+0x808,0);
    CHECK(execute(&f)==1);CHECK(!memcmp(expected,f.record,sizeof(expected)));CHECK(f.returned==123.5f);
    /* Offset reloads body heading after copying the source onto that body. */
    setup(&f,DREADZONE_POSITION_OFFSET);f.request.value=0x2300F0;
    floating(f.record+0x820,1.f);floating(f.record+0x824,2.f);floating(f.record+0x828,.125f);
    word(f.record+0x82C,0xFEEDBABE);CHECK(execute(&f)==1);
    CHECK(fabsf(get(f.body+0xF0)-(1.f+cosf(.125f)*.125f))<.0000002f);
    CHECK(fabsf(get(f.body+0xF4)-(2.f+sinf(.125f)*.125f))<.0000002f);
    CHECK(get(f.body+0xF8)==.125f);CHECK(bits(f.body+0xFC)==0xFEEDBABE);
    /* EEA0 receives all four original register lanes; z signed zero matters. */
    setup(&f,DREADZONE_POSITION_HEADING);word(f.body+0xF8,0x80000000);
    for(i=0;i<3;++i)word(f.record+0x820+i*4,0x80000000);
    word(f.record+0x82C,0x7FC00001);CHECK(execute(&f)==1);
    CHECK(signbit(f.returned)!=0);CHECK(f.returned==0.f);
    setup(&f,DREADZONE_POSITION_HEADING);floating(f.record+0x820,2.f);
    CHECK(execute(&f)==1);CHECK(f.returned==2.f);
    /* Genuine metadata initializes radius and radius squared. No input radius. */
    setup(&f,DREADZONE_POSITION_RADIUS);f.request.initial=1;f.record[0x226]=1;
    CHECK(execute(&f)==1);CHECK(get(f.record+0x1F0)==.75f);CHECK(get(f.record+0x1F4)==1.f);
    CHECK(get(f.record+0x204)==.5f);CHECK(get(f.record+0x240)==.25f);CHECK(f.returned==123.5f);
    setup(&f,DREADZONE_POSITION_RADIUS);word(f.record+0x25CC,6);f.request.initial=1;f.record[0x226]=1;
    CHECK(execute(&f)==1);CHECK(get(f.record+0x1FC)==1.25f);CHECK(get(f.record+0x1F4)==1.25f);
    setup(&f,DREADZONE_POSITION_RADIUS);
    CHECK(execute(&f)==1);CHECK(get(f.record+0x1F0)==.02f);CHECK(get(f.record+0x1F4)==.02f);
    CHECK(get(f.record+0x204)>0.f);CHECK(get(f.record+0x240)==get(f.record+0x204)*get(f.record+0x204));
    /* Settling follows the real height/history branches; it is not a probe. */
    setup(&f,DREADZONE_POSITION_SETTLE);floating(f.record+0xA8,.5f);floating(f.record+0x2C8,.505f);
    CHECK(execute(&f)==1);CHECK(get(f.record+0xA8)==.505f);CHECK(get(f.record+0x304)==.505f);
    setup(&f,DREADZONE_POSITION_SETTLE);word(f.record+0x25D4,1);word(f.record+0x25CC,4);
    floating(f.record+0x304,1.f);floating(f.record+0x308,2.f);word(f.record+0x300,1);
    CHECK(execute(&f)==1);CHECK(bits(f.record+0x300)==0);CHECK(bits(f.record+0x304)==0);CHECK(bits(f.record+0x308)==0);
    setup(&f,DREADZONE_POSITION_SETTLE);floating(f.record+0xA8,1.f);
    floating(f.record+0x304,1.02f);floating(f.record+0x308,.995f);
    CHECK(execute(&f)==1);CHECK(bits(f.record+0x300)==1);CHECK(get(f.record+0xA8)==1.02f);
    /* Value/rate alias executes both original stores in their original order. */
    setup(&f,DREADZONE_POSITION_SCALAR);f.request.rate=f.request.value;
    f.request.scalar[0]=1.f;f.request.scalar[1]=0.f;f.request.scalar[2]=0.f;f.request.scalar[3]=1.f;
    floating(f.record+0x800,.125f);CHECK(execute(&f)==1);
    CHECK(get(f.record+0x800)==.25f);CHECK(f.returned==.75f);
    setup(&f,DREADZONE_POSITION_VECTOR);f.request.scalar[0]=0.f;f.request.scalar[1]=0.f;
    f.request.scalar[2]=1.f;floating(f.record+0x820,1.f);floating(f.record+0x900,.125f);
    word(f.record+0x80C,0xFEEDBABE);CHECK(execute(&f)==1);
    CHECK(get(f.record+0x800)==.125f);CHECK(bits(f.record+0x80C)==0xFEEDBABE);CHECK(f.returned==.875f);
    /* All refusals precede publication, including failures after shadow writes. */
    setup(&f,DREADZONE_POSITION_RADIUS);word(f.record+0x25C4,0x7FC00001);refusal(&f,-4);
    setup(&f,DREADZONE_POSITION_OFFSET);f.views[0].writable=0;f.request.value=0x2300F0;refusal(&f,-2);
    setup(&f,DREADZONE_POSITION_RADIUS);f.views[1].bytes=0x14;refusal(&f,-2);
    setup(&f,DREADZONE_POSITION_OFFSET);word(f.record+0x2EF0,0xFFFFFFFC);refusal(&f,-1);
    setup(&f,DREADZONE_POSITION_OFFSET);f.request.value++;refusal(&f,-1);
    setup(&f,DREADZONE_POSITION_OFFSET);f.record[0x2659]=1;refusal(&f,-3);
    setup(&f,DREADZONE_POSITION_RADIUS);f.views[1].guest_base=0xFFFFFFF0;refusal(&f,-1);
    setup(&f,DREADZONE_POSITION_RADIUS);f.views[1].storage=(u8 *)(UINTPTR_MAX-2);refusal(&f,-1);
    setup(&f,DREADZONE_POSITION_RADIUS);f.views[1].storage=f.body+1;refusal(&f,-1);
    setup(&f,DREADZONE_POSITION_RADIUS);f.views[1].guest_base=f.views[0].guest_base+1;refusal(&f,-1);
    setup(&f,DREADZONE_POSITION_RADIUS);f.context.views=(const DreadzoneActorInputMemory *)(UINTPTR_MAX-2);refusal(&f,-1);
    setup(&f,DREADZONE_POSITION_RADIUS);f.context.view_count=65;refusal(&f,-1);
    setup(&f,DREADZONE_POSITION_PROJECT);
    CHECK(dreadzone_actor_position_helper(&f.context,&f.request,(f32 *)(f.record+0x800))==-1);
    setup(&f,DREADZONE_POSITION_PROJECT);
    memcpy(f.record+0x1000,&f.context,sizeof(f.context));memcpy(expected,f.record,sizeof(expected));
    CHECK(dreadzone_actor_position_helper((const DreadzoneActorInputContext *)(f.record+0x1000),&f.request,&f.returned)==-1);
    CHECK(!memcmp(expected,f.record,sizeof(expected)));
    setup(&f,DREADZONE_POSITION_PROJECT);
    memcpy(f.record+0x1000,&f.request,sizeof(f.request));memcpy(expected,f.record,sizeof(expected));
    CHECK(dreadzone_actor_position_helper(&f.context,(const DreadzonePositionHelperRequest *)(f.record+0x1000),&f.returned)==-1);
    CHECK(!memcmp(expected,f.record,sizeof(expected)));
    setup(&f,DREADZONE_POSITION_OFFSET);f.request.scalar[0]=0x1p-41f;refusal(&f,-4);
    setup(&f,DREADZONE_POSITION_SCALAR);f.request.scalar[1]=-.125f;refusal(&f,-4);
    setup(&f,DREADZONE_POSITION_VECTOR);memcpy(f.record+0x820,f.record+0x800,16);
    word(f.record+0x82C,0xCAFEBABE);f.request.scalar[0]=0.f;f.request.scalar[1]=0.f;
    CHECK(execute(&f)==1);CHECK(bits(f.record+0x80C)==0xCAFEBABE);CHECK(f.returned==0.f);
    printf("%u checks, %u failures, %.6f seconds\n",checks,failures,(double)(clock()-started)/CLOCKS_PER_SEC);
    return failures?1:0;
}
