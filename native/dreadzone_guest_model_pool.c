#include "dreadzone_guest_model_pool.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#define STATE 0x222D78u
#define RECORDS 0x3094u
#define DESCRIPTORS 0x400u
#define MATRIX_BYTES 0xC2500u
#define DESCRIPTOR_BYTES 0x2000u
typedef struct Pool {
    DreadzoneGuestModelPool q;
    uint8_t *ram;
    uint32_t live_actor[DESCRIPTORS], live_definition[DESCRIPTORS], live_count;
} Pool;
static uint32_t read32(const uint8_t *p) {
    return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}
static void write32(uint8_t *p,uint32_t v) {
    p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);
}
static uint32_t read16(const uint8_t *p) {return (uint32_t)p[0]|(uint32_t)p[1]<<8;}
static void write16(uint8_t *p,uint32_t v) {p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
static int overlap(uint64_t a,uint64_t n,uint64_t b,uint64_t m) {return a<b+m&&b<a+n;}
static uint8_t *map(Pool *p,uint32_t a,uint32_t n) {
    if(a<p->q.guest_base||(uint64_t)a+n>(uint64_t)p->q.guest_base+p->q.bytes)return NULL;
    return p->ram+a-p->q.guest_base;
}
static uint32_t state(Pool *p,uint32_t i) {return read32(map(p,STATE+i*4,4));}
static void set_state(Pool *p,uint32_t i,uint32_t v) {write32(map(p,STATE+i*4,4),v);}
static int outside_banks(Pool *p,uint32_t a,uint32_t n) {
    return !overlap(a,n,STATE,40)&&!overlap(a,n,state(p,0),DESCRIPTOR_BYTES)&&
        !overlap(a,n,state(p,1),MATRIX_BYTES);
}
static int validate(Pool *p) {
    uint32_t descriptors,base,i,j,next=0,freed=0,released=0,first=0x7FFFFFFFu,n;
    uint8_t *bank;
    if(!map(p,STATE,40))return -2;
    descriptors=state(p,0);base=state(p,1);n=state(p,5);
    if((descriptors&7)||(base&15)||n>DESCRIPTORS||state(p,3)>RECORDS||
       overlap(descriptors,DESCRIPTOR_BYTES,base,MATRIX_BYTES)||
       overlap(descriptors,DESCRIPTOR_BYTES,STATE,40)||overlap(base,MATRIX_BYTES,STATE,40))return -3;
    bank=map(p,descriptors,DESCRIPTOR_BYTES);
    if(!bank||!map(p,base,MATRIX_BYTES))return -2;
    p->live_count=0;
    for(i=0;i<n;i++) {
        uint8_t *d=bank+i*8,*actor,*definition;
        uint32_t a=read32(d),def,count=d[6];
        if(!count||read16(d+4)!=next||next+count>RECORDS||d[7]>1)return -3;
        next+=count;
        if(d[7]) {
            if(first==0x7FFFFFFFu)first=i;
            freed+=count;released++;continue;
        }
        actor=map(p,a,0x100);if(!actor)return -2;
        if((a&3)||!outside_banks(p,a,0x100))return -3;
        def=read32(actor+0x24);definition=map(p,def,9);if(!definition)return -2;
        if(!outside_banks(p,def,9)||definition[8]!=count||
           read32(actor+0x58)!=base+read16(d+4)*64)return -3;
        p->live_actor[p->live_count]=a;p->live_definition[p->live_count++]=def;
    }
    if(state(p,2)!=base+next*64||state(p,3)!=RECORDS-next||state(p,4)!=freed||
       state(p,6)!=released||state(p,7)!=first)return -3;
    for(i=0;i<p->live_count;i++)for(j=0;j<p->live_count;j++) {
        if(i!=j&&overlap(p->live_actor[i],0x100,p->live_actor[j],0x100))return -3;
        if(overlap(p->live_definition[i],9,p->live_actor[j],0x100))return -3;
    }
    return 1;
}
static int selected(Pool *p,uint32_t actor,uint32_t definition) {
    uint32_t i;
    if((actor&3)||!outside_banks(p,actor,0x100)||!outside_banks(p,definition,9)||
       overlap(actor,0x100,definition,9))return -3;
    for(i=0;i<p->live_count;i++) {
        if(actor!=p->live_actor[i]&&overlap(actor,0x100,p->live_actor[i],0x100))return -3;
        if(overlap(actor,0x100,p->live_definition[i],9)||
           overlap(definition,9,p->live_actor[i],0x100))return -3;
    }
    return 1;
}
static int compact(Pool *p) {
    uint32_t destination,source,next,base=state(p,1),n=state(p,5),i,offset;
    uint8_t *bank=map(p,state(p,0),DESCRIPTOR_BYTES);
    if(state(p,6)<DESCRIPTORS-n&&state(p,4)<state(p,3))return 1;
    destination=state(p,7);
    if(destination>=n)return -3;
    next=read16(bank+destination*8+4);
    for(source=destination+1;source<n;source++) {
        uint8_t *d=bank+source*8;
        if(!d[7]) {
            uint8_t *actor=map(p,read32(d),0x100),quad[16],descriptor[8];
            uint32_t from=read32(actor+0x58),count=d[6];
            write32(actor+0x58,base+next*64);write16(d+4,next);
            for(i=0;i<count;i++) {
                for(offset=0;offset<64;offset+=16) {
                    memcpy(quad,map(p,from+i*64+offset,16),16);
                    memcpy(map(p,base+(next+i)*64+offset,16),quad,16);
                }
            }
            next+=count;
            /* Audited descriptor base is eight-byte aligned. Original paired
             * LDL/LDR and SDL/SDR access this complete aligned eight-byte block.
             * Actor/matrix footprints are disjoint; retain source capture. */
            memcpy(descriptor,d,8);memcpy(bank+destination*8,descriptor,8);destination++;
        }
    }
    set_state(p,2,base+next*64);set_state(p,3,RECORDS-next);set_state(p,5,destination);
    set_state(p,7,0x7FFFFFFFu);set_state(p,6,0);set_state(p,4,0);return 1;
}
int dreadzone_guest_model_pool_apply(const DreadzoneGuestModelPool *request,uint32_t *matrix_result) {
    Pool p;uint8_t *actor,*definition;uint32_t def,count,value=0,i;uint64_t end;
    int status=-2;
    if(!request||sizeof(*request)>UINTPTR_MAX-(uintptr_t)request)return -1;
    p.q=*request;end=(uint64_t)p.q.guest_base+p.q.bytes;
    if(!p.q.storage||!p.q.bytes||p.q.bytes>0x1000000u||p.q.guest_base>=0x80000000u||end>0x80000000u||
       p.q.bytes>UINTPTR_MAX-(uintptr_t)p.q.storage||p.q.gp!=0x22597Fu||
       overlap(p.q.guest_base,p.q.bytes,0x70000000u,0x4000)||
       p.q.operation<1||p.q.operation>3)return -1;
    if(p.q.operation==1&&(!matrix_result||sizeof(*matrix_result)>UINTPTR_MAX-(uintptr_t)matrix_result||
       overlap((uintptr_t)matrix_result,sizeof(*matrix_result),(uintptr_t)p.q.storage,p.q.bytes)||
       overlap((uintptr_t)matrix_result,sizeof(*matrix_result),(uintptr_t)request,sizeof(*request))))return -1;
    p.ram=(uint8_t *)malloc(p.q.bytes);if(!p.ram)return -5;
    memcpy(p.ram,p.q.storage,p.q.bytes);
    if(p.q.operation==3) {
        status=validate(&p);if(status!=1)goto finish;
        status=compact(&p);if(status!=1)goto finish;
    } else {
        actor=map(&p,p.q.actor,0x100);if(!actor||(p.q.actor&3))goto finish;
        if(p.q.operation==2&&!read32(actor+0x58)){status=1;goto finish;}
        def=read32(actor+0x24);
        if(p.q.operation==2&&!def){status=1;goto finish;}
        definition=map(&p,def,9);if(!definition)goto finish;
        count=definition[8];
        /* The zero-count branch still executes its GP-relative LW delay slot. */
        if(p.q.operation==2&&!count){status=map(&p,STATE+4,4)?1:-2;goto finish;}
        if(!count){status=-3;goto finish;}
        status=validate(&p);if(status!=1)goto finish;
        status=selected(&p,p.q.actor,def);if(status!=1)goto finish;
        if(p.q.operation==1) {
            uint8_t *d;
            for(i=0;i<state(&p,5);i++) {
                d=map(&p,state(&p,0)+i*8,8);
                if(!d[7]&&read32(d)==p.q.actor){status=-3;goto finish;}
            }
            if(state(&p,3)<count) {status=compact(&p);if(status!=1)goto finish;}
            if(state(&p,3)<count||state(&p,5)>=DESCRIPTORS){status=-4;goto finish;}
            value=state(&p,2);write32(actor+0x58,value);
            set_state(&p,3,state(&p,3)-count);set_state(&p,2,state(&p,2)+count*64);
            d=map(&p,state(&p,0)+state(&p,5)*8,8);
            write32(d,p.q.actor);write16(d+4,(read32(actor+0x58)-state(&p,1))>>6);
            d[6]=(uint8_t)count;d[7]=0;set_state(&p,5,state(&p,5)+1);
            value=read32(actor+0x58);
        } else {
            uint32_t first=(read32(actor+0x58)-state(&p,1))>>6,low=0,high=state(&p,5),middle=high>>1;
            uint8_t *d;
            for(;;) {
                if(low>high||middle>=state(&p,5)){status=-3;goto finish;}
                d=map(&p,state(&p,0)+middle*8,8);
                if(first<read16(d+4))high=middle-1;
                else if(read16(d+4)<first)low=middle+1;
                else break;
                middle=(high+low)>>1;
            }
            if(d[7]||read32(d)!=p.q.actor||d[6]!=count){status=-3;goto finish;}
            d[7]=1;set_state(&p,6,state(&p,6)+1);
            set_state(&p,4,state(&p,4)+definition[8]);
            if(middle<state(&p,7))set_state(&p,7,middle);
        }
        status=1;
    }
    memcpy(p.q.storage,p.ram,p.q.bytes);
    if(p.q.operation==1)*matrix_result=value;
finish:
    free(p.ram);return status;
}

