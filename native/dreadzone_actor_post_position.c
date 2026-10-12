/* Selected L01 post-position RAM composition; approximate native math.
 * Original API/storage identities remain hypotheses. No matching credit. */
#include "dreadzone_actor_post_position.h"
#include "dreadzone_actor_orientation_ram.h"
#include "dreadzone_actor_visibility.h"
#include "dreadzone_actor_callback_enqueue.h"
#include "dreadzone_actor_input_bridge.h"
#include "dreadzone_actor_counter_step.h"
#include "dreadzone_actor_float_timer.h"
#include "dreadzone_actor_post_position_leaf.h"
#include "dreadzone_actor_velocity.h"
#include <float.h>
#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Post-position requires binary32 floats
#endif
typedef char post_position_float_size[sizeof(float)==4 ? 1 : -1];
#include <math.h>
#include <fenv.h>
#include <stdlib.h>
#include <string.h>

typedef struct PostRun {
    DreadzonePostPositionRequest request;
    DreadzonePostPositionInfo info;
    int status;
    DreadzoneActorInputContext context;
    DreadzoneActorInputMemory views[2];
} PostRun;
static void fail(PostRun *r,int status,uint32_t pc) {
    (void)pc;if(r->status>0){r->status=status;}
}
static uint8_t *memory(PostRun *r,uint32_t address,uint32_t count,uint32_t alignment) {
    uint32_t n=r->request.storage_bytes,base=r->request.guest_base;
    if((address&(alignment-1u)) || address<base || count>n || address-base>n-count) {
        fail(r,-1,0);return NULL;
    }
    return r->request.storage+address-base;
}
static uint32_t load(PostRun *r,uint32_t address,uint32_t n) {
    uint32_t raw=0;uint8_t *p=memory(r,address,n,n);if(p)memcpy(&raw,p,n);return raw;
}
static uint32_t word(PostRun *r,uint32_t address){return load(r,address,4);}
static uint32_t byte(PostRun *r,uint32_t address){return load(r,address,1);}
static int16_t half(PostRun *r,uint32_t address) {
    uint32_t raw=load(r,address,2);int16_t value;memcpy(&value,&raw,2);return value;
}
static int32_t integer(PostRun *r,uint32_t address) {
    uint32_t raw=word(r,address);int32_t value;memcpy(&value,&raw,4);return value;
}
static void store(PostRun *r,uint32_t address,const void *value,uint32_t n,uint32_t alignment) {
    uint8_t *p=memory(r,address,n,alignment);if(p)memcpy(p,value,n);
}
static void store_word(PostRun *r,uint32_t address,uint32_t value){store(r,address,&value,4,4);}
static void store_half(PostRun *r,uint32_t address,uint16_t value){store(r,address,&value,2,2);}
static float checked(PostRun *r,float value) {
    uint32_t raw,m;memcpy(&raw,&value,4);m=raw&0x7FFFFFFFu;
    if(m && (m<0x00800000u || m>=0x7F800000u))fail(r,-2,0);
    return value;
}
static float number(PostRun *r,uint32_t address) {
    uint32_t raw=word(r,address);float value;memcpy(&value,&raw,4);return checked(r,value);
}
static float add(PostRun *r,float a,float b){volatile float value=a+b;return checked(r,value);}
static float mul(PostRun *r,float a,float b){volatile float value=a*b;return checked(r,value);}
static void store_float(PostRun *r,uint32_t address,float value){checked(r,value);store(r,address,&value,4,4);}
static int short_timer(PostRun *r,uint32_t address) {
    int16_t old=half(r,address),value=old;int result=func_L01_0055D180(&value);
    if(old)store_half(r,address,(uint16_t)value);
    ++r->info.half_timers;return result;
}
static int word_timer(PostRun *r,uint32_t address) {
    int32_t old=integer(r,address),value=old;int result=dreadzone_actor_counter_step_0055D148(&value);
    if(old)store_word(r,address,(uint32_t)value);
    ++r->info.word_timers;return result;
}
static void context(PostRun *r) {
    uint32_t start=r->request.record_guest,end=start+0x31F0u;
    uint32_t base=r->request.guest_base,limit=base+r->request.storage_bytes,n=0;
    r->context.record=r->request.storage+start-base;
    r->context.record_guest=start;r->context.record_bytes=0x31F0u;
    if(start>base)r->views[n++]=(DreadzoneActorInputMemory){r->request.storage,base,start-base,1};
    if(end<limit)r->views[n++]=(DreadzoneActorInputMemory){r->request.storage+end-base,end,limit-end,1};
    r->context.views=r->views;r->context.view_count=n;
}
static void orient(PostRun *r) {
    DreadzoneActorOrientationRAMRequest q={r->request.storage,r->request.guest_base,
        r->request.storage_bytes,r->request.record_guest,r->request.gp_guest};
    DreadzoneActorOrientationRAMInfo info;
    if(r->status<0)return;
    if(dreadzone_actor_orientation_ram_00683310(&q,&info)<0){fail(r,-2,0x684F20u);return;}
    ++r->info.orientation;
}
static void visibility(PostRun *r,uint32_t actor,uint32_t mode) {
    DreadzoneActorVisibilityRequest q={r->request.storage,r->request.guest_base,
        r->request.storage_bytes,actor,mode};
    DreadzoneActorVisibilityInfo info;
    if(r->status<0)return;
    if(dreadzone_actor_visibility_005A8630(&q,&info)<0){fail(r,-2,0x684FB0u);return;}
    ++r->info.visibility;
}
static void enqueue(PostRun *r) {
    DreadzoneCallbackEnqueueRequest q={r->request.storage,r->request.guest_base,
        r->request.storage_bytes,r->request.record_guest};
    DreadzoneCallbackEnqueueInfo info;
    if(r->status<0)return;
    if(dreadzone_actor_callback_enqueue_0066ABD8(&q,&info)<0){fail(r,-2,0x6855BCu);return;}
    ++r->info.queue_calls;
}
static void leaf(PostRun *r) {
    DreadzonePostPositionLeafRequest q={r->request.storage,r->request.guest_base,
        r->request.storage_bytes,r->request.record_guest,r->request.gp_guest};
    DreadzonePostPositionLeafInfo info;
    if(r->status<0)return;
    if(dreadzone_actor_post_position_leaf_00686420(&q,&info)<0){fail(r,-2,0x6862B0u);return;}
    ++r->info.leaf_calls;
}
static void float_timer(PostRun *r,uint32_t address) {
    DreadzoneActorFloatTimerRequest q={r->request.storage,r->request.guest_base,
        r->request.storage_bytes,address,r->request.gp_guest};
    DreadzoneActorFloatTimerInfo info;
    if(r->status<0)return;
    if(dreadzone_actor_float_timer_0055D240(&q,&info)<0){fail(r,-2,0x685290u);return;}
    ++r->info.float_timers;
}
/* The original caller stores this projection three times. Every loaded lane
 * and zero multiply remains evaluated, including unconsumed W products. */
static void projection(PostRun *r,uint32_t b) {
    float rows[4][4],position[3],a,t,v;uint32_t i,j,initial[4]={0,0,0x3F333333u,0};
    for(i=0;i<4;++i)for(j=0;j<4;++j)rows[i][j]=number(r,b+0x20u+i*16u+j*4u);
    for(j=0;j<3;++j)position[j]=number(r,b+0xA0u+j*4u);
    store(r,b+0xF0u,initial,16,16);
    for(j=0;j<4;++j) {
        a=mul(r,rows[0][j],0.f);t=mul(r,rows[1][j],0.f);a=add(r,a,t);
        t=mul(r,rows[2][j],.7f);a=add(r,a,t);t=mul(r,rows[3][j],0.f);v=add(r,a,t);
        store_float(r,b+0xF0u+j*4u,v);
    }
    for(j=0;j<3;++j)store_float(r,b+0xF0u+j*4u,add(r,number(r,b+0xF0u+j*4u),position[j]));
}
static void increment(PostRun *r,uint32_t address,uint32_t b) {
    int32_t old=integer(r,address);float value,rounded;
    if(old< -16777216 || old>16777216){fail(r,-2,0);return;}
    value=(float)old;rounded=add(r,value,number(r,b+0x25C4u));
    /* Selected reference operands are integral. Do not infer an uncaptured
     * FCSR rounding mode from the interpreter's truncation implementation. */
    if(rounded< -16777216.f || rounded>16777216.f || truncf(rounded)!=rounded) {
        fail(r,-2,0);return;
    }
    store_word(r,address,(uint32_t)(int32_t)rounded);
}
static void prefix(PostRun *r) {
    uint32_t b=r->request.record_guest,gp=r->request.gp_guest,mode,actor,owner,value;
    float radius;int16_t flags;
    if(gp<0x715Bu){fail(r,-1,0x684F1Cu);return;}
    mode=word(r,gp-0x715Bu); /* Always-executed branch delay load. */
    if(!byte(r,b+0x2672u)){orient(r);mode=word(r,0x21E824u);}
    word(r,gp-0x715Bu); /* 684F38, including the mode-one branch. */
    if(mode!=1u) {
        actor=word(r,b+0x2EF0u);owner=word(r,actor+0x24u);
        radius=mul(r,number(r,owner+0x24u),1.2f);store_float(r,actor+0x70u,radius);
        flags=half(r,b+0x418u);value=word(r,b+0x2EF0u);
        store_half(r,value+0x74u,0);
        if(flags) {
            value=word(r,b+0x2EF0u);store_half(r,value+0x74u,(uint16_t)(load(r,value+0x74u,2)|0xFFEu));
        } else {
            value=word(r,b+0x2EF0u);
            if(gp<0x52CFu){fail(r,-1,0x684F88u);return;}
            store_half(r,value+0x74u,(uint16_t)(load(r,value+0x74u,2)|load(r,gp-0x52CFu,2)));
        }
        if(!word(r,b+0x2F10u))store_word(r,b+0x2F14u,1);
        visibility(r,word(r,b+0x2EF0u),word(r,b+0x2F14u));
        if(byte(r,0x171B5Eu)){fail(r,-2,0x684FDCu);return;}
        value=byte(r,0x21DE3Cu);
        if(value) {
            if(word(r,0x1DFC60u)!=16u){fail(r,-2,0x685014u);return;}
            value=byte(r,0x21DE3Cu);word(r,gp-0x715Bu);
        }
        if(!value) {
            word(r,gp-0x715Bu);
            if(word(r,0x1DFC60u)==16u){fail(r,-2,0x685048u);return;}
            word(r,gp-0x715Bu);
        }
        mode=word(r,0x21E824u);
    } else mode=word(r,gp-0x715Bu);
    value=word(r,gp-0x7157u); /* 685060 delay executes even mode!=0. */
    if(mode==0u && value==1u && byte(r,b+0x2FB8u)){fail(r,-2,0x685098u);return;}
    increment(r,b+0x370u,b);increment(r,b+0x374u,b);
    increment(r,b+0x378u,b);increment(r,b+0x37Cu,b);
    if(word(r,b+0x2EE0u)){fail(r,-2,0x6851B8u);return;}
}
static void timer_group(PostRun *r,uint32_t b,const uint16_t *offsets,uint32_t n) {
    uint32_t i;for(i=0;i<n && r->status>0;++i)short_timer(r,b+offsets[i]);
}
static void timers(PostRun *r) {
    static const uint16_t first[]={0x3BA,0x3B8,0x39C,0x38A,0x3B0,0x3C4,0x3C6};
    static const uint16_t second[]={0x3A8,0x388,0x3C2,0x3A0};
    static const uint16_t third[]={0x3BC,0x3CA,0x3D2,0x3A6,0x3CC,0x3D0,0x3AA,0x3C8,0x3A2,
        0x3D4,0x3BE,0x3D6,0x3D8,0x3DA,0x3AE,0x39E,0x3E0,0x3AC,0x3E2};
    static const uint16_t fourth[]={0x3FE,0x402,0x404,0x406,0x412};
    static const uint16_t fifth[]={0x418,0x2E54,0x41C};
    uint32_t b=r->request.record_guest,gp=r->request.gp_guest,mode,state,value;
    int16_t prior;
    timer_group(r,b,first,7);word_timer(r,b+0x398u);timer_group(r,b,second,4);
    if(number(r,b+0x3ECu)<.015f)store_word(r,b+0x3ECu,0);
    float_timer(r,b+0x3ECu);
    timer_group(r,b,third,19);word_timer(r,b+0x3F4u);word_timer(r,b+0x3E8u);
    timer_group(r,b,fourth,5);word_timer(r,b+0x390u);word_timer(r,b+0x394u);
    timer_group(r,b,fifth,3);prior=half(r,b+0x41Eu);
    if(prior) {
        if(word(r,gp-0x715Bu)==1u && prior==30){fail(r,-2,0x6853B0u);return;}
        if(short_timer(r,b+0x41Eu)!=0){fail(r,-2,0x6853D0u);return;}
    }
    short_timer(r,b+0x420u);short_timer(r,b+0x422u);
    if(word(r,0x21E824u)==1u && byte(r,b+0x265Au) && half(r,b+0x422u))store_half(r,b+0x422u,0);
    if(byte(r,b+0x2FC4u) && word(r,b+0x25D4u)!=15u){fail(r,-2,0x685414u);return;}
    state=word(r,b+0x25CCu);mode=word(r,gp-0x715Bu);
    if(state==102u || state==100u){store_half(r,b+0x404u,25);mode=word(r,0x21E824u);}
    if(mode==0u && word(r,0x21E828u)==0u && !word(r,b+0x390u)) {
        store_float(r,b+0x2FC0u,1.f);
        if(byte(r,0x21DE3Cu))store_float(r,b+0x2FC0u,1.2f);
    }
    if(half(r,b+0x412u)){fail(r,-2,0x6854FCu);return;}
    if(short_timer(r,b+0x408u)!=0)store_word(r,b+0x2FB4u,0);
    if(short_timer(r,b+0x40Au)!=0)store_float(r,b+0x2FB0u,1.f);
    if(word(r,b+0x25D4u)!=20u && word(r,b+0x25CCu)!=122u) {
        increment(r,b+0x3F0u,b);value=word(r,gp-0x7BCBu);
    } else {store_word(r,b+0x3F0u,0);value=word(r,0x21DDB4u);}
    if(value==2u){fail(r,-2,0x685588u);return;}
    value=byte(r,b+0x2FC9u);mode=word(r,gp-0x7BCBu);
    if(value && mode!=2u){fail(r,-2,0x6855B4u);return;}
    enqueue(r);
}
static void input_and_contacts(PostRun *r) {
    uint32_t b=r->request.record_guest,gp=r->request.gp_guest,input,pointer,mode;
    int32_t mask,index;
    if(word(r,0x21E824u)==1u){fail(r,-2,0x6855D8u);return;}
    if(!byte(r,b+0x265Au))store_word(r,b+0x38Cu,0);else increment(r,b+0x38Cu,b);
    index=integer(r,b+0x2F10u);
    if(r->status<0)return;
    if(dreadzone_actor_input_005BE808(&r->context,4,index,&mask)<0){fail(r,-2,0x6858BCu);return;}
    ++r->info.input_calls;
    input=word(r,b+0x2F00u);
    if(!(word(r,input+0x1A0u)&(uint32_t)mask))store_half(r,b+0x3CEu,0);
    input=word(r,b+0x2F00u);number(r,b+0x25C4u);
    if(word(r,input+0x1D4u)) {store_word(r,b+0x380u,0);increment(r,b+0x384u,b);}
    else {store_word(r,b+0x384u,0);increment(r,b+0x380u,b);}
    pointer=word(r,b+0x20Cu);mode=word(r,gp-0x715Bu);
    if(pointer) {
        if(!word(r,b+0x3E8u) || word(r,b+0x2F4Cu)!=pointer){fail(r,-2,0x685944u);return;}
        /* 685940 writes V0=1, while V1 still holds the GP-loaded mode. */
    }
    if(mode==1u && byte(r,b+0x266Du) && byte(r,b+0x2659u)==1u){fail(r,-2,0x685F9Cu);return;}
}
static void quad_copy(PostRun *r,uint32_t output,uint32_t input) {
    uint32_t raw[4];uint8_t *p=memory(r,input,16,16);if(!p)return;
    memcpy(raw,p,16);store(r,output,raw,16,16);
}
static void post_timers(PostRun *r) {
    uint32_t b=r->request.record_guest,gp=r->request.gp_guest,state,i;
    float threshold;
    if(byte(r,b+0x2659u) && !word(r,b+0x25D4u)) {
        state=word(r,b+0x2F0u);
        if(!state) {word(r,gp-0x7B8Bu); /* Delay load is unconsumed. */}
        else {
            threshold=mul(r,mul(r,number(r,gp-0x7B8Bu),.1f),number(r,b+0x25C4u));
            if(number(r,b+0x1B0u)<threshold && number(r,b+0x1C4u)<threshold)quad_copy(r,b+0xA0u,b+0x110u);
        }
    }
    if(half(r,b+0x5F4u)) {
        if(word(r,b+0x25CCu)!=16u)store_word(r,b+0x3B4u,0);else increment(r,b+0x3B4u,b);
    } else {
        if(word(r,b+0x1AE0u)==1u){fail(r,-2,0x686034u);return;}
        increment(r,b+0x3B4u,b);
    }
    for(i=0;i<4u;++i)word_timer(r,b+0x2D84u+i*16u);
    state=word(r,b+0x25CCu);
    if(state!=49u && state!=24u && state!=25u && state!=26u && half(r,b+0x2FEu)) {
        fail(r,-2,0x6860F4u);return;
    }
    store_half(r,b+0x3C0u,0);
    /* With the reached 3C0 clear the state-dependent 67A4F8 branch is false;
     * still read its actual outer state gate before the nested leaf call. */
    if(word(r,b+0x25CCu)!=120u){state=word(r,b+0x25D4u);(void)state;}
    leaf(r);if(r->status<0)return;projection(r,b);
    if(!(word(r,0x21DDB8u)&3u)) {store_float(r,b+0x2E24u,10.625f);store_float(r,b+0x2E28u,5.625f);}
    if(half(r,b+0x41Cu)){store_float(r,b+0x2E24u,127.5f);store_float(r,b+0x2E28u,67.5f);}
    if(number(r,b+0x2CCu)<4.f) {
        quad_copy(r,b+0xE0u,b+0x280u);
        if(r->status<0)return;
        if(dreadzone_actor_height_offset_0068A178(&r->context,b+0xE0u,b+0xE0u,-.6f)<0) {
            fail(r,-2,0x68639Cu);return;
        }
        ++r->info.height_calls;
    } else quad_copy(r,b+0xE0u,b+0xF0u);
    if(number(r,b+0xA8u)<1.f){fail(r,-2,0x6863E4u);return;}
}
static int numeric_host(void) {
    volatile float minimum=0x1p-126f,half_minimum=minimum*.5f;
    float probe=half_minimum;uint32_t raw;memcpy(&raw,&probe,4);
    return fegetround()==FE_TONEAREST && raw==0x00400000u;
}
static int span(const void *p,size_t n){return p && (uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *a,size_t n,const void *b,size_t m) {
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x>=y?x-y<m:y-x<n;
}
int dreadzone_actor_post_position_00684EF0(const DreadzonePostPositionRequest *request,DreadzonePostPositionInfo *info) {
    PostRun r;uint8_t *shadow;uint32_t n,b,base;
    if(!span(request,sizeof(*request)) || !span(info,sizeof(*info)) ||
       overlap(request,sizeof(*request),info,sizeof(*info)))return -1;
    memset(&r,0,sizeof(r));memcpy(&r.request,request,sizeof(r.request));
    n=r.request.storage_bytes;b=r.request.record_guest;base=r.request.guest_base;
    if(!n || n>16u*1024u*1024u || !span(r.request.storage,n) || base>=0x80000000u ||
       n>0x80000000u-base || (base<0x70004000u && base+n>0x70000000u) ||
       overlap(r.request.storage,n,info,sizeof(*info)) || (b&15u) || b<base ||
       b-base>n || 0x31F0u>n-(b-base) || r.request.gp_guest<0x7BCBu)return -1;
    if(!numeric_host())return -2;
    shadow=(uint8_t *)malloc(n);if(!shadow)return -4;
    memcpy(shadow,r.request.storage,n);r.request.storage=shadow;r.status=1;context(&r);
    prefix(&r);
    if(r.status>0)timers(&r);
    if(r.status>0)input_and_contacts(&r);
    if(r.status>0)post_timers(&r);
    if(r.status>0){memcpy(request->storage,shadow,n);memcpy(info,&r.info,sizeof(r.info));}
    free(shadow);return r.status;
}
