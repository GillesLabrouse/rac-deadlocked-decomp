#include "dreadzone_payload_relocation.h"
#include <stdlib.h>
#include <string.h>
static uint32_t r32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint32_t r16(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8;}
static void w32(uint8_t *p,uint32_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static uint8_t *map(const DreadzonePayloadRelocation *q,uint8_t *ram,uint32_t a,uint32_t n){
    if(a<q->guest_base||(uint64_t)a+n>(uint64_t)q->guest_base+q->bytes)return NULL;
    return ram+a-q->guest_base;
}
int dreadzone_payload_relocate(const DreadzonePayloadRelocation *request){
    DreadzonePayloadRelocation q;uint8_t *ram,*p,*actor,*row,*cell,*source,*destination;
    uint8_t pair[32];uint32_t begin,end,count,i,index,row_address,length,source_address,destination_address,j,footprint,cursor,total=0;int status=-2;
    if(!request||sizeof(q)>UINTPTR_MAX-(uintptr_t)request)return -1;q=*request;
    if(!q.storage||!q.bytes||q.bytes>0x1000000u||(q.guest_base&15)||q.guest_base>=0x80000000u||
       (uint64_t)q.guest_base+q.bytes>0x80000000u||q.bytes>UINTPTR_MAX-(uintptr_t)q.storage||
       ((uint64_t)q.guest_base<0x70004000u&&(uint64_t)q.guest_base+q.bytes>0x70000000u)||q.actor_limit>65536)return -1;
    if((q.cursor_cell&3)||(q.descriptors&3))return -3;
    ram=malloc(q.bytes);if(!ram)return -4;memcpy(ram,q.storage,q.bytes);
#define MAPPED(pointer,a,n) do{(pointer)=map(&q,ram,(a),(n));if(!(pointer))goto finish;}while(0)
    MAPPED(p,0x222D14,4);begin=r32(p);MAPPED(p,0x222D2C,4);end=r32(p);
    if(q.actor_begin)begin=q.actor_begin;if(q.actor_end)end=q.actor_end;
    if(begin>=0x80000000u||end>=0x80000000u||(begin&15)||(end&15)||end<begin||(end-begin)%256u){status=-3;goto finish;}
    count=(end-begin)/256u;if(count>q.actor_limit){status=-3;goto finish;}
    MAPPED(cell,q.cursor_cell,4);
    for(i=0;i<count;i++){
        MAPPED(actor,begin+i*256u,0x100);
        if(!r16(actor+0xBC)){w32(actor+0xAC,0);continue;}
        index=r32(actor+0xAC);if(index&0x80000000u){w32(actor+0xAC,0);continue;}
        if(index>65535u){status=-3;goto finish;}
        cursor=r32(cell);row_address=q.descriptors+(index<<3);MAPPED(row,row_address,8);
        length=r32(row+4);source_address=q.payload_base+r32(row);w32(actor+0xAC,cursor);
        destination_address=r32(cell);
        if(!length||(length&15)||length>0x100000u||length>q.bytes-total||
           (source_address&15)||(destination_address&15)){status=-3;goto finish;}
        total+=length;footprint=(length+31u)&~31u;
        if((uint64_t)source_address+footprint>=0x80000000u){status=-3;goto finish;}
        MAPPED(source,source_address,footprint);MAPPED(destination,destination_address,length);
        /* Both original LQ loads precede the first SQ. The odd final quad
         * still loads its following quad, but does not store that lookahead. */
        for(j=0;j<length;j+=32){memcpy(pair,source+j,32);memcpy(destination+j,pair,16);if(length-j>=32)memcpy(destination+j+16,pair+16,16);}
        cursor=r32(cell);w32(row,cursor);cursor=r32(cell);w32(cell,cursor+length);
    }
    cursor=r32(cell);if(cursor>0x7FFFFFC0u){status=-3;goto finish;}
    w32(cell,(cursor+63u)&~63u);memcpy(q.storage,ram,q.bytes);status=1;
finish:free(ram);return status;
#undef MAPPED
}
