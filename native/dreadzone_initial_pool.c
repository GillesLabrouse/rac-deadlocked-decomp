#include "dreadzone_initial_pool.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
static uint32_t get32(const uint8_t *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static void set32(uint8_t *p,uint32_t v) {
    p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);
}
static uint8_t *word(const DreadzoneInitialPoolRequest *q,uint8_t *ram,uint32_t a) {
    if((a&3u)||a<q->guest_base||(uint64_t)a+4u>(uint64_t)q->guest_base+q->storage_bytes)return NULL;
    return ram+a-q->guest_base;
}
static int clear(const DreadzoneInitialPoolRequest *q,uint8_t *ram,uint32_t address,uint32_t count) {
    uint32_t bytes=count&0x80000000u ? 4u : count ? count : 4u;
    /* Actual ADDI count,-4 traps at INT_MIN; the first SW would already exist.
     * All positive counts here are multiples of four. The final pointer ADDI
     * executes even when the first BLEZ or final BGTZ exits the loop. */
    if(count==0x80000000u || (address&3u)||address<q->guest_base||
        (uint64_t)address+bytes>=(uint64_t)0x80000000u ||
        (uint64_t)address+bytes>(uint64_t)q->guest_base+q->storage_bytes)return 0;
    memset(ram+address-q->guest_base,0,bytes);return 1;
}
int dreadzone_initial_pool_0057B6B8(const DreadzoneInitialPoolRequest *request) {
    DreadzoneInitialPoolRequest q;
    uint8_t *ram,*p;uint64_t end;uint32_t world,count,reserve,extra,trailing,base,bytes;
    uint32_t reserve_address,total_address,live_address,count_address,bytes_address,pool_address;
    int status=-2;
    if(!request||sizeof(*request)>UINTPTR_MAX-(uintptr_t)request)return -1;
    q=*request;end=(uint64_t)q.guest_base+q.storage_bytes;
    if(!q.storage||!q.storage_bytes||q.storage_bytes>0x1000000u||q.guest_base>=0x80000000u||end>0x80000000u||
        q.storage_bytes>UINTPTR_MAX-(uintptr_t)q.storage||(q.guest_base<0x70004000u&&end>0x70000000u))return -1;
    ram=(uint8_t *)malloc(q.storage_bytes);if(!ram)return -3;
    memcpy(ram,q.storage,q.storage_bytes);
#define READ(a,v) do {p=word(&q,ram,(a));if(!p)goto finish;(v)=get32(p);} while(0)
#define WRITE(a,v) do {p=word(&q,ram,(a));if(!p)goto finish;set32(p,(v));} while(0)
    READ(0x0021E824u,world);
    READ(q.header_guest+4u,reserve);READ(q.header_guest,count);
    reserve_address=q.gp+0xFFFF9161u;total_address=q.gp+0xFFFF915Du;
    live_address=q.gp+0xFFFF9159u;count_address=q.gp+0xFFFF9145u;
    bytes_address=q.gp+0xFFFF9165u;pool_address=q.gp+0xFFFFD395u;
    WRITE(reserve_address,reserve);if(world==1u)WRITE(reserve_address,500u);
    READ(count_address,extra);READ(reserve_address,reserve);READ(q.gp+0xFFFF9155u,trailing);
    bytes=(count+extra+reserve+trailing)<<8;
    WRITE(total_address,count);WRITE(live_address,0u);
    READ(q.pool_pointer_guest,base);
    WRITE(bytes_address,(reserve+trailing)<<8);WRITE(pool_address,base);
    if(!clear(&q,ram,base,bytes))goto finish;
    /* Complete57B6B0 returns A1=count in its JR delay slot, without memory access. */
    WRITE(live_address,count);
    if(!clear(&q,ram,0x00247A18u,0xC0u)||!clear(&q,ram,0x00247AD8u,0xC0u))goto finish;
    memcpy(q.storage,ram,q.storage_bytes);status=1;
finish:
    free(ram);return status;
#undef READ
#undef WRITE
}
