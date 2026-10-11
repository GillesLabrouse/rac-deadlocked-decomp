#include "dreadzone_pool_bounds.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
static uint32_t get32(const uint8_t *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static void set32(uint8_t *p,uint32_t v) {
    p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);
}
static uint8_t *mapped(const DreadzonePoolBoundsRequest *q,uint8_t *ram,uint32_t a,uint32_t n) {
    if(a<q->guest_base||(uint64_t)a+n>(uint64_t)q->guest_base+q->storage_bytes)return NULL;
    return ram+a-q->guest_base;
}
static uint8_t *word(const DreadzonePoolBoundsRequest *q,uint8_t *ram,uint32_t a) {
    return (a&3u)?NULL:mapped(q,ram,a,4u);
}
static int clear(const DreadzonePoolBoundsRequest *q,uint8_t *ram,uint32_t a,uint32_t count) {
    uint32_t bytes=count&0x80000000u?4u:count?count:4u;
    uint8_t *p;
    /* Actual first SW is unconditional. Signed ADDI count,-4 and the
     * exiting pointer increment can trap; native refusal is atomic. */
    if(count==0x80000000u||(a&3u)||(uint64_t)a+bytes>=0x80000000u)return 0;
    p=mapped(q,ram,a,bytes);if(!p)return 0;
    memset(p,0,bytes);return 1;
}
int dreadzone_pool_bounds_0057BAC0(const DreadzonePoolBoundsRequest *request) {
    DreadzonePoolBoundsRequest q;uint8_t *ram,*p;uint64_t end;
    uint32_t live,extra,tail,reserve,active_bytes,total_bytes,spare_count,cursor;
    uint32_t base,active_end,extra_end,spare_base,start,reloaded_end,span,descriptor,descriptor_end;
    int status=-2;
    if(!request||sizeof(*request)>UINTPTR_MAX-(uintptr_t)request)return -1;
    q=*request;end=(uint64_t)q.guest_base+q.storage_bytes;
    if(!q.storage||!q.storage_bytes||q.storage_bytes>0x1000000u||q.guest_base>=0x80000000u||end>0x80000000u||
       q.storage_bytes>UINTPTR_MAX-(uintptr_t)q.storage||(q.guest_base<0x70004000u&&end>0x70000000u))return -1;
    ram=(uint8_t *)malloc(q.storage_bytes);if(!ram)return -3;
    memcpy(ram,q.storage,q.storage_bytes);
#define READ(a,v) do {p=word(&q,ram,(a));if(!p)goto finish;(v)=get32(p);} while(0)
#define WRITE(a,v) do {p=word(&q,ram,(a));if(!p)goto finish;set32(p,(v));} while(0)
    READ(q.gp-0x6EA7u,live);READ(q.gp-0x6EBBu,extra);
    READ(q.gp-0x6EABu,tail);READ(q.gp-0x6E9Fu,reserve);
    spare_count=reserve+tail;active_bytes=(live+extra)<<8;
    READ(q.cursor_guest,cursor);total_bytes=(live+extra+spare_count)<<8;
    WRITE(q.cursor_guest,cursor+total_bytes);
    READ(q.gp-0x6EA7u,live);READ(0x222D14u,base);READ(q.gp-0x6EBBu,extra);
    active_end=base+(live<<8);spare_base=base+active_bytes;extra_end=active_end+(extra<<8);
    WRITE(0x21EC00u,base);WRITE(0x21EC04u,active_end);WRITE(0x222D2Cu,spare_base);
    WRITE(0x21EC10u,extra_end);WRITE(0x21EC08u,active_end);WRITE(0x21EC0Cu,active_end);
    p=mapped(&q,ram,spare_base+0x20u,1u);if(!p)goto finish;*p=0xFFu;
    READ(0x222D14u,base);READ(0x21EC00u,start);READ(0x21EC04u,reloaded_end);
    span=reloaded_end-start;
    WRITE(0x222D30u,base+total_bytes-0x100u);WRITE(q.gp-0x6E93u,span);
    READ(q.cursor_guest,cursor);WRITE(0x243790u,cursor);
    READ(q.cursor_guest,cursor);
    /* Complete 578928 uses A1=1 here, so its wrapping sum/mask is A2+A0. */
    WRITE(q.cursor_guest,cursor+span);
    WRITE(0x222D40u,spare_count<<2);READ(q.cursor_guest,cursor);WRITE(0x222D3Cu,cursor);
    READ(q.cursor_guest,cursor);descriptor=(cursor+(spare_count<<8)+0x3Fu)&0xFFFFFFC0u;
    WRITE(q.cursor_guest,descriptor);
    /* Complete 586F78: every post-store/post-clear read remains ordered. */
    WRITE(0x222D54u,descriptor+(spare_count<<2));WRITE(0x222D44u,descriptor);
    WRITE(0x222D50u,descriptor);WRITE(0x222D48u,0u);WRITE(q.gp-0x2C33u,0u);
    if(!clear(&q,ram,descriptor,spare_count<<2))goto finish;
    READ(0x222D54u,descriptor_end);
    WRITE(q.cursor_guest,(descriptor_end+0x3Fu)&0xFFFFFFC0u);
    memcpy(q.storage,ram,q.storage_bytes);status=1;
finish:
    free(ram);return status;
#undef READ
#undef WRITE
}
