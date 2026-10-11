#include "dreadzone_actor_walking.h"
#include "dreadzone_actor_speed_target.h"
#include "dreadzone_actor_walk_parameters.h"
#include "dreadzone_actor_position_helpers.h"
#include "dreadzone_actor_slope.h"
#include "dreadzone_actor_velocity.h"
#include "dreadzone_input_angle_history.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Walking requires binary32 host floats
#endif
typedef struct Work {
    DreadzoneActorWalkingRequest request;
    DreadzoneActorWalkingInfo info;
    u8 *copy;
    DreadzoneActorInputContext context;
    DreadzoneActorInputMemory views[2];
    int failed;
} Work;
static int span(const void *p,uintptr_t n){return p&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *p,u32 n,const void *q,u32 m){
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;return a>=b?a-b<m:b-a<n;
}
static u8 *at(Work *w,u32 address,u32 bytes,u32 alignment){
    DreadzoneWallRequest *r=&w->request.wall;
    if(w->failed)return NULL;
    if((address&(alignment-1U))||address<r->guest_base||bytes>r->storage_bytes||
       address-r->guest_base>r->storage_bytes-bytes){w->failed=DREADZONE_WALKING_INVALID;return NULL;}
    return w->copy+address-r->guest_base;
}
static u32 word(Work *w,u32 o){u32 value=0;u8 *p=at(w,w->request.record_guest+o,4,4);if(p)memcpy(&value,p,4);return value;}
static s32 integer(Work *w,u32 o){u32 bits=word(w,o);s32 value;memcpy(&value,&bits,4);return value;}
static s16 half(Work *w,u32 o){s16 value=0;u8 *p=at(w,w->request.record_guest+o,2,2);if(p)memcpy(&value,p,2);return value;}
static u8 byte(Work *w,u32 o){u8 *p=at(w,w->request.record_guest+o,1,1);return p?*p:0;}
static void setbyte(Work *w,u32 o,u8 value){u8 *p=at(w,w->request.record_guest+o,1,1);if(p)*p=value;}
static u32 bits(f32 value){u32 b;memcpy(&b,&value,4);return b;}
static f32 checked(Work *w,f32 value){
    u32 magnitude=bits(value)&0x7FFFFFFFU;
    if(magnitude&&(magnitude<0x00800000U||magnitude>=0x7F800000U))
        if(!w->failed)w->failed=DREADZONE_WALKING_UNSUPPORTED_FLOAT;
    return value;
}
static f32 load(Work *w,u32 address){
    f32 value=0.f;u8 *p=at(w,address,4,4);if(p)memcpy(&value,p,4);
    checked(w,value);
    if((fabsf(value)>100000.f||(value!=0.f&&fabsf(value)<0x1p-40f))&&!w->failed)
        w->failed=DREADZONE_WALKING_UNSUPPORTED_FLOAT;
    return value;
}
static f32 field(Work *w,u32 o){return load(w,w->request.record_guest+o);}
static void store(Work *w,u32 o,f32 value){u8 *p;checked(w,value);p=at(w,w->request.record_guest+o,4,4);if(p)memcpy(p,&value,4);}
static f32 product(Work *w,f32 a,f32 b){
    volatile f32 result;checked(w,a);checked(w,b);result=a*b;
    if(a!=0.f&&b!=0.f&&result==0.f&&!w->failed)w->failed=DREADZONE_WALKING_UNSUPPORTED_FLOAT;
    return checked(w,result);
}
static f32 sum(Work *w,f32 a,f32 b){volatile f32 result;checked(w,a);checked(w,b);result=a+b;return checked(w,result);}
static f32 divide(Work *w,f32 a,f32 b){volatile f32 result;
    if(b==0.f){if(!w->failed)w->failed=DREADZONE_WALKING_UNSUPPORTED_FLOAT;return 0.f;}
    checked(w,a);checked(w,b);result=a/b;return checked(w,result);
}
static void copy(Work *w,u32 out,u32 input,u32 bytes){
    u8 retained[16];u8 *a,*b;
    if(bytes>sizeof(retained)){w->failed=DREADZONE_WALKING_INVALID;return;}
    a=at(w,w->request.record_guest+input,bytes,bytes==16?16:4);
    if(!a)return;memcpy(retained,a,bytes);
    b=at(w,w->request.record_guest+out,bytes,bytes==16?16:4);if(b)memcpy(b,retained,bytes);
}
static int status(Work *w,int result){
    if(result<0&&!w->failed)w->failed=DREADZONE_WALKING_UNSUPPORTED_PATH;
    return !w->failed;
}
static int recent(Work *w){
    s32 result=0,classification;
    if(half(w,0x5F4)!=0)return 1;
    if(integer(w,0x1AE0)!=1)return 0;
    ++w->info.history_calls;
    if(!status(w,dreadzone_direction_history(&w->context,0x28,2,0,word(w,0x2F00),&result)))return 0;
    if(!result)return 0;
    classification=func_L01_006739B0(w->context.record);
    if(classification==1)return 0;
    return func_L01_006739B0(w->context.record)!=15;
}
static f32 angle_history(Work *w,s32 requested){
    DreadzoneInputAngleHistoryRequest request;u32 value=0;f32 result;
    request=(DreadzoneInputAngleHistoryRequest){w->copy,w->request.wall.guest_base,
        w->request.wall.storage_bytes,word(w,0x2F00),requested};
    ++w->info.angle_history_calls;
    status(w,dreadzone_input_angle_history_005BD900(&request,&value));
    memcpy(&result,&value,4);return checked(w,result);
}
static int body(Work *w){
    f32 time,scale,angle,value,acceleration,deceleration,turn_scale=1.f,contribution;
    s32 classification;int alternate,selected;
    u32 record=w->request.record_guest;
    if(integer(w,0x25CC)!=2||byte(w,0x2659))return DREADZONE_WALKING_UNSUPPORTED_PATH;
    alternate=byte(w,0x434)!=0&&field(w,0x2E14)<.25f;
    if(byte(w,0x2661)||alternate)return w->failed?w->failed:DREADZONE_WALKING_UNSUPPORTED_PATH;
    selected=recent(w);time=load(w,0x21DDF4);
    if(w->failed)return w->failed;
    if(!selected||integer(w,0x25CC)==16){
        if(field(w,0x1B4)<product(w,product(w,time,1.7f),field(w,0x25C4))){
            setbyte(w,0x1C8C,1);
            if(fabsf(field(w,0x17A8))>2.0943952f&&field(w,0x2E08)==1.f)setbyte(w,0x1C8F,1);
        }
        value=angle_history(w,1);angle=angle_history(w,7);
        value=checked(w,func_L01_0055D910(value,angle));
        if(value>1.9198622f&&fabsf(field(w,0x17A8))>1.2217305f&&field(w,0x2E08)==1.f){
            setbyte(w,0x1C8C,1);setbyte(w,0x1C8F,1);
        }
        if(fabsf(field(w,0x17A8))<.08726646f)setbyte(w,0x1C8C,0);
    }
    if(integer(w,0x3B4)<3&&fabsf(field(w,0x17A8))>1.5707964f&&byte(w,0x5F6)){
        setbyte(w,0x1C8C,1);setbyte(w,0x1C8F,1);
    }
    if(byte(w,0x1C8F)&&fabsf(field(w,0x17A8))<.06981317f)setbyte(w,0x1C8F,0);
    if(field(w,0x2E08)<.2f){
        if(integer(w,0x1C90))setbyte(w,0x1C8F,1);
        copy(w,0x1790,0x1C70,16);copy(w,0x17A0,0x1C88,4);store(w,0x1780,0.f);
    }else{
        if(w->failed)return w->failed;
        ++w->info.direction_calls;
        if(!status(w,func_L01_0067CAF8(&w->context)))return w->failed;
        classification=integer(w,0x2314)==2?integer(w,0x2318):-1;
        if(classification==1&&(field(w,0x2E38)==1.f||integer(w,0x1C90)!=0)&&!byte(w,0x2658)){
            copy(w,0x1C70,0x1790,16);copy(w,0x1C88,0x17A0,4);
            {u8 *p=at(w,record+0x1C90,4,4);if(p)memcpy(p,&classification,4);}
        }else{copy(w,0x1C70,0x20,16);copy(w,0x1C88,0xB8,4);}
    }
    if(field(w,0x1E70)>0.f){
        DreadzonePositionHelperRequest r={0};r.operation=DREADZONE_POSITION_HEADING;r.target=record+0x1E60;
        ++w->info.heading_calls;
        if(w->failed||!status(w,dreadzone_actor_position_helper(&w->context,&r,&contribution)))return w->failed;
        value=sum(w,field(w,0x1780),contribution);scale=field(w,0x25C4);
        store(w,0x1780,value);time=load(w,0x21DDF4);
        {u8 *p=at(w,record+0x1E60,16,16);if(p)memset(p,0,16);}
        store(w,0x1780,dreadzone_velocity_max(value,product(w,product(w,time,.7f),scale)));
    }
    selected=recent(w);
    if(w->failed)return w->failed;
    if(selected&&integer(w,0x25CC)!=16)return DREADZONE_WALKING_UNSUPPORTED_PATH; /* genuine67BEC0 */
    copy(w,0x5F8,0xB8,4);copy(w,0x5E0,0x20,16);
    if(integer(w,0x25D0)==1&&!byte(w,0x1C8F)&&!byte(w,0x2658)){
        f32 force,damping,maximum;scale=field(w,0x25C4);
        force=product(w,product(w,load(w,0x21DDEC),.037f),scale);
        damping=product(w,product(w,load(w,0x21DDEC),.13f),scale);
        maximum=product(w,product(w,load(w,0x21DDF4),12.217304f),scale);
        ++w->info.turn_calls;
        if(w->failed||!status(w,func_L01_0067CFD0(&w->context,w->context.record,0,force,damping,maximum)))return w->failed;
    }else{
        angle=fabsf(field(w,0x17A8));
        if((field(w,0x2E08)<.9f||(byte(w,0x2658)&&!byte(w,0x1C8F)))&&angle>.7853982f)turn_scale=0.f;
        ++w->info.turn_calls;
        if(w->failed||!status(w,func_L01_0067D100(&w->context)))return w->failed;
    }
    ++w->info.parameter_calls;
    if(!status(w,dreadzone_actor_walk_parameters_00681B30(&w->context)))return w->failed;
    if(byte(w,0x1C8F)){
        f32 reduction,maximum;time=load(w,0x21DDF4);scale=field(w,0x25C4);angle=fabsf(field(w,0x17A8));
        reduction=divide(w,product(w,product(w,product(w,time,5.5f),scale),angle),3.1415927f);
        maximum=dreadzone_velocity_max(sum(w,product(w,product(w,time,7.25f),scale),-reduction),0.f);
        store(w,0x1780,dreadzone_velocity_min(field(w,0x1780),maximum));
    }
    time=load(w,0x21DDF8);scale=field(w,0x25C4);
    deceleration=product(w,product(w,time,30.f),scale);
    if(byte(w,0x1C8F)){
        value=dreadzone_velocity_min(sum(w,2.6179938f,-field(w,0x17A8)),1.f);
        store(w,0x1780,product(w,field(w,0x1780),dreadzone_velocity_max(value,.2f)));
        deceleration=product(w,product(w,load(w,0x21DDF8),17.f),scale);
    }
    angle=fabsf(field(w,0x17A8));
    if(field(w,0x2E08)<.75f&&angle>.61086524f)
        store(w,0x1780,product(w,field(w,0x1780),dreadzone_velocity_max(sum(w,1.f,-product(w,angle,.4f)),0.f)));
    acceleration=product(w,product(w,product(w,time,37.f),scale),turn_scale);
    ++w->info.speed_calls;
    if(w->failed)return w->failed;
    if(!status(w,dreadzone_actor_speed_0067D350(&w->context,acceleration,deceleration)))return w->failed;
    if(byte(w,0x1C8C)||(recent(w)&&integer(w,0x25CC)!=16)){
        if(w->failed)return w->failed;
        value=recent(w)&&integer(w,0x25CC)!=16?field(w,0x5F8):field(w,0x17A0);
    }else value=99999.f;
    ++w->info.velocity_calls;
    if(w->failed||!status(w,dreadzone_actor_velocity_0067D3A0(&w->context,value)))return w->failed;
    {
        DreadzoneActorWallResponseRequest r={0};r.wall=w->request.wall;r.wall.storage=w->copy;
        r.record_guest=record;memcpy(r.incoming_vf28_29_w,w->request.incoming_vf28_29_w,sizeof(r.incoming_vf28_29_w));
        /* The genuine D3A0 -> 652978 producer runs VCALLMS C60 at 652988.
         * VMULi.xyzw at micro CD0 writes VF28 from scalar VF15; its W is zero. */
        r.incoming_vf28_29_w[0]=0;
        if(!status(w,dreadzone_actor_wall_response_0067D6E8(&r,&w->info.wall)))return w->failed;
    }
    if(half(w,0x3D0))return DREADZONE_WALKING_UNSUPPORTED_PATH; /* actual6840B8 */
    ++w->info.height_calls;
    if(half(w,0x2FE)){
        value=product(w,product(w,load(w,0x21DDF8),25.f),field(w,0x25C4));
        if(w->failed||!status(w,dreadzone_actor_height_offset_0068A178(&w->context,record+0x130,record+0x170,value)))return w->failed;
        ++w->info.slope_calls;
        if(!status(w,dreadzone_actor_slope_bytes(&w->context,w->context.record)))return w->failed;
    }else{
        value=product(w,product(w,load(w,0x21DDF8),54.f),field(w,0x25C4));
        if(w->failed||!status(w,dreadzone_actor_height_offset_0068A178(&w->context,record+0x130,record+0x130,value)))return w->failed;
    }
    return w->failed?w->failed:DREADZONE_WALKING_APPROXIMATE;
}
int dreadzone_actor_walking_00695CB8(const DreadzoneActorWalkingRequest *request,DreadzoneActorWalkingInfo *info){
    Work w;DreadzoneWallRequest *r;u32 offset,tail,count=0;int result;
    if(!span(request,sizeof(*request))||!span(info,sizeof(*info)))return DREADZONE_WALKING_INVALID;
    memset(&w,0,sizeof(w));memcpy(&w.request,request,sizeof(*request));r=&w.request.wall;
    if(!r->storage_bytes||r->storage_bytes>16U*1024U*1024U||!span(r->storage,r->storage_bytes)||
       (u64)r->guest_base+r->storage_bytes>0x80000000ULL||
       (r->guest_base<0x70004000U&&(u64)r->guest_base+r->storage_bytes>0x70000000ULL)||
       overlap(info,sizeof(*info),request,sizeof(*request))||overlap(info,sizeof(*info),r->storage,r->storage_bytes)||
       (w.request.record_guest&15U)||w.request.record_guest<r->guest_base||r->storage_bytes<0x31F0U||
       w.request.record_guest-r->guest_base>r->storage_bytes-0x31F0U)return DREADZONE_WALKING_INVALID;
    offset=w.request.record_guest-r->guest_base;tail=r->storage_bytes-offset-0x31F0U;
    w.copy=(u8 *)malloc(r->storage_bytes);if(!w.copy)return DREADZONE_WALKING_MEMORY;
    memcpy(w.copy,r->storage,r->storage_bytes);
    if(offset)w.views[count++]=(DreadzoneActorInputMemory){w.copy,r->guest_base,offset,1};
    if(tail)w.views[count++]=(DreadzoneActorInputMemory){w.copy+offset+0x31F0U,w.request.record_guest+0x31F0U,tail,1};
    w.context=(DreadzoneActorInputContext){w.copy+offset,w.request.record_guest,0x31F0U,w.views,count};
    result=body(&w);
    if(result>0){memcpy(r->storage,w.copy,r->storage_bytes);memcpy(info,&w.info,sizeof(w.info));}
    free(w.copy);return result;
}
