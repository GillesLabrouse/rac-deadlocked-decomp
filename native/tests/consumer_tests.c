#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Boundary-capture fixture, not implementations of the game's transition
 * effects. Expected routing comes from the two resident 17-entry tables.
 */
typedef struct RequestObject { u8 pad0[0x2E20]; f32 f2E20; } RequestObject;
typedef struct RequestSnapshot { s32 state, parameter2, parameter3; } RequestSnapshot;
typedef struct TransitionState { u8 pad0[0x8E]; s16 f8E; } TransitionState;
typedef struct StartupItem { s32 value, kind; } StartupItem;
typedef struct StartupQueue {
    s32 f0, f4; StartupItem items[10]; s32 f58, f5C, f60;
    u8 f64, gap65[3]; s32 f68; u8 f6C;
} StartupQueue;

_Static_assert(offsetof(TransitionState, f8E) == 0x8E, "Transition flag offset");
_Static_assert(sizeof(StartupQueue) == 0x70, "Queue layout");

s32 D_0021DDB4_[1], D_0021EDC4_[1], D_0021EDD4_[1], D_0021EDD8_[1];
s32 D_0021DAE0, D_0021DE10, D_0021EDC0, D_0021EDC8, D_0021EDCC;
s32 D_0021ED20, D_0021F2E0, D_0021F2E4, D_0021F2E8, D_0021F2EC, D_0021F2F4;
s32 D_0021EEB4, D_0021FC90, D_00221E28, D_0023AB0C, D_00241B00;
s32 *D_0021DDA4;
u8 D_0021EE19, *D_0021F2F0;
f32 D_0021EE0C, D_002211D0, D_002211D4, D_002372F0;
s16 D_0020694C;
s32 D_00171FC8[6];
/* Four-byte alignment for observed s32 fields in the otherwise opaque block. */
_Alignas(4) u8 D_001C2040[0xDE00];
TransitionState D_00237000;
u8 D_003D7D70[256];
RequestObject *D_003367B8[11];
RequestSnapshot D_00270040[8];

extern void func_0047EFA8(void);
extern s32 func_0047F568(s32, s32, s32, s32, u8 *);
extern void func_0045C768(StartupQueue *);
extern s32 func_0045C850(StartupQueue *, s32);
extern s32 func_0045C990(StartupQueue *);

typedef struct Event { u32 address; s32 a, b, c; } Event;
static Event events[256];
static size_t event_count;
static s32 record_index;
static int cleanup_changes_request;
static void *transfer_descriptor;

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "Consumer check failed at line %d: %s\n", __LINE__, #condition); \
    fprintf(stderr, "state=%d pending=%d progress=%.9g events=%zu\n", \
        D_0021DDB4_[0], D_0021EDC4_[0], (double)D_0021EE0C, event_count); \
    exit(EXIT_FAILURE); } } while (0)

static void capture(u32 address, s32 a, s32 b, s32 c) {
    CHECK(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count++] = (Event){address, a, b, c};
}

#define CAPTURE0(name) void func_##name(void) { capture(0x##name, 0, 0, 0); }
#define CAPTURE1(name) void func_##name(s32 a) { capture(0x##name, a, 0, 0); }
#define CAPTURE2(name) void func_##name(s32 a, s32 b) { capture(0x##name, a, b, 0); }
CAPTURE0(00437DD0) CAPTURE0(00437DF0) CAPTURE0(00437E10) CAPTURE0(00437E18)
CAPTURE0(00447BA8) CAPTURE0(00459978) CAPTURE0(004D33C0)
CAPTURE0(004E49D0) CAPTURE0(004E4FA0) CAPTURE0(004E5C10)
CAPTURE0(004E7520) CAPTURE0(004EAE88)
CAPTURE1(00489458) CAPTURE1(00489C28) CAPTURE1(004D3060)
CAPTURE1(004D38F8) CAPTURE1(004E4DF0) CAPTURE1(004E5638)
CAPTURE1(004EAAE0) CAPTURE1(004EABC8)
CAPTURE2(004494E8) CAPTURE2(004599F8) CAPTURE2(00496EA8) CAPTURE2(004DCBA8)
void func_004DCC38(void) {
    capture(0x004DCC38, 0, 0, 0);
    if (cleanup_changes_request) D_0021EDC4_[0] = 5;
}
void func_0043A118(s32 a, s32 b, s32 c, f32 d, f32 e, f32 f, f32 g) {
    CHECK(a == 0 && b == 0 && c == 3);
    CHECK(d == 1.11f && e == 0.005f && f == 0.2f && g == 0.0f);
    CHECK(D_002372F0 == 0.62f);
    capture(0x0043A118, a, b, c);
}
s32 func_004E2AF8(s32 a) {
    capture(0x004E2AF8, a, 0, 0);
    return record_index;
}
s32 func_00163840(s32 a, s32 b, s32 c, s32 d, void *e, s32 f, s32 g) {
    CHECK(a == 0 && d == 1 && f == 0 && g == 0);
    transfer_descriptor = e;
    capture(0x00163840, a, b, c);
    return 1;
}
void *func_0011A518(void *destination, s32 value, u32 count) {
    return memset(destination, value, count);
}

static void reset(void) {
    static s32 owner;
    D_0021DDB4_[0] = 0; D_0021EDC4_[0] = -2;
    D_0021EDD4_[0] = 1; D_0021EDD8_[0] = 1;
    D_0021DAE0 = 3; D_0021EDC0 = -99; D_0021EDC8 = 0; D_0021EDCC = 7;
    D_0021F2E4 = 0; D_0021F2E0 = -1; D_0021F2E8 = 31; D_0021F2EC = 41;
    D_0021F2F0 = NULL; D_0021F2F4 = 0; D_0021ED20 = 0; D_0021EE19 = 0;
    D_0021EEB4 = 1; D_0021EE0C = 0; D_002211D0 = 0; D_002211D4 = 0;
    D_0021FC90 = 91; D_00221E28 = 0x5000; D_0023AB0C = 0x100000;
    D_0020694C = 0; owner = 2; D_0021DDA4 = &owner;
    D_0021DE10 = 1; D_00241B00 = 51;
    event_count = 0; record_index = 1; cleanup_changes_request = 0;
    transfer_descriptor = NULL;
    memset(D_001C2040, 0, sizeof(D_001C2040));
    memset(D_00171FC8, 0x5A, sizeof(D_00171FC8));
    memset(&D_00237000, 0, sizeof(D_00237000));
    memset(D_003D7D70, 0, sizeof(D_003D7D70));
    memset(D_003367B8, 0, sizeof(D_003367B8));
    memset(D_00270040, 0, sizeof(D_00270040));
}

static void test_idle_delay_and_completion(void) {
    u8 done = 0;
    reset(); D_0021EDD4_[0] = 0; D_0021DDB4_[0] = -1;
    D_0021F2E4 = 2;
    func_0047EFA8();
    CHECK(event_count == 1 && events[0].address == 0x004EAAE0);
    CHECK(D_0021EDC0 == -1 && D_0021F2E4 == 2);
    CHECK(func_0047F568(1, 1, 61, 71, &done) == 0);
    CHECK(D_00270040[0].state == -1 && D_0021F2F4 == 1);
    func_0047EFA8(); func_0047EFA8();
    CHECK(D_0021F2E4 == 0 && done == 0 && D_0021EDC4_[0] == 1);
    CHECK(event_count == 3 && D_0021F2E0 == -1);
    func_0047EFA8();
    CHECK(event_count == 5 && events[4].address == 0x004E4DF0 && events[4].a == 71);
    CHECK(done == 1 && D_0021DDB4_[0] == 1 && D_0021EDC4_[0] == -2);
    CHECK(D_0021F2F0 == NULL && D_0021F2E0 == 10);
}

static void test_old_state_table(void) {
    static const u32 cleanup[17] = {0,0x004E4FA0,0,0x004D33C0,0x00459978,0,0,
        0x004DCC38,0x004EAE88,0x00437DF0,0x00437E10,0,0,0,0,0x004E49D0,0};
    s32 state;
    for (state = -3; state <= 18; state++) {
        reset(); D_0021DDB4_[0] = state; D_0021EDC4_[0] = 5;
        func_0047EFA8();
        if (state >= 0 && state <= 16 && cleanup[state] != 0) {
            CHECK(event_count >= 1 && events[0].address == cleanup[state]);
            CHECK(event_count == (state == 1 ? 3U : state == 15 ? 2U : 1U));
        } else CHECK(event_count == 0);
        CHECK(D_0021DDB4_[0] == 5 && D_0021EDC4_[0] == -2 && D_0021EDC0 == state);
    }
    reset(); D_0021DDB4_[0] = 3; D_0021EDC4_[0] = 4;
    func_0047EFA8();
    CHECK(event_count == 1 && events[0].address == 0x004599F8);
    reset(); D_0021DDB4_[0] = 3; D_0021EDC4_[0] = 4; D_0021F2E8 = 7;
    func_0047EFA8();
    CHECK(event_count == 2 && events[0].address == 0x004D33C0);
    reset(); D_0021DDB4_[0] = 7; D_0021EDC4_[0] = 1; cleanup_changes_request = 1;
    func_0047EFA8();
    CHECK(event_count == 1 && D_0021DDB4_[0] == 5); /* Re-read after cleanup. */
}

static void test_new_state_table(void) {
    static const u32 entry[17] = {0,0x004E4DF0,0x004E5638,0,0x004599F8,0,0,
        0x004DCBA8,0x004EABC8,0x00437DD0,0x00437E18,0,0,0,0,0x004494E8,0x004D38F8};
    s32 request;
    for (request = -3; request <= 18; request++) {
        if (request == -2) continue; /* Idle sentinel, not a transition. */
        reset(); D_0021DDB4_[0] = 2; D_0021EDC4_[0] = request;
        func_0047EFA8();
        if (request >= 0 && request <= 16 && entry[request]) {
            CHECK(event_count == 1 && events[0].address == entry[request]);
        } else CHECK(event_count == 0);
        CHECK(D_0021DDB4_[0] == request && D_0021EDC4_[0] == -2);
    }
    reset(); D_0021EDC4_[0] = 7; D_0021F2E8 = 0;
    func_0047EFA8();
    CHECK(events[0].a == 91 && events[0].b == 41);
    /* State-three entry guards observed when leaving state four. */
    reset(); D_0021DDB4_[0] = 4; D_0021EDC4_[0] = 3; D_0021EDD4_[0] = 0;
    func_0047EFA8();
    CHECK(event_count == 2 && events[1].address == 0x00459978);
    reset(); D_0021DDB4_[0] = 4; D_0021EDC4_[0] = 3;
    D_0021EDD4_[0] = 0; D_0021ED20 = 7;
    func_0047EFA8();
    CHECK(event_count == 3 && events[2].address == 0x004D3060 && events[2].a == 31);
}

static void test_deferred_transition(void) {
    unsigned calls;
    u8 done = 0;
    reset();
    CHECK(func_0047F568(2, 2, 101, 0, &done) == 0);
    for (calls = 0; calls < 30 && D_0021EDC4_[0] != -2; calls++) {
        func_0047EFA8();
        if (done == 0) CHECK(D_0021DDB4_[0] == 0 && D_0021EDC4_[0] == 2);
    }
    CHECK(calls == 22 && done == 1 && D_0021DDB4_[0] == 2);
    CHECK(D_0021EE0C == 1.0f && D_002211D0 == 1.0f && D_002211D4 == 1.0f);
    CHECK(event_count == 4 && events[0].address == 0x00489458);
    CHECK(events[1].address == 0x00489C28 && events[2].address == 0x004E2AF8);
    CHECK(events[3].address == 0x004E5638 && events[3].a == 101);
    for (calls = 0; calls < 6; calls++) CHECK(D_00171FC8[calls] == 0);
    reset(); D_0021EDC4_[0] = 2; D_0021EE0C = 1.09f; D_0021EDC8 = 1;
    D_00237000.f8E = 1;
    func_0047EFA8();
    CHECK(D_0021EDC4_[0] == 2 && D_0021DDB4_[0] == 0 && D_0021EDCC == 0);
    CHECK(D_0021EE0C == 1.0f && event_count == 0);
    D_0021EDC8 = 0;
    for (calls = 0; calls < 4 && D_0021EDC4_[0] != -2; calls++) func_0047EFA8();
    CHECK(D_0021EDC4_[0] == -2 && event_count == 1);
}

static void test_transfer_offsets_and_queue(void) {
    StartupQueue queue;
    reset(); D_0021EDC4_[0] = 2; D_0020694C = 1;
    *(s32 *)(D_001C2040 + 0xDABC) = 43;
    *(s32 *)(D_001C2040 + 0xDAF0 + 0x14C) = 57;
    *(s32 *)(D_001C2040 + 0xDAF4 + 0x14C) = 1;
    func_0047EFA8();
    CHECK(event_count == 6 && events[3].address == 0x00489C28 && events[3].a == 1);
    CHECK(events[4].address == 0x004E7520 && events[5].address == 0x00163840);
    CHECK(events[5].b == 0x104000 && events[5].c == 100);
    CHECK(transfer_descriptor == D_001C2040 + 0xDAB8 && D_00221E28 == 0x4000);
    func_0047EFA8();
    CHECK(event_count == 6 && D_00221E28 == 0x4000); /* Initial work happens once. */
    reset(); func_0045C768(&queue);
    CHECK(func_0045C850(&queue, 23) == 1 && func_0045C990(&queue) == 1);
    CHECK(D_0021EDC4_[0] == 1 && D_0021F2EC == 23);
    func_0047EFA8();
    CHECK(D_0021DDB4_[0] == 1 && D_0021EDC4_[0] == -2);
    CHECK(func_0045C850(&queue, 101) == 1 && func_0045C990(&queue) == 1);
    CHECK(queue.f68 == 2 && D_0021EDC4_[0] == 2 && D_0021F2F4 == 1);
    func_0047EFA8();
    CHECK(D_0021DDB4_[0] == 2 && D_0021EDC4_[0] == -2);
    CHECK(events[event_count - 1].address == 0x004E5638);
}

int main(void) {
    test_idle_delay_and_completion();
    test_old_state_table();
    test_new_state_table();
    test_deferred_transition();
    test_transfer_offsets_and_queue();
    puts("Experimental consumer routing and queue integration checks passed.");
    return EXIT_SUCCESS;
}
