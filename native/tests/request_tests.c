#include "common.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compatible views of the fields used by the experimental recovered handler. */
typedef struct RequestObject { u8 pad0[0x2E20]; f32 f2E20; } RequestObject;
typedef struct RequestSnapshot { s32 state; s32 parameter2; s32 parameter3; } RequestSnapshot;
typedef struct StartupItem { s32 value; s32 kind; } StartupItem;
typedef struct StartupQueue {
    s32 f0; s32 f4; StartupItem items[10];
    s32 f58; s32 f5C; s32 f60; u8 f64; u8 gap65[3]; s32 f68; u8 f6C;
} StartupQueue;

_Static_assert(offsetof(RequestObject,f2E20)==0x2E20,"Request object field offset changed");
_Static_assert(sizeof(RequestSnapshot)==12,"Request history stride changed");
_Static_assert(sizeof(StartupQueue)==0x70,"Startup queue layout changed");

s32 D_0021DDB4_[1], D_0021EDC4_[1], D_0021EDD4_[1], D_0021EDD8_[1];
u8 D_003D7D70[256];
RequestObject *D_003367B8[11];
RequestSnapshot D_00270040[8];
s32 D_0021F2F4, D_0021F2E8, D_0021F2EC, D_0021EEB4, D_0021DAE0;
u8 *D_0021F2F0;

extern s32 func_00465C08(void);
extern s32 func_0047F568(s32,s32,s32,s32,u8 *);
extern void func_0045C768(StartupQueue *);
extern s32 func_0045C850(StartupQueue *,s32);
extern s32 func_0045C990(StartupQueue *);

/* This is the existing byte-filling boundary, not a game request-handler stub. */
void *func_0011A518(void *destination,s32 value,u32 count) {
    return memset(destination,value,count);
}

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr,"Request check failed at line %d: %s\n",__LINE__,#condition); \
    exit(EXIT_FAILURE); } } while (0)

static void reset(void) {
    D_0021DDB4_[0]=0; D_0021EDC4_[0]=-2;
    D_0021EDD4_[0]=1; D_0021EDD8_[0]=1;
    D_0021F2F4=0; D_0021F2E8=100; D_0021F2EC=200;
    D_0021F2F0=NULL; D_0021EEB4=1; D_0021DAE0=0;
    memset(D_003D7D70,0,sizeof(D_003D7D70));
    memset(D_003367B8,0,sizeof(D_003367B8));
    memset(D_00270040,0x5A,sizeof(D_00270040));
}

static void test_snapshot_and_pointer(void) {
    u8 completion=0xA5;
    RequestSnapshot before[8];
    reset(); D_0021DDB4_[0]=17;
    memcpy(before,D_00270040,sizeof(before));
    CHECK(func_0047F568(1,1,INT32_MIN,INT32_MAX,&completion)==0);
    CHECK(completion==0 && D_0021F2F0==&completion);
    CHECK(D_0021F2F4==1 && D_0021EDC4_[0]==1);
    CHECK(D_00270040[0].state==17 && D_00270040[0].parameter2==100 && D_00270040[0].parameter3==200);
    CHECK(memcmp(D_00270040+1,before+1,sizeof(before)-sizeof(before[0]))==0);
    CHECK(D_0021F2E8==INT32_MIN && D_0021F2EC==INT32_MAX);
    reset(); D_0021F2F4=8;
    memcpy(before,D_00270040,sizeof(before));
    CHECK(func_0047F568(-7,2,-11,23,NULL)==0);
    CHECK(D_0021EDC4_[0]==-7 && D_0021F2F4==8 && D_0021F2F0==NULL);
    CHECK(memcmp(before,D_00270040,sizeof(before))==0);
}

static void test_rejections_and_precedence(void) {
    u8 completion=0xA5;
    RequestSnapshot before[8];
    reset(); D_0021EDC4_[0]=4;
    memcpy(before,D_00270040,sizeof(before));
    CHECK(func_0047F568(1,1,7,9,&completion)==-1);
    CHECK(completion==0 && D_0021EDC4_[0]==4 && D_0021F2F0==NULL);
    CHECK(D_0021F2F4==0 && D_0021F2E8==100 && D_0021F2EC==200);
    CHECK(memcmp(before,D_00270040,sizeof(before))==0);
    D_0021F2F4=8; completion=0xA5;
    CHECK(func_0047F568(1,1,7,9,&completion)==-2 && completion==0);
    D_003D7D70[1]=1;
    CHECK(func_0047F568(1,1,7,9,NULL)==-3);
    CHECK(D_0021EDC4_[0]==4 && D_0021F2F4==8 && memcmp(before,D_00270040,sizeof(before))==0);
    reset(); D_0021F2F4=7;
    CHECK(func_0047F568(2,1,31,41,NULL)==0 && D_0021F2F4==8);
    CHECK(D_00270040[7].parameter2==100 && D_00270040[7].parameter3==200);
    D_0021EDC4_[0]=-2;
    CHECK(func_0047F568(2,1,51,61,NULL)==-2 && D_0021F2F4==8);
}

static void test_object_gate(void) {
    static RequestObject first,second;
    reset(); D_0021EDD4_[0]=0; D_0021DDB4_[0]=5;
    first.f2E20=0.0f; second.f2E20=-0.0f;
    D_003367B8[0]=&first;
    CHECK(func_0047F568(1,2,3,4,NULL)==0);
    D_0021EDC4_[0]=-2; D_003367B8[1]=&second;
    CHECK(func_0047F568(1,2,3,4,NULL)==-4);
    /* The object gate overrides the earlier pending/capacity/table failures. */
    D_0021EDC4_[0]=6; D_0021F2F4=8; D_003D7D70[1]=1;
    CHECK(func_0047F568(1,1,3,4,NULL)==-4);
    reset(); D_0021EDD4_[0]=0; D_0021DDB4_[0]=5;
    first.f2E20=NAN; D_003367B8[0]=&first;
    CHECK(func_0047F568(1,2,3,4,NULL)==0);
    D_0021EDC4_[0]=-2; first.f2E20=0.0f; D_0021EEB4=0;
    CHECK(func_0047F568(1,2,3,4,NULL)==-4);
    reset(); D_0021EDD4_[0]=0; D_0021DDB4_[0]=0; D_0021DAE0=3;
    D_003367B8[0]=&first; D_003367B8[1]=&second;
    CHECK(func_0047F568(1,2,3,4,NULL)==0);
    D_0021EDC4_[0]=-2; D_0021DAE0=2;
    CHECK(func_0047F568(1,2,3,4,NULL)==-4);
    reset(); D_0021EDD4_[0]=0; D_0021DDB4_[0]=5;
    D_003367B8[10]=&first; D_0021EEB4=0;
    CHECK(func_0047F568(1,2,3,4,NULL)==0); /* Slot ten is loaded but not examined. */
    D_0021DAE0=INT32_MIN; CHECK(func_00465C08()==INT32_MIN);
    D_0021DAE0=INT32_MAX; CHECK(func_00465C08()==INT32_MAX);
}

static void test_real_queue_dispatch(void) {
    StartupQueue queue;
    reset(); memset(&queue,0,sizeof(queue)); func_0045C768(&queue);
    CHECK(func_0045C850(&queue,23)==1 && func_0045C850(&queue,24)==1);
    CHECK(func_0045C990(&queue)==1);
    CHECK(D_0021EDC4_[0]==1 && D_0021F2F4==1 && D_0021F2E8==0 && D_0021F2EC==23);
    CHECK(D_00270040[0].state==0 && D_00270040[0].parameter2==100 && D_00270040[0].parameter3==200);
    CHECK(func_0045C990(&queue)==0); /* Pending request causes rejection and queue reset. */
    CHECK(queue.f60==0 && queue.f6C==0 && D_0021EDC4_[0]==1 && D_0021F2F4==1);
    reset(); D_0021DDB4_[0]=2; D_0021F2F4=8; func_0045C768(&queue);
    CHECK(func_0045C850(&queue,123)==1 && func_0045C990(&queue)==1);
    CHECK(D_0021EDC4_[0]==2 && D_0021F2F4==8 && D_0021F2E8==123 && D_0021F2EC==0);
}

int main(void) {
    test_snapshot_and_pointer(); test_rejections_and_precedence();
    test_object_gate(); test_real_queue_dispatch();
    puts("Experimental request preflight, snapshots and real queue dispatch passed.");
    return EXIT_SUCCESS;
}
