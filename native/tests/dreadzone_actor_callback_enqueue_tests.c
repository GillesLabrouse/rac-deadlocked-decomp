#include "dreadzone_actor_callback_enqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static u32 checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"callback enqueue check failed line%d\n",__LINE__);return 1;}}while(0)
#define BASE 0x21D000U
#define RECORD 0x368F60U
#define BYTES 0x1A0000U
static void word(u8 *b,u32 address,u32 value) {memcpy(b+address-BASE,&value,4);}
static u32 read_word(u8 *b,u32 address) {u32 value;memcpy(&value,b+address-BASE,4);return value;}
static void fixture(u8 *b) {
    memset(b,0xA5,BYTES);b[RECORD+0x25C8-BASE]=0;
    word(b,RECORD+0x2EF0,0x230000);word(b,0x222AF4,0);word(b,0x222A9C,0);
}
int main(void) {
    u8 *b=malloc(BYTES),*before=malloc(BYTES);DreadzoneCallbackEnqueueRequest q={0};
    DreadzoneCallbackEnqueueInfo info,old;u32 count;
    CHECK(b&&before);q.storage=b;q.guest_base=BASE;q.storage_bytes=BYTES;q.record_guest=RECORD;
    fixture(b);
    CHECK(dreadzone_actor_callback_enqueue_0066ABD8(&q,&info)==1);
    CHECK(info.queue_calls==1&&info.queued==1&&info.skipped==0);
    CHECK(info.entry_guest==0x241140&&info.argument_guest==0x230000);
    CHECK(b[RECORD+0x25C8-BASE]==1&&read_word(b,0x222AF4)==1);
    CHECK(read_word(b,0x241140)==0x66ACA0&&read_word(b,0x241144)==0x230000);
    CHECK(read_word(b,0x222A9C)==0x241140&&read_word(b,0x24114C)==0x241140);
    CHECK(read_word(b,0x241148)==0xA5A5A5A5U);
    memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_callback_enqueue_0066ABD8(&q,&info)==1);
    CHECK(info.skipped==1&&info.queue_calls==0&&!memcmp(before,b,BYTES));
    b[RECORD+0x25C8-BASE]=0;word(b,RECORD+0x2EF0,0x230040);
    CHECK(dreadzone_actor_callback_enqueue_0066ABD8(&q,&info)==1);
    CHECK(info.entry_guest==0x241150&&info.argument_guest==0x230040);
    CHECK(read_word(b,0x241148)==0x241150&&read_word(b,0x24114C)==0x241150);
    CHECK(read_word(b,0x24115C)==0xA5A5A5A5U&&read_word(b,0x222AF4)==2);
    fixture(b);word(b,0x222AF4,256);word(b,0x222A9C,0x500000);
    CHECK(dreadzone_actor_callback_enqueue_0066ABD8(&q,&info)==1);
    CHECK(info.capacity_exits==1&&info.queue_calls==1&&info.queued==0);
    CHECK(b[RECORD+0x25C8-BASE]==1&&read_word(b,0x222AF4)==256&&read_word(b,0x222A9C)==0x500000);
    fixture(b);count=((0x222AF0U-0x241140U)>>4)|0xF0000000U;word(b,0x222AF4,count);
    CHECK(dreadzone_actor_callback_enqueue_0066ABD8(&q,&info)==1);
    CHECK(info.entry_guest==0x222AF0&&read_word(b,0x222AF4)==0x230000);
    fixture(b);word(b,0x222A9C,0x230100);word(b,0x23010C,0x500000);
    memset(&info,0xA5,sizeof(info));old=info;memcpy(before,b,BYTES);
    CHECK(dreadzone_actor_callback_enqueue_0066ABD8(&q,&info)==DREADZONE_CALLBACK_ENQUEUE_INVALID);
    CHECK(!memcmp(b,before,BYTES)&&!memcmp(&info,&old,sizeof(info)));
    CHECK(dreadzone_actor_callback_enqueue_0066ABD8(NULL,&info)==DREADZONE_CALLBACK_ENQUEUE_INVALID);
    CHECK(dreadzone_actor_callback_enqueue_0066ABD8(&q,NULL)==DREADZONE_CALLBACK_ENQUEUE_INVALID);
    CHECK(dreadzone_actor_callback_enqueue_0066ABD8(&q,(DreadzoneCallbackEnqueueInfo *)b)==DREADZONE_CALLBACK_ENQUEUE_INVALID);
    CHECK(!memcmp(b,before,BYTES));
    fixture(b);memcpy(b+0x310000-BASE,&q,sizeof(q));
    CHECK(dreadzone_actor_callback_enqueue_0066ABD8((DreadzoneCallbackEnqueueRequest *)(b+0x310000-BASE),&info)==1);
    CHECK(info.queued==1&&info.argument_guest==0x230000);
    free(before);free(b);printf("callback enqueue checks: %u\n",checks);return 0;
}
