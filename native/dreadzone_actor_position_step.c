#include "dreadzone_actor_position_step.h"
#include "dreadzone_actor_capsule_resolve.h"
#include "dreadzone_actor_ground_step.h"
#include "dreadzone_actor_position_helpers.h"
#include "dreadzone_actor_velocity.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Position step requires IEEE binary32 host floats
#endif
typedef char position_step_float_size[sizeof(f32)==4?1:-1];
typedef struct PositionWork {
    DreadzonePositionStepRequest request;
    DreadzonePositionStepInfo info;
    u8 *copy;
    int failed;
    int track_incoming;
    u32 high64[2];
    u32 approximate_q_bits;
    DreadzoneProbeRegisters ground_registers;
} PositionWork;
typedef struct PositionVector { f32 lane[4]; } PositionVector;

static int host_span(const void *p,uintptr_t bytes) {
    return p && (uintptr_t)p<=UINTPTR_MAX-bytes;
}
static int overlaps(const void *p,u32 bytes,const void *q,u32 other) {
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;
    return a>=b?a-b<other:b-a<bytes;
}
static u8 *memory(PositionWork *w,u32 address,u32 bytes,u32 alignment) {
    DreadzonePositionStepRequest *r=&w->request;
    if(w->failed)return NULL;
    if((address&(alignment-1U)) || address<r->guest_base ||
        bytes>r->storage_bytes || address-r->guest_base>r->storage_bytes-bytes) {
        w->failed=DREADZONE_POSITION_STEP_INVALID;return NULL;
    }
    return w->copy+(address-r->guest_base);
}
static u32 word(PositionWork *w,u32 address) {
    u32 value=0;u8 *p=memory(w,address,4,4);if(p)memcpy(&value,p,4);return value;
}
static s32 signed_word(PositionWork *w,u32 address) {
    u32 raw=word(w,address);s32 value;memcpy(&value,&raw,4);return value;
}
static u8 byte(PositionWork *w,u32 address) {
    u8 *p=memory(w,address,1,1);return p?*p:0;
}
static void store_word(PositionWork *w,u32 address,u32 value) {
    u8 *p=memory(w,address,4,4);if(p)memcpy(p,&value,4);
}
static u32 raw(f32 value) {u32 bits;memcpy(&bits,&value,4);return bits;}
static int normal(f32 value) {
    u32 magnitude=raw(value)&0x7FFFFFFFU;
    return !magnitude || (magnitude>=0x00800000U && magnitude<0x7F800000U);
}
static f32 checked(PositionWork *w,f32 value) {
    if(!normal(value)&&!w->failed)w->failed=DREADZONE_POSITION_STEP_UNSUPPORTED_FLOAT;
    return value;
}
static f32 load(PositionWork *w,u32 address) {
    u32 bits=word(w,address);f32 value;memcpy(&value,&bits,4);return checked(w,value);
}
static f32 add(PositionWork *w,f32 a,f32 b) {
    volatile f32 result;
    checked(w,a);checked(w,b);result=a+b;return checked(w,result);
}
static f32 mul(PositionWork *w,f32 a,f32 b) {
    volatile f32 result;
    checked(w,a);checked(w,b);result=a*b;
    if(a!=0.f&&b!=0.f&&result==0.f&&!w->failed)w->failed=DREADZONE_POSITION_STEP_UNSUPPORTED_FLOAT;
    return checked(w,result);
}
/* Actual C60 writes VF28.W as well as its returned VF15.xy. Its W lane
 * folds against VF0.W=1, unlike the ordinary XY angle pair. This is the
 * intermediate c5/c7/c9 tail at micro CFC, not sin(angle). Separate host
 * binary32 operations and signed subnormal flushing remain approximate. */
static f32 micro_round(f32 value) {
    u32 bits=raw(value);
    if((bits&0x7FFFFFFFU)!=0 && (bits&0x7FFFFFFFU)<0x00800000U) {
        bits&=0x80000000U;memcpy(&value,&bits,4);
    }
    return value;
}
static f32 micro_add(f32 a,f32 b) {volatile f32 v=a+b;return micro_round(v);}
static f32 micro_mul(f32 a,f32 b) {volatile f32 v=a*b;return micro_round(v);}
static f32 micro_constant(u32 bits) {f32 value;memcpy(&value,&bits,4);return value;}
static u32 heading_w(PositionWork *w,f32 angle) {
    f32 pi=micro_constant(0x40490FDBU),upper,lower,a,a2,a4,acc,product;
    if(!normal(angle)||fabsf(angle)>pi||(angle!=0.f&&fabsf(angle)<0x1p-40f)) {
        w->failed=DREADZONE_POSITION_STEP_UNSUPPORTED_FLOAT;return 0;
    }
    upper=micro_add(micro_add(1.f,pi),-angle);
    lower=micro_add(micro_add(1.f,-pi),-angle);
    a=angle<upper?angle:upper;a=a>lower?a:lower;
    a2=micro_mul(a,a);a4=micro_mul(a2,a2);
    acc=micro_mul(a,micro_constant(0x3C08873EU));
    product=micro_mul(micro_mul(a,micro_constant(0xB94FB21FU)),a2);
    acc=micro_add(acc,product);
    product=micro_mul(micro_mul(a,micro_constant(0x362E9C14U)),a4);
    return raw(micro_add(acc,product));
}
static PositionVector vector(PositionWork *w,u32 address) {
    PositionVector result={{0.f,0.f,0.f,0.f}};u8 *p=memory(w,address,16,16);
    if(p)memcpy(&result,p,16);return result;
}
static void store_vector(PositionWork *w,u32 address,const PositionVector *value) {
    u8 *p=memory(w,address,16,16);if(p)memcpy(p,value,16);
}
static f32 squares(PositionWork *w,const PositionVector *v,PositionVector *out) {
    u32 i;for(i=0;i<4;++i)out->lane[i]=mul(w,v->lane[i],v->lane[i]);
    return add(w,out->lane[0],out->lane[1]);
}
static f32 reciprocal_sqrt(PositionWork *w,f32 numerator,f32 denominator) {
    volatile f32 root,result;
    checked(w,numerator);checked(w,denominator);
    if(denominator<0.f)w->approximate_q_bits|=0x10U;
    root=sqrtf(fabsf(denominator));checked(w,root);
    if(root==0.f) {
        u32 sign=(raw(numerator)^raw(root))&0x80000000U;
        u32 bits=0x7F7FFFFFU|sign;f32 saturated;
        w->approximate_q_bits|=numerator==0.f?0x10U:0x20U;
        memcpy(&saturated,&bits,4);return saturated;
    }
    result=numerator/root;return checked(w,result);
}

static s16 half(PositionWork *w,u32 address) {
    s16 value=0;u8 *p=memory(w,address,2,2);if(p)memcpy(&value,p,2);return value;
}
static f32 sub(PositionWork *w,f32 a,f32 b) {
    volatile f32 v;checked(w,a);checked(w,b);v=a-b;return checked(w,v);
}
static f32 division(PositionWork *w,f32 a,f32 b) {
    volatile f32 v;checked(w,a);checked(w,b);
    if(b==0.f){w->failed=DREADZONE_POSITION_STEP_UNSUPPORTED_FLOAT;return 0.f;}
    v=a/b;return checked(w,v);
}
static f32 number(PositionWork *w,u32 address,f32 maximum,int time) {
    f32 value=load(w,address),floor=time?0x1p-20f:0x1p-40f;
    if(fabsf(value)>maximum || (value!=0.f && fabsf(value)<floor) || (time&&value<0.f))
        w->failed=DREADZONE_POSITION_STEP_UNSUPPORTED_FLOAT;
    return value;
}
static f32 square3(PositionWork *w,const PositionVector *v,PositionVector *square) {
    f32 xy=squares(w,v,square);return add(w,xy,mul(w,1.f,square->lane[2]));
}
static f32 root(PositionWork *w,f32 value) {
    volatile f32 r;checked(w,value);r=sqrtf(fabsf(value));return checked(w,r);
}
static PositionVector difference(PositionWork *w,PositionVector a,PositionVector b) {
    u32 i;for(i=0;i<3;++i)a.lane[i]=sub(w,a.lane[i],b.lane[i]);return a;
}
static PositionVector sum(PositionWork *w,PositionVector a,PositionVector b) {
    u32 i;for(i=0;i<3;++i)a.lane[i]=add(w,a.lane[i],b.lane[i]);return a;
}
static PositionVector scale(PositionWork *w,PositionVector a,f32 q,u32 count) {
    u32 i;for(i=0;i<count;++i)a.lane[i]=mul(w,a.lane[i],q);return a;
}
static PositionVector normalize(PositionWork *w,PositionVector a,f32 amount,u32 count) {
    PositionVector square;f32 norm=count==2?squares(w,&a,&square):square3(w,&a,&square);
    f32 q=reciprocal_sqrt(w,amount,norm);u32 i;
    if(raw(norm)||raw(square.lane[1]))return scale(w,a,q,count);
    for(i=0;i<count;++i)a.lane[i]=0.f;
    return a;
}
static PositionVector project(PositionWork *w,PositionVector velocity,PositionVector direction,u32 target) {
    PositionVector products,square;f32 dot,norm,q;u32 i;
    for(i=0;i<4;++i)products.lane[i]=mul(w,direction.lane[i],velocity.lane[i]);
    dot=add(w,add(w,products.lane[0],products.lane[1]),mul(w,1.f,products.lane[2]));
    norm=square3(w,&velocity,&square);
    dot=dreadzone_velocity_max(dot,0.f);
    store_vector(w,target,&direction); /* Real intermediate store before Q. */
    q=reciprocal_sqrt(w,dot,norm);
    /* Original norm destination is ones, so its low64 retains Y=1. */
    return scale(w,velocity,q,3);
}
static int error_status(PositionWork *w,int result) {
    if(result<0&&!w->failed)w->failed=DREADZONE_POSITION_STEP_UNSUPPORTED_PATH;
    return result>=0;
}
static void context(PositionWork *w,DreadzoneActorInputContext *c,DreadzoneActorInputMemory views[2]) {
    u32 record=w->request.record_guest,base=w->request.guest_base,n=0;
    c->record=memory(w,record,0x31F0,16);c->record_guest=record;c->record_bytes=0x31F0;
    if(record>base)views[n++]=(DreadzoneActorInputMemory){w->copy,base,record-base,1};
    if(record-base+0x31F0<w->request.storage_bytes)views[n++]=(DreadzoneActorInputMemory){
        w->copy+record-base+0x31F0,record+0x31F0,w->request.storage_bytes-(record-base+0x31F0),1};
    c->views=views;c->view_count=n;
}
static int helper(PositionWork *w,u32 operation,u32 value,u32 target,f32 *returned) {
    DreadzoneActorInputContext c;DreadzoneActorInputMemory views[2];
    DreadzonePositionHelperRequest q={0};int result;
    context(w,&c,views);if(w->failed)return 0;
    q.operation=operation;q.value=value;q.target=target;
    result=dreadzone_actor_position_helper(&c,&q,returned);
    if(operation==DREADZONE_POSITION_SETTLE&&result>0)++w->info.settle_calls;
    return error_status(w,result);
}
static int capsule(PositionWork *w) {
    DreadzoneCapsuleResolveRequest q={0};DreadzoneCapsuleResolveInfo info;int result;
    q.storage=w->copy;q.guest_base=w->request.guest_base;q.storage_bytes=w->request.storage_bytes;
    q.record_guest=w->request.record_guest;q.operation=DREADZONE_CAPSULE_RESOLVE_F518;
    q.actor_pool_count=w->request.actor_pool_count;
    memcpy(q.incoming_vf25_29_w,w->info.outgoing_vf25_29_w,20);
    if(w->failed)return 0;
    if(w->track_incoming) {
        DreadzoneCapsuleResolveRegisters state;
        result=dreadzone_actor_capsule_registers(&q,w->high64,&state,&info);
        if(result>0)memcpy(w->high64,state.high64,8);
    }else result=dreadzone_actor_capsule_resolve(&q,&info);
    if(!error_status(w,result))return 0;
    ++w->info.capsule_resolve_calls;w->info.capsule_calls+=info.capsule_calls;
    w->info.static_calls+=info.static_calls;return 1;
}
static int ground(PositionWork *w) {
    DreadzoneGroundRequest q={0};DreadzoneGroundInfo info;int result;
    q.storage=w->copy;q.guest_base=w->request.guest_base;q.storage_bytes=w->request.storage_bytes;
    q.record_guest=w->request.record_guest;q.operation=DREADZONE_GROUND_UPDATE;
    q.actor_pool_count=w->request.actor_pool_count;q.grid_block_count=w->request.grid_block_count;
    memcpy(q.incoming_vf25_29_w,w->info.outgoing_vf25_29_w,20);
    memcpy(q.incoming_side_output_w,w->request.incoming_ground_side_w,8);
    if(w->failed)return 0;
    /* The preceding capsule state is symbolic/unknown in this adapter.
     * Poison bookkeeping is never fed into C60: only a definite reached
     * ground producer or the original settling load may establish its W. */
    if(w->track_incoming) {
        result=dreadzone_actor_ground_registers(&q,w->high64,&w->ground_registers,&info);
        if(result>0)memcpy(w->high64,w->ground_registers.high64,8);
    }else {
        const u32 unknown[2]={0xFFFFFFFFU,0x7FC0BEEFU};
        result=dreadzone_actor_ground_registers(&q,unknown,&w->ground_registers,&info);
    }
    if(!error_status(w,result))return 0;
    ++w->info.ground_calls;w->info.probe_calls+=info.probe_calls;
    w->info.ground_contacts+=info.ground_contacts;
    memcpy(w->info.outgoing_vf25_29_w,info.outgoing_vf25_29_w,20);return 1;
}
/* Complete genuine B258 s4==0 exits. FloatB8 is raw LWC1 transport even on
 * these exits. Active object lookup/transforms are a reached boundary. */
static int object_exit(PositionWork *w,u32 b) {
    s32 state=signed_word(w,b+0x25D4);u32 prior,flags;int active=0;
    (void)word(w,b+0xB8);
    if(state==0 || state==12)(void)word(w,b+0x25CC);
    state=signed_word(w,b+0x25D4);
    if(state==4)(void)byte(w,b+0x1D0C);
    if(half(w,b+0x2FE)==0) {
        u32 object=word(w,b+0x2EC);
        if(object) {
            prior=word(w,b+0x360);active=1;
            if(prior!=object)store_word(w,b+0x364,word(w,b+0x364)&~1U);
            store_word(w,b+0x360,word(w,b+0x2EC));
        } else {
            flags=word(w,b+0x364);store_word(w,b+0x360,0);store_word(w,b+0x364,flags&~3U);
        }
    } else {
        u32 object=0;
        if(signed_word(w,b+0x25CC)==27)object=word(w,b+0x1DD8);
        if(object){store_word(w,b+0x360,object);active=1;}
        else {
            if(state==3)object=word(w,b+0x1DD8);
            if(object){store_word(w,b+0x360,object);active=1;}
            else {
                prior=word(w,b+0x360);
                if(prior) {
                    (void)half(w,b+0x2FE);active=1;
                    if(state==4){(void)half(w,b+0x1D1C);(void)signed_word(w,b+0x370);}
                } else {
                    flags=word(w,b+0x364);store_word(w,b+0x360,0);store_word(w,b+0x364,flags&~3U);
                }
            }
        }
    }
    if(w->failed)return 0;
    if(active){w->failed=DREADZONE_POSITION_STEP_UNSUPPORTED_PATH;return 0;}
    ++w->info.object_exits;return 1;
}

static int body(PositionWork *w) {
    u32 b=w->request.record_guest,i;u8 *record=memory(w,b,0x31F0,16);
    PositionVector saved,savedSecond,position,velocity,second,square,observed,planarVelocity,xyVelocity;
    f32 maxxy,threshold,vertical,length,norm,value,headingW;int second_phase;u32 state,secondary;
    if(!record)return w->failed;
    if(byte(w,b+0x2659)!=0)return DREADZONE_POSITION_STEP_UNSUPPORTED_PATH;
    saved=vector(w,b+0xA0);
    for(i=0;i<3;++i)if(fabsf(checked(w,saved.lane[i]))>8192.f)w->failed=DREADZONE_POSITION_STEP_UNSUPPORTED_FLOAT;
    maxxy=sub(w,number(w,b+0x204,16.f,0),.02f);
    threshold=sub(w,number(w,b+0x240,16.f,0),.0004f);
    if(word(w,0x21E824)!=1 && !object_exit(w,b))return w->failed;
    state=word(w,b+0x25CC);
    velocity=vector(w,b+0x130);
    if(state==20||state==131) {
        VelocityVector input;memcpy(&input,&velocity,16);
        if(half(w,b+0x3A0)==0) {
            length=checked(w,func_L01_0068A920(record,&input));
            if(maxxy<length) {
                if(maxxy<0.f)return DREADZONE_POSITION_STEP_UNSUPPORTED_FLOAT;
                if(!error_status(w,dreadzone_velocity_0068AC58(record,(u8 *)&velocity,(u8 *)&velocity,maxxy)))return w->failed;
                store_vector(w,b+0x130,&velocity);
            }
        }
        memcpy(&input,&velocity,16);vertical=checked(w,func_L01_0068AB40(record,&input));
        if(vertical<0.f) {
            value=-number(w,b+0x2CC,16.f,0);
            if(vertical<value) {
                vertical=dreadzone_velocity_min(dreadzone_velocity_max(vertical,sub(w,value,.1f)),0.f);
                if(!error_status(w,dreadzone_velocity_0068ABA8(record,(u8 *)&velocity,(u8 *)&velocity,vertical)))return w->failed;
                store_vector(w,b+0x130,&velocity);
            }
        }
        if(word(w,b+0x25CC)==20) {
            norm=square3(w,&velocity,&square);length=root(w,norm);
            if(maxxy<length) {
                if(maxxy<0.f)return DREADZONE_POSITION_STEP_UNSUPPORTED_FLOAT;
                velocity=scale(w,velocity,division(w,maxxy,length),3);store_vector(w,b+0x130,&velocity);
            }
        }
    } else {
        secondary=word(w,b+0x25D4);
        if(secondary==15 && half(w,b+0x3A0)==0) {
            length=root(w,squares(w,&velocity,&square));value=sub(w,number(w,b+0x204,16.f,0),.02f);
            if(value<length) {
                if(value<0.f)return DREADZONE_POSITION_STEP_UNSUPPORTED_FLOAT;
                velocity=normalize(w,velocity,value,2);store_vector(w,b+0x130,&velocity);
            }
        } else if(secondary!=15 && secondary!=13 && secondary!=14 && half(w,b+0x3A0)==0) {
            norm=square3(w,&velocity,&square);
            if(threshold<norm) {
                if(threshold<0.f || maxxy<0.f)return DREADZONE_POSITION_STEP_UNSUPPORTED_FLOAT;
                velocity=normalize(w,velocity,maxxy,3);store_vector(w,b+0x130,&velocity);
            }
        }
    }
    velocity=vector(w,b+0x130);position=sum(w,vector(w,b+0xA0),velocity);store_vector(w,b+0xA0,&position);
    if(byte(w,b+0x2FCB)) {
        PositionVector zero={{0.f,0.f,0.f,0.f}};
        position=sum(w,position,vector(w,b+0x1E60));store_vector(w,b+0x1E60,&zero);
        store_vector(w,b+0xA0,&position);record[0x2FCB]=0;
    }
    second=vector(w,b+0x140);norm=square3(w,&second,&square);second_phase=norm>1e-8f;
    record[0x226]=0;store_word(w,b+0x20C,0);
    if(w->failed)return w->failed;
    /* With the actual secondary-position branch, EA88 is not called here.
     * Its inherited a0.high64 producer has not been closed by this API.
     * Refuse the reached path rather than supply a fabricated C60 W. */
    if(second_phase&&!w->track_incoming)return DREADZONE_POSITION_STEP_UNSUPPORTED_PATH;
    if(!second_phase && !capsule(w))return w->failed;
    if(!ground(w))return w->failed;
    if(!second_phase) {
        value=0.f;observed=difference(w,vector(w,b+0xA0),saved);store_vector(w,b+0x150,&observed);
        /* Mode0 + half2FE==0 executes original EA88 67EBEC QMFC2 a0=vf2
         * loaded from A0. All subsequent a0 writes before A3F8 C60 change
         * only low64. A nonzero half bypasses this producer: accept only a
         * definite earlier ground publisher. The scheduled capsule residue
         * remains symbolic and refuses before its W can be consumed. */
        if(half(w,b+0x2FE)!=0) {
            u32 word3;
            if(!w->track_incoming&&!w->ground_registers.last_writer_pc)return DREADZONE_POSITION_STEP_UNSUPPORTED_PATH;
            word3=w->ground_registers.high64[1];memcpy(&headingW,&word3,4);
        }else {position=vector(w,b+0xA0);headingW=position.lane[3];}
        if(!helper(w,DREADZONE_POSITION_SETTLE,0,0,&value))return w->failed;
    } else {u32 bits=w->high64[1];memcpy(&headingW,&bits,4);}
    observed=difference(w,vector(w,b+0xA0),saved);
    store_vector(w,b+0x160,&observed);store_vector(w,b+0x180,&observed);store_vector(w,b+0x170,&observed);
    observed=normalize(w,observed,1.f,3);
    velocity=project(w,vector(w,b+0x130),observed,b+0x160);store_vector(w,b+0x160,&velocity);
    planarVelocity=vector(w,b+0x130);
    /* Genuine unary mode0 leaf 68A7F0 on an actual caller-local capture:
     * copy all raw words, then overwrite Z with +0 + zero, preserving XY/W. */
    if(byte(w,b+0x2659))return DREADZONE_POSITION_STEP_UNSUPPORTED_PATH;
    planarVelocity.lane[2]=add(w,0.f,0.f);
    value=0.f;if(!helper(w,DREADZONE_POSITION_PROJECT,b+0x180,b+0x180,&value))return w->failed;
    observed=normalize(w,vector(w,b+0x180),1.f,3);
    velocity=project(w,planarVelocity,observed,b+0x180);store_vector(w,b+0x180,&velocity);
    xyVelocity=vector(w,b+0x130);
    if(!error_status(w,dreadzone_velocity_0068AC58(record,(u8 *)&xyVelocity,(u8 *)&xyVelocity,0.f)))return w->failed;
    velocity=vector(w,b+0x170);
    if(!error_status(w,dreadzone_velocity_0068AC58(record,(u8 *)&velocity,(u8 *)&velocity,0.f)))return w->failed;
    store_vector(w,b+0x170,&velocity);observed=normalize(w,velocity,1.f,3);
    velocity=project(w,xyVelocity,observed,b+0x170);store_vector(w,b+0x170,&velocity);
    velocity=vector(w,b+0x160);length=root(w,square3(w,&velocity,&square));
    store_word(w,b+0x1B0,raw(length));
    velocity=vector(w,b+0x160);
    {VelocityVector input;memcpy(&input,&velocity,16);value=checked(w,func_L01_0068A920(record,&input));}
    store_word(w,b+0x1B4,raw(value));
    if(!helper(w,DREADZONE_POSITION_HEADING,0,b+0x160,&value))return w->failed;
    w->info.outgoing_vf25_29_w[3]=heading_w(w,headingW);
    store_word(w,b+0x1B8,raw(value));
    {VelocityVector input;memcpy(&input,&velocity,16);value=checked(w,func_L01_0068AB40(record,&input));}
    store_word(w,b+0x1C0,raw(value));position=vector(w,b+0xA0);savedSecond=position;
    store_word(w,b+0x1B8,raw(dreadzone_velocity_max(number(w,b+0x1B8,16.f,0),0.f)));
    if(second_phase) {
        PositionVector zero={{0.f,0.f,0.f,0.f}};second=vector(w,b+0x140);
        store_vector(w,b+0x140,&zero);position=sum(w,position,second);store_vector(w,b+0xA0,&position);
        zero.lane[3]=second.lane[3];store_vector(w,b+0x140,&zero);
        /* Actual67FD74 PCPYUD duplicates140 upper64 into both a0 halves.
         * The scalar operations and call delay DADDU preserve its high64. */
        if(w->track_incoming)memcpy(w->high64,&second.lane[2],8);
        if(!capsule(w)||!ground(w))return w->failed;
        observed=difference(w,vector(w,b+0xA0),saved);store_vector(w,b+0x150,&observed);
        if(!helper(w,DREADZONE_POSITION_SETTLE,0,0,&value))return w->failed;
        observed=difference(w,vector(w,b+0xA0),savedSecond);store_vector(w,b+0x190,&observed);
        length=root(w,square3(w,&observed,&square));second=vector(w,b+0x140);
        observed.lane[3]=second.lane[3];second.lane[3]=0.f;
        store_word(w,b+0x1C4,raw(length));store_vector(w,b+0x190,&observed);store_vector(w,b+0x140,&second);
    } else {PositionVector zero={{0.f,0.f,0.f,0.f}};store_word(w,b+0x1C4,0);store_vector(w,b+0x190,&zero);}
    length=number(w,b+0x1B4,16.f,0);observed=difference(w,vector(w,b+0xA0),saved);
    store_word(w,b+0x1BC,0);store_vector(w,b+0x150,&observed);
    if(length>.004f) {
        value=division(w,observed.lane[2],length);
        value=dreadzone_velocity_max(dreadzone_velocity_min(value,.5f),-.5f);store_word(w,b+0x1BC,raw(value));
    }
    value=number(w,b+0x25C4,16.f,0);vertical=mul(w,mul(w,number(w,0x21DDF4,16.f,1),52.f),value);
    length=number(w,b+0x1B0,16.f,0);
    if(vertical<length) {
        observed=scale(w,vector(w,b+0x150),division(w,vertical,length),3);store_vector(w,b+0x150,&observed);
        store_word(w,b+0x1B0,raw(mul(w,mul(w,number(w,0x21DDF4,16.f,1),52.f),value)));
    }
    return w->failed?w->failed:DREADZONE_POSITION_STEP_APPROXIMATE;
}

static int position_execute(const DreadzonePositionStepRequest *request,int track_incoming,
                            const u32 incoming_high64[2],DreadzonePositionStepInfo *info) {
    PositionWork w;DreadzonePositionStepRequest *q;PositionVector old;int result;u8 *p;
    if(!host_span(request,sizeof(*request))||!host_span(info,sizeof(*info)))return DREADZONE_POSITION_STEP_INVALID;
    if(track_incoming&&!host_span(incoming_high64,8))return DREADZONE_POSITION_STEP_INVALID;
    memset(&w,0,sizeof(w));memcpy(&w.request,request,sizeof(*request));q=&w.request;
    w.track_incoming=track_incoming;
    if(track_incoming)memcpy(w.high64,incoming_high64,8);
    if(!q->storage_bytes||q->storage_bytes>16U*1024U*1024U||!host_span(q->storage,q->storage_bytes)||
        ((uintptr_t)q->storage&3U)||(u64)q->guest_base+q->storage_bytes>0x80000000ULL||
        (q->record_guest&15U)||q->record_guest<q->guest_base||q->storage_bytes<0x31F0U||
        q->record_guest-q->guest_base>q->storage_bytes-0x31F0U||
        (q->guest_base<0x70004000U&&(u64)q->guest_base+q->storage_bytes>0x70000000ULL)||
        overlaps(info,sizeof(*info),request,sizeof(*request))||overlaps(info,sizeof(*info),q->storage,q->storage_bytes))
        return DREADZONE_POSITION_STEP_INVALID;
    w.copy=(u8 *)malloc(q->storage_bytes);if(!w.copy)return DREADZONE_POSITION_STEP_TRANSACTION_MEMORY;
    memcpy(w.copy,q->storage,q->storage_bytes);memcpy(w.info.outgoing_vf25_29_w,q->incoming_vf25_29_w,20);
    old=vector(&w,q->record_guest+0xA0);
    result=w.failed?w.failed:body(&w);
    p=memory(&w,q->record_guest+0xA0,16,16);
    if(result>0&&w.failed)result=w.failed;
    if(result>0){w.info.position_changed=memcmp(p,&old,16)!=0;memcpy(q->storage,w.copy,q->storage_bytes);memcpy(info,&w.info,sizeof(w.info));}
    free(w.copy);return result;
}

int dreadzone_actor_position_step_0067F5E8(const DreadzonePositionStepRequest *request,DreadzonePositionStepInfo *info) {
    return position_execute(request,0,NULL,info);
}

int dreadzone_actor_position_state_0067F5E8(const DreadzonePositionStepRequest *request,
                                         const u32 incoming_high64[2],DreadzonePositionStepInfo *info) {
    return position_execute(request,1,incoming_high64,info);
}
