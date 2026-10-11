#include "dreadzone_fresh_class.h"
#include "dreadzone_guest_model_pool.h"
#include "dreadzone_model_transform.h"
#include <stdlib.h>
#include <string.h>
typedef struct ClassWork {
    DreadzoneFreshClass q;
    uint8_t *ram;
    uint32_t descriptors, matrices, model;
} ClassWork;
static uint32_t r32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint32_t r16(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8;}
static void w32(uint8_t *p,uint32_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static void w16(uint8_t *p,uint32_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
static int overlap(uint64_t a,uint64_t n,uint64_t b,uint64_t m){return a<b+m&&b<a+n;}
static uint8_t *map(ClassWork *w,uint32_t a,uint32_t n){
    if(a<w->q.guest_base||(uint64_t)a+n>(uint64_t)w->q.guest_base+w->q.bytes)return NULL;
    return w->ram+a-w->q.guest_base;
}
/* This bounded composition caches table fields only when later actor/pool
 * stores cannot change them. Model adapter separately validates its assets. */
static int input(ClassWork *w,uint32_t a,uint32_t n){
    uint32_t i,count;uint8_t *bank,*state;
    if(!map(w,a,n))return -2;
    if(overlap(a,n,w->q.actor,0x100))return -3;
    if(w->model&&(overlap(a,n,0x222D78,40)||overlap(a,n,w->descriptors,0x2000)||
        overlap(a,n,w->matrices,0xC2500)))return -3;
    if(w->model){
        state=map(w,0x222D78,40);bank=map(w,w->descriptors,0x2000);
        if(!state||!bank)return -2;
        count=r32(state+20);if(count>1024)return -3;
        for(i=0;i<count;i++)if(!bank[i*8+7]&&overlap(a,n,r32(bank+i*8),0x100))return -3;
    }
    return 1;
}
int dreadzone_fresh_class_initialize(const DreadzoneFreshClass *request){
    ClassWork w;uint8_t scratch[0x4000],inherited[0x1000];
    uint8_t *actor,*definition=NULL,*auxiliary=NULL,*p;
    uint32_t index,callback=0,handler=0,def=0,aux=0,environment,entry,upper=0,component=0;
    uint32_t preserved,i,matrix_result,flags,status_code;int status=-2;
    const uint32_t tables[][2]={{0x26B590,0x10000},{0x26B2B0,0x200},{0x26DD90,0x400},{0x26AE00,0x400},{0x26AA40,0x400},{0x222C50,4}};
    if(!request||sizeof(*request)>UINTPTR_MAX-(uintptr_t)request)return -1;
    memset(&w,0,sizeof(w));w.q=*request;
    if(!w.q.storage||!w.q.bytes||w.q.bytes>0x1000000u||w.q.guest_base>=0x80000000u||
       (uint64_t)w.q.guest_base+w.q.bytes>0x80000000u||w.q.bytes>UINTPTR_MAX-(uintptr_t)w.q.storage||
       overlap(w.q.guest_base,w.q.bytes,0x70000000u,0x4000)||w.q.gp!=0x22597Fu||
       w.q.object_class>0xFFFFu||!w.q.scratch||w.q.scratch_bytes<sizeof(scratch)||
       sizeof(scratch)>UINTPTR_MAX-(uintptr_t)w.q.scratch||!w.q.vu_data||
       w.q.vu_data_bytes<sizeof(inherited)||sizeof(inherited)>UINTPTR_MAX-(uintptr_t)w.q.vu_data||
       overlap((uintptr_t)w.q.storage,w.q.bytes,(uintptr_t)w.q.scratch,sizeof(scratch))||
       overlap((uintptr_t)w.q.storage,w.q.bytes,(uintptr_t)w.q.vu_data,sizeof(inherited))||
       overlap((uintptr_t)w.q.scratch,sizeof(scratch),(uintptr_t)w.q.vu_data,sizeof(inherited)))return -1;
    w.ram=(uint8_t *)malloc(w.q.bytes);if(!w.ram)return -4;
    memcpy(w.ram,w.q.storage,w.q.bytes);memcpy(scratch,w.q.scratch,sizeof(scratch));memcpy(inherited,w.q.vu_data,sizeof(inherited));
    actor=map(&w,w.q.actor,0x100);if(!actor)goto finish;
    if(w.q.actor&15){status=-3;goto finish;}
    if(!w.q.skip_cleanup&&r32(actor+0x58)){status=-3;goto finish;}
    for(i=0;i<sizeof(tables)/sizeof(tables[0]);i++){status=input(&w,tables[i][0],tables[i][1]);if(status!=1)goto finish;}
    index=map(&w,0x26B590+w.q.object_class,1)[0];
    p=map(&w,0x26B2B0+index*2,2);
    if((int16_t)r16(p)!=(int32_t)w.q.object_class)handler=r32(map(&w,0x26DD90+index*4,4));
    else {
        callback=r32(map(&w,0x26AE00+index*4,4));def=r32(map(&w,0x26AA40+index*4,4));
        if(def&3){status=-3;goto finish;}
        status=input(&w,def,0x4C);if(status!=1)goto finish;
        definition=map(&w,def,0x4C);w.model=definition[8]!=0;
        if(w.model){
            p=map(&w,0x222D78,40);if(!p){status=-2;goto finish;}
            w.descriptors=r32(p);w.matrices=r32(p+4);
            status=input(&w,def,0x4C);if(status!=1)goto finish;
            for(i=0;i<sizeof(tables)/sizeof(tables[0]);i++){status=input(&w,tables[i][0],tables[i][1]);if(status!=1)goto finish;}
            /* Clearing a currently live actor would invalidate a different
             * descriptor's input before the allocator can validate it. */
            if(overlap(w.q.actor,0x100,0x222D78,40)||overlap(w.q.actor,0x100,w.descriptors,0x2000)||
               overlap(w.q.actor,0x100,w.matrices,0xC2500)){status=-3;goto finish;}
        }
        aux=r32(definition+0x48);
        if(aux){status=input(&w,aux,0x20);if(status!=1)goto finish;auxiliary=map(&w,aux,0x20);}
    }
    environment=r32(map(&w,0x222C50,4));
    if(environment){
        status=input(&w,environment,2);if(status!=1)goto finish;
        if((environment&1)||(uint64_t)environment+r16(map(&w,environment,2))>UINT32_MAX){status=-3;goto finish;}
        entry=environment+r16(map(&w,environment,2));status=input(&w,entry,0x4B);if(status!=1)goto finish;
        if((entry+0x40)&3){status=-3;goto finish;}
        upper=r32(map(&w,entry+0x40,4));component=map(&w,entry+0x4A,1)[0];
    }
    preserved=actor[0x20]==0xFF?0:actor[0x2D];
    /* Actual filler055D2A8 has exactly64 word stores for this positive256B
     * request, before any defaults or environmental packing. */
    for(i=0;i<0x100;i+=4)w32(actor+i,0);
    actor[0x2D]=(uint8_t)preserved;actor[0x6D]=0xFF;w16(actor+0xBC,w.q.object_class);
    actor[0x2C]=0xFF;actor[0x21]=0xFF;actor[0x56]=0xFF;actor[0x64]=0xFF;actor[0x65]=0xFF;
    w16(actor+0x94,0);actor[0x6C]=0;actor[0x22]=(uint8_t)index;w16(actor+0x6E,0xFFFF);
    actor[0xA7]=0x80;actor[0x23]=0x80;actor[0xA6]=0x80;w16(actor+0x2E,0x7F80);actor[0xA5]=0x7F;actor[0xA4]=0x7F;
    w32(actor+0x38,environment?component|(component<<8):0);w32(actor+0x3C,environment?upper:0x00404040);
    if(!definition){
        w32(actor+0x24,0);w16(actor+0x34,0x45|(handler?0:2));w32(actor+0x98,0);w32(actor+0xA8,handler);
    } else {
        w32(actor+0xA8,callback);if(!callback)w16(actor+0x34,r16(actor+0x34)|2);
        w32(actor+0x24,def);actor[0x64]=definition[0xE];w16(actor+0x34,r16(actor+0x34)|r16(definition+0x44));
        w32(actor+0x98,r32(definition+0x10));w32(actor+0x70,r32(definition+0x24));
        if(definition[0xA]&&(int8_t)definition[0xE]>=0)actor[0x65]=(uint8_t)(definition[0xE]<<1);
        w16(actor+0x4C,1);w32(actor+0x48,0x3F800000);actor[0x57]=definition[8];
        if(w.model){
            DreadzoneGuestModelPool allocate={w.ram,w.q.guest_base,w.q.bytes,w.q.gp,w.q.actor,DREADZONE_GUEST_MODEL_ALLOCATE};
            status=dreadzone_guest_model_pool_apply(&allocate,&matrix_result);if(status!=1){status=-3;goto finish;}
            status_code=r32(definition+0x1C);
            if(status_code){status=input(&w,status_code,4);if(status!=1)goto finish;if(status_code&3){status=-3;goto finish;}status_code=r32(map(&w,status_code,4));}
            if(!status_code){
                status_code=r32(actor+0x98);
                if(status_code){status=input(&w,status_code,4);if(status!=1)goto finish;if(status_code&1){status=-3;goto finish;}status_code=r16(map(&w,status_code+2,2));}
                if(!status_code)actor[0x55]|=0x40;
            }
        } else {w32(actor+0x48,0);w16(actor+0x34,r16(actor+0x34)|0x40);}
        if(r32(definition+0x40)){w16(actor+0x34,r16(actor+0x34)|0x10);w32(actor+0x60,r32(definition+0x40));}
        if(definition[0xF]){actor[0x76]=0x18;w16(actor+0x34,r16(actor+0x34)|0x400);w32(actor+0x78,0);w32(actor+0x7C,0);actor[0x77]=0;}
        if(definition[6])actor[0x66]=0x18;
        if(auxiliary){
            w32(actor+0x40,aux);actor[0x54]=0;actor[0x6C]=auxiliary[0x12];actor[0x6D]=auxiliary[0x11];
            if(auxiliary[0x10]>=2)w16(actor+0x34,r16(actor+0x34)&0xFFFD);
            if(definition[0xC]==1&&auxiliary[0x10]<2){w32(actor+0x48,0);if((int8_t)auxiliary[0x11]<0)w16(actor+0x34,r16(actor+0x34)|0x40);}
        }
        if(r32(actor+0x58)){
            DreadzoneModelView view={w.q.guest_base,w.q.bytes,w.ram};
            DreadzoneInitialModelTransform model={{&view,1,w.q.actor,scratch,sizeof(scratch)},inherited,sizeof(inherited)};
            status=dreadzone_model_transform_initial(&model);if(status!=DREADZONE_TRANSFORM_APPROXIMATE){status=-3;goto finish;}
        }
        flags=r16(actor+0x36)|definition[0x47];w16(actor+0x36,flags);
    }
    memcpy(w.q.storage,w.ram,w.q.bytes);memcpy(w.q.scratch,scratch,sizeof(scratch));status=1;
finish:
    free(w.ram);return status;
}

