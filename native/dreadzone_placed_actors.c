#include "dreadzone_placed_actors.h"
#include "dreadzone_fresh_class.h"
#include "dreadzone_actor_transform.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static uint32_t r32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint32_t r16(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8;}
static void w32(uint8_t *p,uint32_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static void w16(uint8_t *p,uint32_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
static int overlap(uint64_t a,uint64_t n,uint64_t b,uint64_t m){return a<b+m&&b<a+n;}
static int valid(float x){return isfinite(x)&&(x==0.0f||fabsf(x)>=FLT_MIN);}
static uint8_t *map(const DreadzonePlacedActors *q,uint8_t *ram,uint32_t a,uint32_t n){
    if(a<q->guest_base||(uint64_t)a+n>(uint64_t)q->guest_base+q->bytes)return NULL;
    return ram+a-q->guest_base;
}
int dreadzone_placed_actors_load(const DreadzonePlacedActors *request){
    DreadzonePlacedActors q;uint8_t *ram,*p,*actor,*definition,*record;uint8_t scratch[0x4000],vu[0x1000];
    uint32_t count,offset,map_offset,i,row,next,s3,s0,s1,s2,cid,special,pool,a,def,v[3],packed,old_flags;int status=-2;
    float first,second;volatile float product;
    if(!request||sizeof(q)>UINTPTR_MAX-(uintptr_t)request)return -1;q=*request;
    if(!q.storage||!q.bytes||q.bytes>0x1000000u||(q.guest_base&15)||q.guest_base>=0x80000000u||
       (uint64_t)q.guest_base+q.bytes>0x80000000u||q.bytes>UINTPTR_MAX-(uintptr_t)q.storage||q.gp!=0x22597F||
       overlap(q.guest_base,q.bytes,0x70000000u,0x4000)||!q.scratch||q.scratch_bytes<sizeof(scratch)||
       sizeof(scratch)>UINTPTR_MAX-(uintptr_t)q.scratch||!q.vu_data||q.vu_data_bytes<sizeof(vu)||
       sizeof(vu)>UINTPTR_MAX-(uintptr_t)q.vu_data||q.actor_pool_count==0||q.actor_pool_count>65536||
       overlap((uintptr_t)q.storage,q.bytes,(uintptr_t)q.scratch,sizeof(scratch))||
       overlap((uintptr_t)q.storage,q.bytes,(uintptr_t)q.vu_data,sizeof(vu))||
       overlap((uintptr_t)q.scratch,sizeof(scratch),(uintptr_t)q.vu_data,sizeof(vu)))return -1;
    if((q.header|q.output_cell)&3)return -3;
    ram=(uint8_t *)malloc(q.bytes);if(!ram)return -4;memcpy(ram,q.storage,q.bytes);memcpy(scratch,q.scratch,sizeof(scratch));memcpy(vu,q.vu_data,sizeof(vu));
#define MAPPED(ptr,address,length) do { (ptr)=map(&q,ram,(address),(length));if(!(ptr))goto finish; } while(0)
    MAPPED(p,0x220160,4);if(r32(p)!=0x1170D0){status=-3;goto finish;}
    MAPPED(p,q.output_cell,4);w32(p,0x243800);
    /* Both actual GP words are read even when mode0 later selects zero. */
    MAPPED(p,0x21EAD8,4);offset=q.mode?r32(p):0;MAPPED(p,0x21EADC,4);map_offset=q.mode?r32(p):0;
    MAPPED(p,q.header,4);count=r32(p);
    if((int32_t)count<=0)goto publish;
    if(count>1024||offset>65535||map_offset>65535||count+offset>q.actor_pool_count||count+map_offset>65536){status=-3;goto finish;}
    row=q.header+16;if(row<q.header){status=-3;goto finish;}
    for(i=0;i<count;i++){
        MAPPED(record,row,0x6C);
        /* Actual SRA2/SLL2 clears low bits, including a signed negative stride. */
        next=row+(r32(record)&~3u);
        s3=r32(record+4);s0=r32(record+8);s1=r16(record+0xC);s2=r16(record+0xE);cid=r32(record+0x10);special=cid==0x7D0;
        MAPPED(p,q.output_cell,4);a=r32(p);if((a&1)||(uint64_t)a+(map_offset+i)*2+2>UINT32_MAX){status=-3;goto finish;}
        MAPPED(p,a+(map_offset+i)*2,2);w16(p,offset+i);
        MAPPED(p,0x222D14,4);pool=r32(p);
        if((pool&15)||(uint64_t)pool+(offset+i)*256+256>UINT32_MAX){status=-3;goto finish;}a=pool+(offset+i)*256;
        MAPPED(p,row+0x10,4);cid=r32(p);
        {
            DreadzoneFreshClass fresh={ram,q.guest_base,q.bytes,q.gp,a,cid,1,scratch,sizeof(scratch),vu,sizeof(vu)};
            if(dreadzone_fresh_class_initialize(&fresh)!=1){status=-3;goto finish;}
        }
        MAPPED(actor,a,0x100);def=r32(actor+0x24);
        w16(actor+0xB2,s0);actor[0xB0]=(uint8_t)s3;w16(actor+0xB4,s1);w16(actor+0xB6,s2);
        if(def){
            MAPPED(p,def+0x24,4);memcpy(&first,p,4);MAPPED(p,row+0x14,4);memcpy(&second,p,4);product=first*second;
            if(!valid(first)||!valid(second)||!valid(product)){status=-3;goto finish;}memcpy(actor+0x70,(const void *)&product,4);
        }
        MAPPED(p,row+0x18,2);w16(actor+0x32,r16(p));MAPPED(p,row+0x1C,1);actor[0x30]=p[0];
        MAPPED(p,row+0x28,12);for(unsigned j=0;j<3;j++)v[j]=r32(p+j*4);
        for(unsigned j=0;j<3;j++)w32(actor+0x10+j*4,v[j]);w32(actor+0x1C,0);
        for(unsigned j=0;j<3;j++){MAPPED(p,row+0x34+j*4,4);w32(actor+0xF0+j*4,r32(p));}
        MAPPED(p,row+0x40,1);actor[0x21]=p[0];MAPPED(p,row+0x44,4);s0=r32(p);
        MAPPED(p,row+0x48,4);if(s0){status=-3;goto finish;}
        /* R4C is skipped on every supported no-height path. R50 is payload. */
        w16(actor+0x2E,0x7F80);MAPPED(p,row+0x50,4);w32(actor+0xAC,r32(p));MAPPED(p,row+0x54,4);if(!r32(p))w16(actor+0x2E,0);
        def=r32(actor+0x24);
        if(def){MAPPED(definition,def+0x44,2);old_flags=r16(definition);MAPPED(p,row+0x58,2);w16(definition,old_flags|r16(p));}
        MAPPED(p,row+0x58,2);old_flags=r16(actor+0x34);s0=r16(p);def=r32(actor+0x24);w16(actor+0x34,old_flags|s0);
        if(def){DreadzoneActorTransform transform={ram,q.guest_base,q.bytes,a,q.actor_pool_count,scratch,sizeof(scratch),vu,sizeof(vu)};
            if(dreadzone_actor_transform(&transform)!=1){status=-3;goto finish;}}
        MAPPED(p,0x222C50,4);if(r32(p)){status=-3;goto finish;}
        MAPPED(p,row+0x5C,4);v[0]=r32(p);MAPPED(p,row+0x60,4);v[1]=r32(p);MAPPED(p,row+0x64,4);v[2]=r32(p);
        MAPPED(p,row+0x68,4);s0=r32(p);packed=v[0]+(v[1]<<8)+(v[2]<<16);
        /* Real58D468 stores zero low32 and packed upper32; caller then reloads
         * its preserved final word and overwrites low32 at actor38. */
        w32(actor+0x38,0);w32(actor+0x3C,packed);w32(actor+0x38,s0);
        if(special){status=-3;goto finish;}row=next;
    }
publish:memcpy(q.storage,ram,q.bytes);memcpy(q.scratch,scratch,sizeof(scratch));status=1;
finish:free(ram);return status;
#undef MAPPED
}
