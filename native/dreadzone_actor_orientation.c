#include "dreadzone_actor_orientation.h"
#include "dreadzone_actor_direction.h"
#include "dreadzone_input.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
typedef struct Run {DreadzoneActorOrientationRequest r;DreadzoneActorOrientationInfo info;int error;} Run;
static u8 *at(Run *r,u32 a,u32 n,u32 align){
    if(a%align||a<r->r.guest_base||n>r->r.storage_bytes||a-r->r.guest_base>r->r.storage_bytes-n){r->error=-1;return NULL;}
    return r->r.storage+a-r->r.guest_base;
}
static u32 word(Run *r,u32 a,u32 n){u32 v=0;u8 *p=at(r,a,n,n);if(p)memcpy(&v,p,n);return v;}
static s32 integer(Run *r,u32 a){u32 w=word(r,a,4);s32 v;memcpy(&v,&w,4);return v;}
static float check(Run *r,float x){
    u32 w;memcpy(&w,&x,4);w&=0x7FFFFFFFU;
    if(w&&(w<0x00800000U||w>=0x7F800000U))r->error=-3;return x;
}
static float value(Run *r,u32 a,float maximum){
    u32 w=word(r,a,4);float v;memcpy(&v,&w,4);check(r,v);
    if(fabsf(v)>maximum||(v!=0.f&&fabsf(v)<0x1p-40f))r->error=-3;return v;
}
static void put(Run *r,u32 a,float v){u8 *p=at(r,a,4,4);check(r,v);if(p)memcpy(p,&v,4);}
static float mul(Run *r,float a,float b){volatile float v=a*b;return check(r,v);}
static void compose(Run *r,float x,float y){
    u32 b=r->r.record_guest;
    if(x!=0.f||y!=0.f){r->error=-2;return;}
    put(r,b+0xB8,func_L01_0055D828(value(r,b+0xB8,3.1415927410125732421875f),0));
}
static void update(Run *r){
    u32 b=r->r.record_guest,mode=word(r,b+0x2659,1),i,active,flag,frame;float local[4],boost,time,force,damping,maximum;
    s32 state,previous;u8 *p;
    if(word(r,b+0x3D6,2))mode=1;
    if(mode!=0){if(mode<3)r->error=-2;else r->info.skips++;return;}
    state=integer(r,b+0x25CC);previous=integer(r,b+0x25D4);
    if((u32)((u32)state-36U)<2U||previous==17||state==118||state==48||state==44||state==43){r->info.skips++;return;}
    active=word(r,b+0x266D,1);boost=0;word(r,r->r.gp_guest-0x7B93U,4);
    if(!active){
        flag=word(r,0x220649,1);frame=word(r,r->r.gp_guest-0x7BC7U,4);
        if(flag&&((word(r,b+0x2F10,4)&7)!=(frame&7))){r->info.skips++;return;}
    }
    if(r->error)return;
    p=at(r,b+0xB0,16,16);if(!p)return;memcpy(local,p,16);
    for(i=0;i<2&&!r->error;i++){
        boost=value(r,i||!active?0x21DDEC:r->r.gp_guest-0x7B93U,16);
        check(r,local[i]);if(fabsf(local[i])>3.1415927410125732421875f)r->error=-3;
        time=value(r,b+0x25C4,16);force=mul(r,mul(r,boost,.015f),time);
        damping=mul(r,mul(r,boost,.3f),time);maximum=mul(r,mul(r,value(r,0x21DDF4,16),5.23598766326904296875f),time);
        if(force<0||force>1||damping<0||damping>1||maximum<0||maximum>16){r->error=-3;return;}
        value(r,b+0xC0+4*i,6.283185482025146484375f);p=at(r,b+0xC0+4*i,4,4);
        if(!p||r->error)return;
        check(r,dreadzone_direction_rotation_bytes((u8 *)(local+i),p,0,0,force,damping,maximum));r->info.turns++;
    }
    if(!r->error)compose(r,value(r,b+0xC0,3.1415927410125732421875f),value(r,b+0xC4,3.1415927410125732421875f));
}
static int overlap(uintptr_t a,uintptr_t n,uintptr_t b,uintptr_t m){return a>=b?a-b<m:b-a<n;}
int dreadzone_actor_orientation_00683310(const DreadzoneActorOrientationRequest *request,DreadzoneActorOrientationInfo *info){
    Run r;u8 *copy,*destination;uintptr_t a=(uintptr_t)request,b=(uintptr_t)info,c;
    if(!request||!info||a>UINTPTR_MAX-sizeof(*request)||b>UINTPTR_MAX-sizeof(*info)||overlap(a,sizeof(*request),b,sizeof(*info)))return -1;
    memset(&r,0,sizeof(r));memcpy(&r.r,request,sizeof(r.r));c=(uintptr_t)r.r.storage;
    if(!c||!r.r.storage_bytes||r.r.storage_bytes>16U*1024U*1024U||c>UINTPTR_MAX-r.r.storage_bytes||
       (u64)r.r.guest_base+r.r.storage_bytes>0x80000000ULL||
       (r.r.guest_base<0x70004000U&&(u64)r.r.guest_base+r.r.storage_bytes>0x70000000ULL)||
       overlap(b,sizeof(*info),c,r.r.storage_bytes)||!at(&r,r.r.record_guest,0x31F0,16))return -1;
    destination=r.r.storage;copy=(u8 *)malloc(r.r.storage_bytes);if(!copy)return -4;
    memcpy(copy,destination,r.r.storage_bytes);r.r.storage=copy;
    memcpy(r.info.outgoing_vf25_29_w,r.r.incoming_vf25_29_w,20);
    memcpy(r.info.outgoing_a0_high64,r.r.incoming_a0_high64,8);update(&r);
    if(!r.error){memcpy(destination,copy,r.r.storage_bytes);memcpy(info,&r.info,sizeof(*info));}
    free(copy);return r.error?r.error:1;
}
