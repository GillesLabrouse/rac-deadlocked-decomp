#include "dreadzone_input_poll.h"
#include "dreadzone_input_context_init.h"
#include "dreadzone_memory.h"
#include "dreadzone_input_repeat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Platform responses below are explicit test captures. No actual controller,
 * RPC or feedback transport implementation is provided. The complete-chain
 * case binds the genuine post-pool repetition pass; other cases capture it.
 * The state machine, callback, decoder, math and actor reset all execute C.
 */
DreadzonePoolRecord D_00375720[DREADZONE_POOL_COUNT];
u32 D_00220828;
s32 D_002208A8, D_002208AC;
u8 D_0021DE31[1];
s32 D_0021DDB4_[1], D_0021E8A0[1];
static unsigned checks;

typedef struct Capture {
    s32 state, read, modes[2], mode, request[2], select, align, pressure, query_pressure, hint;
    unsigned modes_count, request_count, count, post_count;
    u32 calls[128];
    s32 ports[128], slots[128], arg2[128], arg3[128];
    u8 packet[32], alignment[6], feedback[6];
    DreadzoneRepeatContext *repeat;
} Capture;

static void check(int condition, const char *message) {
    checks++;
    if (!condition) { fprintf(stderr, "Dreadzone poll check failed: %s\n", message); exit(1); }
}

static void event(Capture *c, u32 address, s32 port, s32 slot, s32 a2, s32 a3) {
    unsigned i = c->count++;
    check(i < 128, "capture event storage remains bounded");
    c->calls[i] = address; c->ports[i] = port; c->slots[i] = slot;
    c->arg2[i] = a2; c->arg3[i] = a3;
}
static s32 state_capture(void *u, s32 p, s32 s) { Capture *c=u; event(c,0x13BA98,p,s,0,0); return c->state; }
static s32 read_capture(void *u, s32 p, s32 s, u8 *b) {
    Capture *c=u; event(c,0x13BA20,p,s,0,0);
    if (c->read) memcpy(b,c->packet,32);
    return c->read;
}
static s32 modes_capture(void *u, s32 p, s32 s, s32 a, s32 b) {
    Capture *c=u; unsigned i=c->modes_count++;
    event(c,0x13BD50,p,s,a,b); check(i<2,"mode query capture count"); return c->modes[i];
}
static s32 mode_capture(void *u, s32 p, s32 s, s32 a, s32 b) {
    Capture *c=u; event(c,0x13BE88,p,s,a,b); return c->mode;
}
static s32 request_capture(void *u, s32 p, s32 s) {
    Capture *c=u; unsigned i=c->request_count++;
    event(c,0x13BBD0,p,s,0,0); check(i<2,"request state is actually queried twice"); return c->request[i];
}
static s32 select_capture(void *u, s32 p, s32 s, s32 a, s32 b) {
    Capture *c=u; event(c,0x13BC30,p,s,a,b); return c->select;
}
static s32 alignment_capture(void *u, s32 p, s32 s, const u8 *b) {
    Capture *c=u; event(c,0x13C008,p,s,0,0); memcpy(c->alignment,b,6); return c->align;
}
static s32 pressure_capture(void *u, s32 p, s32 s) { Capture *c=u; event(c,0x13C190,p,s,0,0); return c->pressure; }
static s32 query_pressure_capture(void *u, s32 p, s32 s) { Capture *c=u; event(c,0x13C210,p,s,0,0); return c->query_pressure; }
static s32 hint_capture(void *u, s32 p) { Capture *c=u; event(c,0x13B0F8,p,0,0,0); return c->hint; }
static s32 feedback_capture(void *u, s32 p, s32 s, const u8 *b) {
    Capture *c=u; event(c,0x13BF40,p,s,0,0); memcpy(c->feedback,b,6); return 1;
}
static void post_capture(void *u) {
    Capture *c=u; event(c,0x614EE8,0,0,0,0); c->post_count++;
    if(c->repeat) check(dreadzone_input_repeat(c->repeat)==0,"bound real post-pool repeat pass completes");
}
static const DreadzonePollPlatform platform = {
    state_capture, read_capture, modes_capture, mode_capture, request_capture,
    select_capture, alignment_capture, pressure_capture, query_pressure_capture,
    hint_capture, feedback_capture, post_capture
};

static void put(InputRecord5C0 *r, unsigned o, u32 w) {
    u8 *b=(u8 *)r+o;
    b[0]=(u8)w; b[1]=(u8)(w>>8); b[2]=(u8)(w>>16); b[3]=(u8)(w>>24);
}
static u32 get(const InputRecord5C0 *r, unsigned o) {
    const u8 *b=(const u8 *)r+o;
    return (u32)b[0]|((u32)b[1]<<8)|((u32)b[2]<<16)|((u32)b[3]<<24);
}
static void setup(DreadzonePollContext *h, InputRecord5C0 *pool, Capture *c) {
    memset(h,0,sizeof(*h)); memset(pool,0,8*sizeof(*pool)); memset(c,0,sizeof(*c));
    h->pool=pool; h->pool_count=8; h->pool_guest_base=0x1EE600;
    h->platform=&platform; h->platform_context=c;
    c->state=6; c->read=1; c->packet[1]=0x79;
    c->packet[2]=c->packet[3]=255;
    c->packet[4]=c->packet[5]=c->packet[6]=c->packet[7]=128;
}

static void test_default_state_table(void) {
    /* All ten jump-table destinations are recovered from the actual table.
     * This list is the independent non-default set, not a C switch clone.
     */
    static const unsigned specific[]={0,40,41,42,70,71,72,76,77,78};
    DreadzonePollContext h;
    InputRecord5C0 pool[8]; Capture c;
    unsigned state,i;
    for (state=0; state<79; state++) {
        int skip=0;
        for(i=0;i<sizeof(specific)/sizeof(specific[0]);i++) if(state==specific[i]) skip=1;
        if(skip) continue;
        setup(&h,pool,&c); put(pool,0x198,state);
        ((u8 *)pool)[0x54C]=3; ((u8 *)pool)[0x558]=2;
        check(dreadzone_poll_record(&h,0)==DREADZONE_POLL_COMPLETE,"all default jump-table states execute connected packet path");
        check(c.count==4 && c.calls[0]==0x13BA98 && c.calls[1]==0x13B0F8 && c.calls[2]==0x13BA20 && c.calls[3]==0x13BF40,
              "default states preserve actual platform call order");
        check(c.ports[0]==3 && c.slots[0]==2 && c.ports[2]==3 && c.slots[2]==2,
              "port and slot byte fields reach actual driver ABI");
        check(get(pool,0x198)==state && get(pool,0x1DC)==0x79 && get(pool,0x55C)==0x79,
              "default state retains configuration and records packet type");
    }
}

static void test_configuration(void) {
    /* Expected next states and ordered addresses are independent observations
     * for selected platform responses, including actual fallthrough paths.
     */
    static const struct {unsigned state; s32 first,second; unsigned next; u32 call1,call2; } cases[]={
        {0,4,0,40,0x13BD50,0x13BD50}, {0,4,7,70,0x13BD50,0x13BD50},
        {0,3,0,99,0x13BD50,0x13BD50}, {0,0,0,0,0x13BD50,0},
        {40,1,1,42,0x13BD50,0x13BE88}, {40,1,0,41,0x13BD50,0x13BE88}, {40,0,1,99,0x13BD50,0},
        {41,1,0,42,0x13BE88,0}, {41,0,0,41,0x13BE88,0},
        {42,1,2,41,0x13BBD0,0x13BBD0}, {42,2,0,0,0x13BBD0,0x13BBD0},
        {71,1,0,71,0x13BBD0,0x13BBD0}, {71,2,0,72,0x13BBD0,0x13BBD0},
        {77,1,2,76,0x13BBD0,0x13BBD0}, {77,2,0,78,0x13BBD0,0x13BBD0},
        {72,1,0,76,0x13C190,0}, {72,0,0,99,0x13C190,0},
        {76,0,0,72,0x13C210,0}, {76,1,0,77,0x13C210,0},
        {78,0,0,99,0,0}
    };
    DreadzonePollContext h; InputRecord5C0 pool[8]; Capture c;
    unsigned i;
    for(i=0;i<sizeof(cases)/sizeof(cases[0]);i++) {
        setup(&h,pool,&c); put(pool,0x198,cases[i].state);
        c.modes[0]=cases[i].first; c.modes[1]=cases[i].second;
        c.mode=cases[i].state==40?cases[i].second:cases[i].first;
        c.request[0]=cases[i].first; c.request[1]=cases[i].second;
        c.pressure=c.query_pressure=cases[i].first;
        check(dreadzone_poll_record(&h,0)==0 && get(pool,0x198)==cases[i].next,"actual configuration next state");
        check(c.count==(unsigned)(1+(cases[i].call1!=0)+(cases[i].call2!=0)),"configuration operation count");
        if(cases[i].call1) check(c.calls[1]==cases[i].call1,"first configuration operation address");
        if(cases[i].call2) check(c.calls[2]==cases[i].call2,"second configuration operation address");
        for(unsigned call=1;call<c.count;call++) {
            if(c.calls[call]==0x13BE88) check(c.arg2[call]==1 && c.arg3[call]==3,"mode request preserves actual mode1/lock3 ABI");
            if(c.calls[call]==0x13BD50) check(c.arg3[call]==0 && (c.arg2[call]==1 || c.arg2[call]==2),"mode query selector and index are exact");
        }
        check(c.calls[0]==0x13BA98 && pool[0].f190==0,"configuration states query connection without decoding a packet");
    }
    for(i=0;i<4;i++) {
        static const u8 expected[]={0,1,255,255,255,255};
        setup(&h,pool,&c); put(pool,0x198,70); c.select=(s32)(i/2); c.align=(s32)(i%2);
        check(dreadzone_poll_record(&h,0)==0 && get(pool,0x198)==(i/2?70U:99U)+i%2,
              "failed selection is retained before alignment completion increments state");
        check(c.count==3 && c.calls[1]==0x13BC30 && c.arg2[1]==-1 && c.arg3[1]==0 && c.calls[2]==0x13C008,
              "state70 preserves configuration ABI and still calls actual alignment boundary");
        check(memcmp(c.alignment,expected,6)==0 && memcmp((u8 *)pool+0x552,expected,6)==0,
              "real alignment bytes are passed intact to the six-byte core ABI");
    }
}

static void test_actual_callback_dispatch(void) {
    DreadzonePollContext h; InputRecord5C0 pool[8]; Capture c;
    DreadzoneActorInit31F0 actor;
    DreadzonePollContextView view;
    unsigned frame, nibble;
    setup(&h,pool,&c); memset(&actor,0xA5,sizeof(actor)); func_L01_006A6598(&actor);
    view.guest_address=0x00368F60+0x2FEC; view.storage=&actor.fields.f2FEC;
    h.contexts=&view; h.context_count=1;
    put(pool,0x198,1); put(pool,0x56C,0x6B5CB8); put(pool,0x570,view.guest_address);
    D_002208AC=D_002208A8=0;
    for(frame=0;frame<8;frame++) {
        c.count=0; c.packet[2]=frame<4?0xF7:0xFF;
        check(dreadzone_poll_record(&h,0)==0,"poll invokes real bound callback then actual decoder");
        check(pool[0].f1A0==(frame<4?0x800:0) && pool[0].f1A4==(frame==0?0x800:0) && pool[0].f1A8==(frame==4?0x800:0),
              "actual poll previous masks and callback ordering retain press/hold/release");
        check(view.storage->f1D8==(s32)frame+1 && pool[0].f190==(s32)frame+1,
              "actual callback frame counter and decoder history advance through poll");
        check(c.count==(frame==0?4U:3U),"port hint occurs only on first connected pass");
    }
    check(D_002208AC==8,"real callback statistics accumulate across poll groups");
    for(nibble=0;nibble<16;nibble++) {
        setup(&h,pool,&c); put(pool,0x198,1); c.packet[1]=(u8)(0x70+nibble);
        c.packet[2]=0xF7; c.packet[4]=255; c.packet[8]=255;
        check(dreadzone_poll_record(&h,0)==0,"each protocol length nibble passes bounded decoder path");
        check(pool[0].f100[0]==(nibble>=3?1.0f:0.0f) && pool[0].f100[4]==(nibble>=9?1.0f:0.0f),
              "poll derives length2+2*nibble capped20 at actual axis and pressure thresholds");
    }
}

static void test_failure_and_fallback(void) {
    DreadzonePollContext h; InputRecord5C0 pool[8]; Capture c;
    DreadzoneActorInit31F0 actor;
    DreadzonePollContextView view;
    unsigned i;
    setup(&h,pool,&c); h.primary_0021DDCC=h.pool_guest_base;
    c.state=0; pool[0].f348=0x800; pool[0].f1B0=0x400;
    put(pool,0x198,70); ((u8 *)pool)[0x559]=1;
    check(dreadzone_poll_record(&h,0)==0 && get(pool,0x198)==0 && get(pool,0x19C)==0,
          "disconnect clears configuration before entering actual neutral state0 path");
    check(pool[0].f1A0==0 && pool[0].f1A8==0x800 && pool[0].f1BC==0x400 && pool[0].f190==1,
          "neutral fallback actually decodes release after preserving caller masks");
    check(((u8 *)pool)[0x551]==1 && ((u8 *)pool)[0x559]==0 && h.flag_0021DA60==1 && c.count==1,
          "fallback leaves disconnection marker and avoids packet/feedback transport");
    setup(&h,pool,&c); memset(&actor,0xA5,sizeof(actor)); func_L01_006A6598(&actor);
    view.guest_address=0x00368F60+0x2FEC; view.storage=&actor.fields.f2FEC;
    h.contexts=&view; h.context_count=1;
    put(pool,0x56C,0x6B5CB8); put(pool,0x570,view.guest_address); c.state=1;
    check(dreadzone_poll_record(&h,0)==0 && view.storage->f1D8==1 && view.storage->f0[0]==0xFFFF && pool[0].f190==1,
          "nonready state0 runs the real callback and decoder on actual neutral values");
    check(c.count==1 && ((u8 *)pool)[0x559]==0,"neutral callback dispatch does not invent a packet or feedback request");
    setup(&h,pool,&c); put(pool,0x198,1); c.state=2;
    check(dreadzone_poll_record(&h,0)==0 && pool[0].f190==1 && c.calls[2]==0x13BA20,
          "both confirmed connection states2 and6 reach actual packet dispatch");
    for(i=0;i<3;i++) {
        setup(&h,pool,&c); put(pool,0x198,1); ((u8 *)pool)[0x551]=1;
        if(i==0) c.state=1;
        if(i==1) c.read=0;
        if(i==2) c.packet[0]=1;
        put(pool,0x1DC,0x79); put(pool,0x55C,0x79);
        for(unsigned j=0;j<6;j++) ((u8 *)pool)[0x188+j]=(u8)(20+j);
        check(dreadzone_poll_record(&h,0)==0 && pool[0].f190==0,"nonready/failed/invalid-status paths do not invent decoded packets");
        if(i==2) {
            static const u8 expected[]={20,21,22,23,24,25};
            check(c.calls[c.count-1]==0x13BF40 && memcmp(c.feedback,expected,6)==0,
                  "invalid packet status still performs actual six-byte feedback call");
            check(((u8 *)pool)[0x551]==0 && ((u8 *)pool)[0x188]==0 && ((u8 *)pool)[0x189]==0 &&
                  ((u8 *)pool)[0x18A]==22 && ((u8 *)pool)[0x18D]==25,
                  "feedback clears only the two observed bytes afterward");
        } else check(((u8 *)pool)[0x551]==1,"no-read paths preserve connection marker");
    }
    setup(&h,pool,&c); put(pool,0x198,1); put(pool,0x55C,0x41);
    check(dreadzone_poll_record(&h,0)==0 && get(pool,0x198)==0 && get(pool,0x55C)==0,
          "changed positive packet type restarts real configuration");
    setup(&h,pool,&c); put(pool,0x198,99); pool[0].f348=3; pool[0].f1B0=4;
    check(dreadzone_poll_record(&h,0)==0 && c.count==4 && pool[0].f1AC==3 && pool[0].f1BC==4 && pool[0].f190==1,
          "configured state99 reaches the real packet path after preserving prior masks");
    {
        static const u32 states[]={79,80,0x7FFFFFFF,0x80000000,0xFFFFFFFF};
        for(i=0;i<sizeof(states)/sizeof(states[0]);i++) {
            setup(&h,pool,&c); put(pool,0x198,states[i]);
            check(dreadzone_poll_record(&h,0)==0 && c.count==4 && pool[0].f190==1,
                  "unsigned out-of-table states take the actual default packet branch");
        }
    }
}

static void test_pool_and_boundaries(void) {
    DreadzonePollContext h; InputRecord5C0 pool[8]; Capture c;
    DreadzonePollPlatform missing=platform;
    unsigned i;
    setup(&h,pool,&c);
    for(i=0;i<8;i++) { put(&pool[i],0x198,1); pool[i].f54C=(u8)i; }
    check(dreadzone_poll_pool(&h)==0 && c.post_count==1 && c.count==33 && c.calls[32]==0x614EE8,
          "real wrapper visits exactly eight records then explicit unrecovered post-pool boundary");
    for(i=0;i<8;i++) check(pool[i].f190==1 && c.ports[i*4]==(s32)i,"each distinct input pool record is processed in order");
    setup(&h,pool,&c); missing.after_pool=NULL; h.platform=&missing;
    check(dreadzone_poll_pool(&h)==DREADZONE_POLL_NEEDS_POST_POOL && c.count==0,"missing post-pool implementation is explicit before any processing");
    missing=platform; missing.read_packet=NULL; h.platform=&missing;
    check(dreadzone_poll_record(&h,0)==DREADZONE_POLL_NEEDS_PLATFORM && c.count==0,"missing platform prerequisite does not invent driver effects");
    h.platform=&platform; put(pool,0x56C,0x123456);
    check(dreadzone_poll_record(&h,0)==DREADZONE_POLL_NEEDS_CALLBACK && c.count==0,"unknown guest callback is never reinterpreted as host pointer");
    put(pool,0x56C,0x6B5CB8); put(pool,0x570,0x36BF4C);
    check(dreadzone_poll_record(&h,0)==DREADZONE_POLL_NEEDS_CONTEXT && c.count==0,"real callback requires explicitly mapped context storage");
    check(dreadzone_poll_record(&h,8)==DREADZONE_POLL_INVALID_STORAGE && c.count==0,"pool index bound is checked at host boundary");
    setup(&h,pool,&c); h.pool_guest_base=0xFFFFFFFFU-0x100;
    check(dreadzone_poll_record(&h,1)==DREADZONE_POLL_INVALID_STORAGE && c.count==0,"guest pool address addition cannot wrap");
    check(dreadzone_poll_record(&h,0)==DREADZONE_POLL_INVALID_STORAGE && c.count==0,"complete guest record span cannot wrap");
    {
        DreadzoneInputContext1E0 context;
        DreadzonePollContextView views[2];
        setup(&h,pool,&c); put(pool,0x56C,0x6B5CB8); put(pool,0x570,0x36BF4C);
        views[0].guest_address=views[1].guest_address=0x36BF4C;
        views[0].storage=views[1].storage=&context;
        h.contexts=views; h.context_count=2;
        check(dreadzone_poll_record(&h,0)==DREADZONE_POLL_INVALID_STORAGE && c.count==0,
              "ambiguous context address is rejected before any platform effects");
        h.context_count=1; views[0].guest_address=0xFFFFFFF0; put(pool,0x570,0xFFFFFFF0);
        check(dreadzone_poll_record(&h,0)==DREADZONE_POLL_INVALID_STORAGE && c.count==0,
              "complete guest context span cannot wrap");
    }
}

static void test_primary_mode_flag(void) {
    DreadzonePollContext h; InputRecord5C0 pool[8]; Capture c;
    unsigned mode_a,mode_b,primary,state;
    for(mode_a=0;mode_a<2;mode_a++) for(mode_b=0;mode_b<2;mode_b++)
        for(primary=0;primary<2;primary++) for(state=0;state<2;state++) {
            setup(&h,pool,&c); h.mode_0021E824=(s32)mode_a; h.mode_0021E828=(s32)mode_b;
            h.primary_0021DDCC=h.pool_guest_base+(primary?0:0x5C0); h.flag_0021DA60=0x55;
            put(pool,0x198,state); c.modes[0]=4;
            check(dreadzone_poll_record(&h,0)==0,"primary input mode case executes recovered path");
            check(h.flag_0021DA60==(primary && !mode_a && !mode_b?(state==0?1:0):0x55),
                  "actual flag21DA60 changes only for matching guest input and zero modes");
        }
}

static void test_configured_repeat_pipeline(void) {
    DreadzonePollContext h; InputRecord5C0 pool[8]; Capture c;
    DreadzoneActorInit31F0 actor;
    DreadzoneRepeatContext repeat;
    DreadzoneRepeatInputView input;
    DreadzonePollContextView context;
    unsigned frame;
    u32 guest_input=0x1EE600;
    setup(&h,pool,&c); memset(&actor,0xA5,sizeof(actor)); func_L01_006A6598(&actor);
    memcpy((u8 *)&actor+0x2F00,&guest_input,4);
    context.guest_address=0x368F60+0x2FEC; context.storage=&actor.fields.f2FEC;
    h.contexts=&context; h.context_count=1;
    put(pool,0x56C,0x6B5CB8); put(pool,0x570,context.guest_address);
    memset(&repeat,0,sizeof(repeat)); repeat.player_count=1; repeat.actors=&actor; repeat.actor_count=1;
    input.guest_address=guest_input; input.storage=pool; repeat.inputs=&input; repeat.input_count=1;
    repeat.mask_0021F580=0x800; c.repeat=&repeat;
    /* Real mode7 configuration sequence, followed by its state99 packet path.
     * Only the driver calls are captures. Context reset/callback/decoder/repeat
     * execute their recovered C, including the actual eight-record wrapper. */
    c.modes[0]=c.modes[1]=7;
    c.select=c.align=c.pressure=c.query_pressure=1;
    check(dreadzone_poll_record(&h,0)==0 && get(pool,0x198)==70,"real configuration begins at mode7 state70");
    c.count=0;
    check(dreadzone_poll_record(&h,0)==0 && get(pool,0x198)==71,"alignment advances to real request state71");
    c.count=0; c.request_count=0; c.request[0]=c.request[1]=0;
    check(dreadzone_poll_record(&h,0)==0 && get(pool,0x198)==72,"request completion advances to state72");
    c.count=0;
    check(dreadzone_poll_record(&h,0)==0 && get(pool,0x198)==76,"pressure enable advances to state76");
    c.count=0;
    check(dreadzone_poll_record(&h,0)==0 && get(pool,0x198)==77,"pressure query advances to state77");
    c.count=0; c.request_count=0;
    check(dreadzone_poll_record(&h,0)==0 && get(pool,0x198)==78,"request completion advances to state78");
    c.count=0;
    check(dreadzone_poll_record(&h,0)==0 && get(pool,0x198)==99,"completed configuration selects the packet state99");
    for(frame=0;frame<28;frame++) {
        c.count=0; c.packet[2]=frame<26?0xF7:0xFF;
        /* All other records are configured; the wrapper really polls them. */
        for(unsigned i=1;i<8;i++) put(&pool[i],0x198,99);
        check(dreadzone_poll_pool(&h)==0,"configured pool executes real callback decoding and post-pool repetition");
        check(pool[0].f1C0==(frame<26?0x800:0),"actual held mask reaches real repetition pass");
        check(pool[0].f1C4==(frame==0||frame==20||frame==25?0x800:0),
              "full recovered input chain emits initial press then repeat at frames20 and25");
        check(actor.fields.f2FEC.f1D8==(s32)frame+1 && c.post_count==frame+1,
              "real callback and repeat pass advance once per pool frame");
    }
}

int main(void) {
    test_default_state_table(); test_configuration(); test_actual_callback_dispatch();
    test_failure_and_fallback(); test_pool_and_boundaries(); test_primary_mode_flag();
    test_configured_repeat_pipeline();
    printf("Dreadzone recovered input polling with captured platform boundaries: %u checks passed.\n",checks);
    return 0;
}
