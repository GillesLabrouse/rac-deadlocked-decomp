#include "dreadzone_actor_transform.h"
#include "dreadzone_actor_basis.h"
#include "dreadzone_spatial_grid.h"
#include "dreadzone_model_transform.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static uint32_t r32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint32_t r16(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8;}
static void w32(uint8_t *p,uint32_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static float rf(const uint8_t *p){float x;memcpy(&x,p,4);return x;}
static int overlap(uint64_t a,uint64_t n,uint64_t b,uint64_t m){return a<b+m&&b<a+n;}
static int valid(float x){return isfinite(x)&&(x==0.0f||fabsf(x)>=FLT_MIN);}
static int mul(float a,float b,float *v){volatile float x=a*b;*v=x;return valid(a)&&valid(b)&&valid(x);}
static int add(float a,float b,float *v){volatile float x=a+b;*v=x;return valid(a)&&valid(b)&&valid(x);}
static int sub(float a,float b,float *v){volatile float x=a-b;*v=x;return valid(a)&&valid(b)&&valid(x);}
static uint8_t *map(const DreadzoneActorTransform *q,uint8_t *ram,uint32_t a,uint32_t n){
    if(a<q->guest_base||(uint64_t)a+n>(uint64_t)q->guest_base+q->bytes)return NULL;
    return ram+a-q->guest_base;
}
int dreadzone_actor_transform(const DreadzoneActorTransform *request){
    DreadzoneActorTransform q;uint8_t *ram,*actor,*animation,*p,*grid,*blocks,*bitmap;
    uint8_t scratch[0x4000],inherited[0x1000];float basis[16],offset[4],world[4],bounds[4],term,acc,scale,angles[3];
    uint32_t flags,i,key=0,pool;int16_t generation;int status=-3;
    if(!request||sizeof(q)>UINTPTR_MAX-(uintptr_t)request)return -1;
    q=*request;
    if(!q.storage||!q.bytes||q.bytes>0x1000000u||(q.guest_base&15)||q.guest_base>=0x80000000u||
       (uint64_t)q.guest_base+q.bytes>0x80000000u||q.bytes>UINTPTR_MAX-(uintptr_t)q.storage||
       overlap(q.guest_base,q.bytes,0x70000000u,0x4000)||
       !q.scratch||q.scratch_bytes<sizeof(scratch)||sizeof(scratch)>UINTPTR_MAX-(uintptr_t)q.scratch||
       !q.vu_data||q.vu_data_bytes<sizeof(inherited)||sizeof(inherited)>UINTPTR_MAX-(uintptr_t)q.vu_data||
       overlap((uintptr_t)q.storage,q.bytes,(uintptr_t)q.scratch,sizeof(scratch))||
       overlap((uintptr_t)q.storage,q.bytes,(uintptr_t)q.vu_data,sizeof(inherited))||
       overlap((uintptr_t)q.scratch,sizeof(scratch),(uintptr_t)q.vu_data,sizeof(inherited)))return -1;
    if(q.actor&15)return -3;
    ram=(uint8_t *)malloc(q.bytes);if(!ram)return -4;
    memcpy(ram,q.storage,q.bytes);memcpy(scratch,q.scratch,sizeof(scratch));memcpy(inherited,q.vu_data,sizeof(inherited));
    actor=map(&q,ram,q.actor,0x100);if(!actor){status=-2;goto finish;}
    if((int8_t)actor[0x20]<0){status=0;goto finish;}
    if(r32(actor+0x50))goto finish;
    flags=r16(actor+0x34);
    if(!(flags&0x100)){
        if(r32(actor+0xFC)!=0)goto finish;
        memcpy(angles,actor+0xF0,sizeof(angles));
        if(dreadzone_actor_basis(angles,basis)!=DREADZONE_BASIS_APPROXIMATE)goto finish;
        if(flags&0x8000)for(i=0;i<3;i++)if(!sub(0.0f,basis[4+i],basis+4+i))goto finish;
        memcpy(actor+0xC0,basis,48);
    }else memcpy(basis,actor+0xC0,48);
    memcpy(offset,actor+0x80,16);
    if(actor[0x54]!=actor[0x56]){
        uint32_t address=r32(actor+0x40);
        animation=map(&q,ram,address,16);if(!animation){status=-2;goto finish;}
        if(address&15)goto finish;
        actor[0x56]=actor[0x54];memcpy(offset,animation,16);memcpy(actor+0x80,animation,16);
    }
    scale=rf(actor+0x70);
    for(i=0;i<4;i++)if(!mul(offset[i],scale,offset+i))goto finish;
    memcpy(world,offset,16);
    for(i=0;i<3;i++){
        if(!mul(basis[i],offset[0],&acc)||!mul(basis[4+i],offset[1],&term)||!add(acc,term,&acc)||
           !mul(basis[8+i],offset[2],&term)||!add(acc,term,&acc)||
           !mul(rf(actor+0x10+i*4),1024.0f,&term)||!add(acc,term,world+i))goto finish;
    }
    generation=(int16_t)r16(actor+0x94);if(generation<32767)++generation;
    actor[0x94]=(uint8_t)generation;actor[0x95]=(uint8_t)((uint16_t)generation>>8);
    memcpy(actor,world,16);
    /* These operations execute even when actor98 is zero; their W source is
     * the scaled animation radius, not position.W or an invented constant. */
    if(!add(0.0f,world[0],bounds+2)||!sub(world[0],world[3],bounds)||!sub(world[1],world[3],bounds+1))goto finish;
    if(r32(actor+0x98)){
        if(!add(world[3],world[1],bounds+3)||!add(bounds[2],world[3],bounds+2))goto finish;
        for(i=0;i<4;i++){
            if(!isfinite(bounds[i])||bounds[i]<-2147483648.0f||bounds[i]>=2147483648.0f)goto finish;
            key|=(((uint32_t)(int32_t)bounds[i]>>14)&255)<<(i*8);
        }
        if(key!=r32(actor+0xA4)&&!(key&0xC0C0u)){
            DreadzoneSpatialRequest spatial;
            p=map(&q,ram,0x222D14,4);grid=map(&q,ram,0x26FF20,0x4000);blocks=map(&q,ram,0x273F20,0xC000);bitmap=map(&q,ram,0x28BF20,0xC0);
            if(!p||!grid||!blocks||!bitmap){status=-2;goto finish;}pool=r32(p);
            memset(&spatial,0,sizeof(spatial));spatial.actor=actor;spatial.actor_bytes=0x100;
            spatial.grid=grid;spatial.grid_bytes=0x4000;spatial.blocks=blocks;spatial.block_bytes=0xC000;spatial.bitmap=bitmap;spatial.bitmap_bytes=0xC0;
            spatial.actor_guest=q.actor;spatial.actor_pool_guest=pool;spatial.actor_pool_count=q.actor_pool_count;spatial.rectangle=key;
            if(dreadzone_spatial_update(&spatial)!=DREADZONE_SPATIAL_COMMITTED)goto finish;
        }
    }
    if(actor[0x55]&0xA0){
        DreadzoneModelView view={q.guest_base,q.bytes,ram};
        DreadzoneInitialModelTransform model={{&view,1,q.actor,scratch,sizeof(scratch)},inherited,sizeof(inherited)};
        int result=dreadzone_model_transform_initial(&model);
        if(result!=DREADZONE_TRANSFORM_APPROXIMATE&&result!=DREADZONE_TRANSFORM_NO_MODEL)goto finish;
    }
    memcpy(q.storage,ram,q.bytes);memcpy(q.scratch,scratch,sizeof(scratch));status=1;
finish:free(ram);return status;
}
