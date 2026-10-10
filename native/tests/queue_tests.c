#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compatible scalar-layout fixtures for the recovered queue translation units. */
typedef struct StartupItem {
    s32 value;
    s32 kind;
} StartupItem;
typedef struct StartupQueue {
    s32 f0;
    s32 f4;
    StartupItem items[10];
    s32 f58;
    s32 f5C;
    s32 f60;
    u8 f64;
    u8 gap65[3];
    s32 f68;
    u8 f6C;
} StartupQueue;

_Static_assert(sizeof(StartupQueue)==0x70, "Queue ABI must preserve the retail scalar layout");
_Static_assert(offsetof(StartupQueue, items)==8, "Queue item offset changed");
_Static_assert(offsetof(StartupQueue, f58)==0x58, "Queue write index offset changed");
_Static_assert(offsetof(StartupQueue, f5C)==0x5C, "Queue read index offset changed");
_Static_assert(offsetof(StartupQueue, f60)==0x60, "Queue count offset changed");
_Static_assert(offsetof(StartupQueue, f68)==0x68, "Queue mode offset changed");
_Static_assert(offsetof(StartupQueue, f6C)==0x6C, "Queue lock offset changed");

extern void func_0045C768(StartupQueue *);
extern s32 func_0045C7B0(StartupQueue *, s32 *, s32 *);
extern s32 func_0045C850(StartupQueue *, s32);
extern void func_0045C8C8(StartupQueue *);
extern s32 func_0045C990(StartupQueue *);
s32 D_0021DDB4_[1];

/* The native fixture substitutes CRT byte filling for PS2 vectorized memset. */
void *func_0011A518(void *destination, s32 value, u32 count) {
    return memset(destination,value,count);
}

static unsigned handler_calls;
static s32 handler_arguments[4];
static u8 *handler_completion;
static s32 handler_result;

/* Capture the unrecovered handler boundary; do not simulate game behavior. */
s32 func_0047F568(s32 request, s32 mode, s32 parameter2, s32 parameter3, u8 *completion) {
    handler_calls++;
    handler_arguments[0]=request;
    handler_arguments[1]=mode;
    handler_arguments[2]=parameter2;
    handler_arguments[3]=parameter3;
    handler_completion=completion;
    return handler_result;
}

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr,"Queue check failed at line %d: %s\n",__LINE__,#condition); \
    exit(EXIT_FAILURE); } } while (0)

static void initialized(StartupQueue *queue) {
    const u8 *bytes=(const u8 *)queue->items;
    CHECK(queue->f58==0 && queue->f5C==0 && queue->f60==0);
    CHECK(queue->f68==1 && queue->f6C==0);
    for (unsigned i=0;i<sizeof(queue->items);i++) CHECK(bytes[i]==0xCD);
}

static void test_fifo_and_capacity(void) {
    const s32 values[10]={INT32_MIN,-1,0,1,99,100,101,1024,65536,INT32_MAX};
    StartupQueue queue, saved;
    s32 value=-123, kind=-456;
    memset(&queue,0x5A,sizeof(queue));
    queue.f0=0x11223344; queue.f4=-765;
    func_0045C768(&queue);
    initialized(&queue);
    CHECK(queue.f0==0x11223344 && queue.f4==-765 && queue.f64==0x5A);
    for (unsigned i=0;i<sizeof(queue.gap65);i++) CHECK(queue.gap65[i]==0x5A);
    CHECK(func_0045C7B0(&queue,&value,&kind)==0);
    CHECK(value==-123 && kind==-456);
    for (unsigned i=0;i<10;i++) CHECK(func_0045C850(&queue,values[i])==1);
    CHECK(queue.f58==0 && queue.f60==10);
    memcpy(&saved,&queue,sizeof(queue));
    CHECK(func_0045C850(&queue,17)==0);
    CHECK(memcmp(&saved,&queue,sizeof(queue))==0);
    for (unsigned i=0;i<10;i++) {
        CHECK(func_0045C7B0(&queue,&value,&kind)==1);
        CHECK(value==values[i] && kind==(values[i]>=100));
        CHECK(queue.items[i].value==-1 && queue.items[i].kind==-1);
        CHECK(queue.f60==(s32)(9-i));
    }
    CHECK(queue.f5C==0);
    value=17; kind=23;
    CHECK(func_0045C7B0(&queue,&value,&kind)==0 && value==17 && kind==23);
    queue.f6C=1;
    memcpy(&saved,&queue,sizeof(queue));
    CHECK(func_0045C850(&queue,123)==0);
    CHECK(memcmp(&saved,&queue,sizeof(queue))==0);
    func_0045C8C8(&queue);
    initialized(&queue);
    CHECK(queue.f0==0x11223344 && queue.f4==-765);
}

static void test_wraparound_and_lock(void) {
    StartupQueue queue={0};
    s32 value, kind;
    func_0045C768(&queue);
    for (s32 i=0;i<8;i++) CHECK(func_0045C850(&queue,i)==1);
    for (s32 i=0;i<6;i++) {
        CHECK(func_0045C7B0(&queue,&value,&kind)==1);
        CHECK(value==i && kind==0);
    }
    for (s32 i=8;i<16;i++) CHECK(func_0045C850(&queue,i)==1);
    CHECK(queue.f58==6 && queue.f5C==6 && queue.f60==10);
    queue.f6C=1;
    for (s32 i=6;i<16;i++) {
        CHECK(func_0045C7B0(&queue,&value,&kind)==1);
        CHECK(value==i && kind==0);
        CHECK(queue.f6C==(i==15 ? 0 : 1));
    }
    CHECK(queue.f5C==6 && queue.f60==0);
}

static void reset_capture(s32 result) {
    handler_calls=0;
    memset(handler_arguments,0x5A,sizeof(handler_arguments));
    handler_completion=(u8 *)&handler_calls;
    handler_result=result;
}

static void test_dispatch_arguments(void) {
    const s32 states[]={INT32_MIN,-2,-1,0,1,2,3,16,INT32_MAX};
    const s32 values[]={-1,99,100,INT32_MAX};
    for (unsigned s=0;s<sizeof(states)/sizeof(states[0]);s++) {
        for (unsigned v=0;v<sizeof(values)/sizeof(values[0]);v++) {
            StartupQueue queue={0};
            func_0045C768(&queue);
            CHECK(func_0045C850(&queue,values[v])==1);
            D_0021DDB4_[0]=states[s];
            queue.f64=0xA5;
            reset_capture(0);
            CHECK(func_0045C990(&queue)==1);
            CHECK(handler_calls==1 && handler_completion==NULL);
            CHECK(handler_arguments[0]==(values[v]>=100 ? 2 : 1));
            CHECK(handler_arguments[1]==((states[s]==1 || states[s]==2) ? 2 : 1));
            CHECK(handler_arguments[2]==(values[v]>=100 ? values[v] : 0));
            CHECK(handler_arguments[3]==(values[v]>=100 ? 0 : values[v]));
            CHECK(queue.f64==0 && queue.f60==0 && queue.f6C==0);
        }
    }
}

static void test_dispatch_results(void) {
    StartupQueue queue={0}, saved;
    func_0045C768(&queue);
    reset_capture(0);
    memcpy(&saved,&queue,sizeof(queue));
    CHECK(func_0045C990(&queue)==0 && handler_calls==0);
    CHECK(memcmp(&saved,&queue,sizeof(queue))==0);
    CHECK(func_0045C850(&queue,23)==1);
    CHECK(func_0045C850(&queue,24)==1);
    D_0021DDB4_[0]=2;
    reset_capture(7);
    CHECK(func_0045C990(&queue)==0 && handler_calls==1);
    CHECK(queue.f60==1 && queue.f6C==1 && queue.f68==2);
    CHECK(func_0045C850(&queue,25)==0);
    reset_capture(-4);
    CHECK(func_0045C990(&queue)==0 && handler_calls==1);
    initialized(&queue);
    CHECK(func_0045C850(&queue,100)==1);
    CHECK(func_0045C850(&queue,101)==1);
    reset_capture(-1);
    CHECK(func_0045C990(&queue)==0);
    initialized(&queue);  /* Failure discards the still-pending second request. */
    CHECK(func_0045C850(&queue,123)==1);
    queue.items[queue.f5C].kind=2;
    reset_capture(0);
    CHECK(func_0045C990(&queue)==1 && handler_calls==0);
    CHECK(queue.f60==0 && queue.f6C==0);
}

int main(void) {
    test_fifo_and_capacity();
    test_wraparound_and_lock();
    test_dispatch_arguments();
    test_dispatch_results();
    puts("Recovered startup queue preserves capacity, FIFO, locking and dispatch arguments.");
    return EXIT_SUCCESS;
}
