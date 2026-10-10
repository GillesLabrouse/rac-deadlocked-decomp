#include "dreadzone_actor_speed_target.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static u8 record[0x31F0+2],before[0x31F0+2],input[0x5C0],body[0xF0],flag[4],globals_[12],coefficient[4];
static u8 modes[2],predicate_state[4],bridge_mode[4],one_shot[4];
static DreadzoneActorInputMemory views[9];static DreadzoneActorInputContext ctx;
static unsigned checks,failures;
static void check(int good,const char *text) {++checks;if(!good && failures++<12) printf("FAIL: %s\n",text);}
static void put(u32 offset,u32 value) {dreadzone_actor_input_store32(ctx.record+offset,value);}
static void fput(u8 *p,u32 offset,f32 value) {dreadzone_direction_store_float(p+offset,value);}
static f32 get(u32 offset) {return dreadzone_direction_float(ctx.record+offset);}
static void reset(void) {
    memset(record,0,sizeof(record));record[0]=0xAC;record[sizeof(record)-1]=0x71;
    memset(input,0,sizeof(input));memset(body,0,sizeof(body));memset(flag,0,4);
    memset(globals_,0,12);
    views[0]=(DreadzoneActorInputMemory){input,0x1EE600,sizeof(input),0};
    views[1]=(DreadzoneActorInputMemory){body,0x230000,sizeof(body),0};
    views[2]=(DreadzoneActorInputMemory){flag,0x21E878,4,0};
    views[3]=(DreadzoneActorInputMemory){globals_,0x21DDEC,12,0};
    views[4]=(DreadzoneActorInputMemory){coefficient,0x3B01A0,4,0};
    ctx=(DreadzoneActorInputContext){record+1,0x368F60,0x31F0,views,5};
    put(0x2EF0,0x230000);put(0x2F00,0x1EE600);
    fput(body,0xC0,1);fput(body,0xD4,1);fput(body,0xE8,1);
    fput(ctx.record,0x2CF0,1);fput(ctx.record,0x2D04,1);fput(ctx.record,0x2D18,1);
    fput(ctx.record,0x25C4,1);fput(ctx.record,0x2E08,1);fput(ctx.record,0x2FC0,1);
    fput(globals_,0,.5f);fput(globals_,8,.005f);fput(coefficient,0,7.25f);
}
static int near(f32 a,f32 b) {return fabsf(a-b)<0.000002f;}
static void target_cases(void) {
    unsigned state,axis,gain;f32 expected;
    for(state=0;state<2;++state) for(axis=0;axis<2;++axis) for(gain=0;gain<5;++gain) {
        reset();put(0x25CC,state?114:2);fput(ctx.record,0x120,axis?.25f:0.f);
        fput(ctx.record,0x2E08,(f32)gain/4);
        memcpy(before,record,sizeof(record));
        check(dreadzone_actor_speed_target_0067CAF8(&ctx)==1,"actual direction and target helper complete");
        expected=axis?7.25f*.005f*((f32)gain/4):0.f;
        if(state) expected=fmaxf(expected*.8f,2.5f*.005f);
        check(near(get(0x1780),expected),"mapped coefficient/time/gain and state114 minimum produce target");
        memcpy(before+1+0x1780,ctx.record+0x1780,4);
        memcpy(before+1+0x17A0,ctx.record+0x17A0,4);
        memcpy(before+1+0x1790,ctx.record+0x1790,16);
        check(memcmp(before,record,sizeof(record))==0,"all unrelated actor bytes and guards stay exact");
    }
    /* 3B01A0 resolves into the actual direction destination: late reload matters. */
    reset();ctx.record_guest=0x3AEA10;ctx.view_count=4;
    fput(ctx.record,0x120,.5f);memcpy(before,record,sizeof(record));
    check(dreadzone_actor_speed_target_0067CAF8(&ctx)==1,"coefficient aliases the real actor1790 destination");
    check(get(0x1790)==0.f && get(0x1780)==0.f,"coefficient read observes the preceding direction store");
}
static void turn_cases(void) {
    unsigned branch,gain;f32 expected,maximum;
    for(branch=0;branch<3;++branch) for(gain=0;gain<5;++gain) {
        reset();fput(ctx.record,0x2E08,(f32)gain/4);
        fput(ctx.record,0x17A0,1.f);
        if(branch==1) ctx.record[0x1C8F]=1;
        if(branch==2) {put(0x2314,2);put(0x2318,2);ctx.record[0x265A]=1;}
        expected=gain==4?.011f:fmaxf(fminf(.01f*((f32)gain/4),.011f),.005f);
        maximum=gain==4?.0279252672f:fmaxf(fminf(.0191986215f*((f32)gain/4),.0279252672f),.0095993108f);
        if(branch==1) {expected=.0175f;maximum=.0584685278f;}
        if(branch==2) {
            expected=gain==4?.0085f:.0045f*((f32)gain/4);
            maximum=gain==4?.0436332321f:.0235619449f*((f32)gain/4);
        }
        memcpy(before,record,sizeof(record));
        check(dreadzone_actor_state_turn_0067D100(&ctx)==1,"actual state-derived rates reach real turn controller");
        check(near(get(0xB8),fminf(expected,maximum)),"first-frame angular displacement follows actual force/cap");
        check(near(get(0x17A4),get(0xB8)) && near(get(0x17A8),1.f-get(0xB8)),"real speed and remaining angular error");
        memcpy(before+1+0xB8,ctx.record+0xB8,4);
        memcpy(before+1+0x17A4,ctx.record+0x17A4,8);
        check(memcmp(before,record,sizeof(record))==0,"turn preserves unrelated bytes and guards");
    }
}
static void bridge_sequence(void) {
    f32 magnitude;
    reset();memset(modes,0,sizeof(modes));memset(predicate_state,0,4);
    memset(bridge_mode,0,4);memset(one_shot,0,4);
    views[5]=(DreadzoneActorInputMemory){modes,0x171DE0,2,0};
    views[6]=(DreadzoneActorInputMemory){predicate_state,0x21DDB4,4,0};
    views[7]=(DreadzoneActorInputMemory){bridge_mode,0x21E824,4,0};
    views[8]=(DreadzoneActorInputMemory){one_shot,0x21E8F4,4,1};
    ctx.view_count=9;
    fput(input,0x108,.375f);fput(input,0x10C,.5f);
    fput(ctx.record,0x128,.125f);
    check(dreadzone_actor_input_006A7D48(&ctx)==1,"genuine input bridge supplies both axes and 2E08");
    magnitude=sqrtf(.375f*.375f+.5f*.5f+.125f*.125f);
    check(near(get(0x2E08),magnitude),"third actor component participates in the actual magnitude producer");
    check(get(0x120)==.375f && get(0x124)==-.5f && get(0x128)==.125f,"raw axes/sign inversion and retained third component");
    check(dreadzone_actor_speed_target_0067CAF8(&ctx)==1,"real bridge state feeds the complete target helper");
    check(near(get(0x1780),7.25f*.005f*magnitude),"target uses actual bridge magnitude rather than a supplied target");
    check(dreadzone_actor_state_turn_0067D100(&ctx)==1,"actual bridge and target state feed the complete turn helper");
    check(get(0xB8)!=0.f && get(0x128)==.125f,"real turn changes facing while third input component survives");
    check(record[0]==0xAC && record[sizeof(record)-1]==0x71,"sequence retains actor boundary guards");
}
static void boundaries(void) {
    unsigned i;int turning,status;
    for(turning=0;turning<2;++turning) for(i=0;i<12;++i) {
        reset();fput(ctx.record,0x120,.5f);fput(ctx.record,0x17A0,1.f);
        switch(i) {
        case 0:ctx.record_guest++;break;
        case 1:ctx.record=(u8 *)(UINTPTR_MAX-8);break;
        case 2:ctx.views=(DreadzoneActorInputMemory *)(UINTPTR_MAX-8);break;
        case 3:views[0].storage=(u8 *)(UINTPTR_MAX-8);break;
        case 4:views[0].guest_base=0xFFFFFFF0U;break;
        case 5:ctx.views=(DreadzoneActorInputMemory *)(record+1);break;
        case 6:dreadzone_actor_input_store32(flag,1);break;
        case 7:ctx.record[0x2659]=1;break;
        case 8:fput(ctx.record,0x25C4,NAN);break;
        case 9:fput(ctx.record,0x2E08,NAN);break;
        case 10:views[3].guest_base=0x250000;break;
        case 11:fput(globals_,turning?0:8,NAN);break;
        }
        memcpy(before,record,sizeof(record));
        status=turning?dreadzone_actor_state_turn_0067D100(&ctx):dreadzone_actor_speed_target_0067CAF8(&ctx);
        check(status<0,"malformed selected state refuses");
        check(memcmp(before,record,sizeof(record))==0,"refusal after possible direction writes remains atomic");
    }
    reset();fput(ctx.record,0x120,.5f);ctx.view_count=4;memcpy(before,record,sizeof(record));
    check(dreadzone_actor_speed_target_0067CAF8(&ctx)==-2,"selected actual coefficient mapping is necessary");
    check(memcmp(before,record,sizeof(record))==0,"late missing coefficient discards direction effects");
    reset();fput(globals_,0,16.f);fput(ctx.record,0x17A0,1.f);memcpy(before,record,sizeof(record));
    check(dreadzone_actor_state_turn_0067D100(&ctx)==-4,"derived damping outside existing turn domain refuses");
    check(memcmp(before,record,sizeof(record))==0,"unsupported rate leaves entire actor image untouched");
    reset();fput(ctx.record,0x17A0,-0.000000059604644775390625f);
    memcpy(before,record,sizeof(record));
    check(dreadzone_actor_state_turn_0067D100(&ctx)==1,"actual tiny direction completes the turn controller");
    check(get(0xB8)==-0.000000059604644775390625f && get(0x17A4)==0.f && get(0x17A8)==0.f,
        "original snap preserves the tiny target instead of replacing it with zero");
    memcpy(before+1+0xB8,ctx.record+0xB8,4);
    check(memcmp(before,record,sizeof(record))==0,"small-angle snap preserves unrelated state");
    reset();fput(ctx.record,0x17A0,-0.0000000000001f);memcpy(before,record,sizeof(record));
    check(dreadzone_actor_state_turn_0067D100(&ctx)==-4,"angle below the proved floor refuses");
    check(memcmp(before,record,sizeof(record))==0,"below-floor refusal remains atomic");
}
int main(void) {
    target_cases();turn_cases();bridge_sequence();boundaries();
    printf("Speed target/state turn: %u checks, %u failures\n",checks,failures);
    return failures?1:0;
}
