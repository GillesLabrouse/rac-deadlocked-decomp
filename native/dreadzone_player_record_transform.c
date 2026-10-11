#include "dreadzone_player_record_transform.h"
#include "dreadzone_player_volume_internal.h"
#include "dreadzone_actor_transform.h"
#include "dreadzone_actor_velocity.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
static uint32_t r32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint32_t r16(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8;}
static float rf(const uint8_t *p){float f;memcpy(&f,p,4);return f;}
static int valid(float f){return isfinite(f)&&(f==0.0f||fabsf(f)>=FLT_MIN);}
static int overlap(uint64_t a,uint64_t n,uint64_t b,uint64_t m){return a<b+m&&b<a+n;}
static uint8_t *map(const DreadzonePlayerRecordTransform *q,uint8_t *ram,uint32_t a,uint32_t n){
    if(a<q->guest_base||(uint64_t)a+n>(uint64_t)q->guest_base+q->bytes)return NULL;
    return ram+a-q->guest_base;
}
static int oscillator(const uint8_t *record){uint32_t state=r32(record+0x25CC);return state==0x7E||(state==0x99&&r32(record+0x25D8)==0x7E);}
static int predicate(const uint8_t *record){
    uint32_t state=r32(record+0x25CC);return (r32(record+0x25D4)-17u)<2u||state==106||state==129||state==117||state==116;
}
static int addition(float a,float b,float *out){volatile float f=a+b;*out=f;return valid(a)&&valid(b)&&valid(f);}
static int difference(float a,float b,float *out){volatile float f=a-b;*out=f;return valid(a)&&valid(b)&&valid(f);}
int dreadzone_player_record_transform(const DreadzonePlayerRecordTransform *request){
    DreadzonePlayerRecordTransform q;DreadzonePlayerVolume volume;uint8_t *ram,*record,*actor,*p;
    uint8_t scratch[0x4000],vu[0x1000],quad[32];uint32_t address,definition;float center[4],lower,radius,height,upper;int status=-2;
    if(!request||sizeof(q)>UINTPTR_MAX-(uintptr_t)request)return -1;q=*request;
    if(!q.storage||!q.bytes||q.bytes>0x1000000u||(q.guest_base&15)||q.guest_base>=0x80000000u||
       (uint64_t)q.guest_base+q.bytes>0x80000000u||q.bytes>UINTPTR_MAX-(uintptr_t)q.storage||
       overlap(q.guest_base,q.bytes,0x70000000u,0x4000)||!q.scratch||q.scratch_bytes<sizeof(scratch)||
       sizeof(scratch)>UINTPTR_MAX-(uintptr_t)q.scratch||!q.vu_data||q.vu_data_bytes<sizeof(vu)||
       sizeof(vu)>UINTPTR_MAX-(uintptr_t)q.vu_data||q.actor_pool_count==0||q.actor_pool_count>65536||
       overlap((uintptr_t)q.storage,q.bytes,(uintptr_t)q.scratch,sizeof(scratch))||
       overlap((uintptr_t)q.storage,q.bytes,(uintptr_t)q.vu_data,sizeof(vu))||
       overlap((uintptr_t)q.scratch,sizeof(scratch),(uintptr_t)q.vu_data,sizeof(vu)))return -1;
    if(q.record&15)return -3;
    ram=malloc(q.bytes);if(!ram)return -4;memcpy(ram,q.storage,q.bytes);memcpy(scratch,q.scratch,sizeof(scratch));memcpy(vu,q.vu_data,sizeof(vu));
#define MAPPED(pointer,a,n) do{(pointer)=map(&q,ram,(a),(n));if(!(pointer))goto finish;}while(0)
#define ACTOR() do{address=r32(record+0x2EF0);MAPPED(actor,address,0x100);if(address&15){status=-3;goto finish;}}while(0)
    MAPPED(record,q.record,0x31F0);ACTOR();actor[0x2C]=0xFF;
    ACTOR();memcpy(quad,record+0xA0,16);memcpy(actor+0x10,quad,16);
    if(oscillator(record)){status=-3;goto finish;}
    ACTOR();memcpy(quad,record+0xB0,16);memcpy(actor+0xF0,quad,16);
    if(oscillator(record)){status=-3;goto finish;}
    if((int16_t)r16(record+0x52C)!=0&&!r32(record+0x4D0)){ACTOR();memcpy(quad,record+0x520,4);memcpy(actor+0xF4,quad,4);}
    ACTOR();
    /* Actual55D3B8 loads pairs before its forward stores; the final extra
     * source quad is read even though only the third quad is published. */
    memcpy(quad,record+0x20,32);memcpy(actor+0xC0,quad,16);memcpy(actor+0xD0,quad+16,16);
    memcpy(quad,record+0x40,32);memcpy(actor+0xE0,quad,16);
    ACTOR();actor[0x55]&=0x7F;ACTOR();actor[0x55]&=0xDF;ACTOR();
    {DreadzoneActorTransform transform={ram,q.guest_base,q.bytes,address,q.actor_pool_count,scratch,sizeof(scratch),vu,sizeof(vu)};
     if(dreadzone_actor_transform(&transform)<0){status=-3;goto finish;}}
    ACTOR();definition=r32(actor+0x24);
    if(definition){if((uint64_t)definition+6>UINT32_MAX){status=-3;goto finish;}MAPPED(p,definition+6,1);if(!p[0]){ACTOR();}}
    actor[0x66]=0x10;
    if((int16_t)r16(record+0x3A0)!=0)goto publish;
    ACTOR();if(!r32(actor+0x98))goto publish;
    if(record[0x2659]!=0){status=-3;goto finish;}
    lower=rf(record+0x1F4);if(!valid(lower)){status=-3;goto finish;}
    memcpy(center,record+0xA0,16);
    if(!valid(center[2])||dreadzone_velocity_0068A178(record,(uint8_t *)center,(const uint8_t *)center,-lower)<0){status=-3;goto finish;}
    lower=rf(record+0x1F4);upper=rf(record+0x1F0);radius=rf(record+0x204);
    if(!difference(upper,lower,&height)||!valid(radius)){status=-3;goto finish;}if(height<0.05f)height=0.05f;
    ACTOR();memset(&volume,0,sizeof(volume));volume.storage=ram;volume.guest_base=q.guest_base;volume.bytes=q.bytes;volume.actor=address;
    memcpy(volume.center,center,16);volume.radius=radius;volume.height=height;
    if(dreadzone_player_volume_apply(&volume)<0){status=-3;goto finish;}
    if(!addition(rf(record+0x204),0.05f,&lower)){status=-3;goto finish;}
    if(r32(record+0x25D4)==4&&!addition(lower,0.15f,&lower)){status=-3;goto finish;}
    if(predicate(record)){lower=rf(record+0x1F4);if(!valid(lower)){status=-3;goto finish;}}
    memcpy(center,record+0xA0,16);
    if(!valid(center[2])||dreadzone_velocity_0068A178(record,(uint8_t *)center,(const uint8_t *)center,-lower)<0){status=-3;goto finish;}
    if(!difference(rf(record+0x1F0),lower,&height)){status=-3;goto finish;}radius=rf(record+0x204);
    if(!valid(radius)){status=-3;goto finish;}if(height<0.05f)height=0.05f;
    ACTOR();volume.actor=address;volume.index=1;memcpy(volume.center,center,16);volume.radius=radius;volume.height=height;
    if(dreadzone_player_volume_apply(&volume)<0){status=-3;goto finish;}
publish:memcpy(q.storage,ram,q.bytes);memcpy(q.scratch,scratch,sizeof(scratch));status=1;
finish:free(ram);return status;
#undef ACTOR
#undef MAPPED
}
