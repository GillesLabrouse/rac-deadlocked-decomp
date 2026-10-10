/* cflags: -mno-split-addresses */
/* Actual L01 controller state machine, 005BE338..005BE7B0 (1144 bytes).
 * NONMATCHING. The 79-entry table at003AD890 has ten distinct destinations.
 * Native execution requires explicit platform implementations and maps only
 * the verified real006B5CB8 callback. It provides no controller driver.
 */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_input_poll.h"

static s32 poll_read_word(InputRecord5C0 *record, u32 offset) {
    const u8 *p = (const u8 *)record + offset;
    return (s32)((u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24));
}
static void poll_write_word(InputRecord5C0 *record, u32 offset, s32 value) {
    u8 *p = (u8 *)record + offset;
    u32 bits = (u32)value;
    p[0] = (u8)bits; p[1] = (u8)(bits >> 8);
    p[2] = (u8)(bits >> 16); p[3] = (u8)(bits >> 24);
}
#define RW(offset) poll_read_word(record, offset)
#define SW(offset, value) poll_write_word(record, offset, value)
#define MODE_A host->mode_0021E824
#define MODE_B host->mode_0021E828
#define PRIMARY (host->primary_0021DDCC == host->pool_guest_base + index * 0x5C0)
#define PRIMARY_FLAG host->flag_0021DA60
#define CORE_STATE(p,s) api->query_state(user,p,s)
#define CORE_READ(p,s,b) api->read_packet(user,p,s,b)
#define CORE_MODES(p,s,c,i) api->query_modes(user,p,s,c,i)
#define CORE_MODE(p,s,m,l) api->request_mode(user,p,s,m,l)
#define CORE_REQUEST(p,s) api->request_state(user,p,s)
#define CORE_SELECT(p,s,m,l) api->select_config(user,p,s,m,l)
#define CORE_ALIGN(p,s,b) api->set_alignment(user,p,s,b)
#define CORE_PRESSURE(p,s) api->request_pressure(user,p,s)
#define CORE_QUERY_PRESSURE(p,s) api->query_pressure(user,p,s)
#define CORE_HINT(p) api->port_hint(user,p)
#define CORE_FEEDBACK(p,s,b) api->feedback(user,p,s,b)
#define POLL_RETURN return DREADZONE_POLL_COMPLETE
static const u8 D_003B8340[32] = {
    0, 0x79, 255, 255, 128, 128, 128, 128, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
#else
typedef struct InputRecord5C0 { u8 bytes[0x5C0]; } InputRecord5C0;
extern const u8 D_003B8340[];
extern s32 D_0021E824, D_0021E828;
extern InputRecord5C0 *D_0021DDCC;
extern u8 D_0021DA60;
s32 func_0013BA98(s32, s32);
s32 func_0013BA20(s32, s32, u8 *);
s32 func_0013BD50(s32, s32, s32, s32);
s32 func_0013BE88(s32, s32, s32, s32);
s32 func_0013BBD0(s32, s32);
s32 func_0013BC30(s32, s32, s32, s32);
s32 func_0013C008(s32, s32, const u8 *);
s32 func_0013C190(s32, s32);
s32 func_0013C210(s32, s32);
s32 func_0013B0F8(s32);
s32 func_0013BF40(s32, s32, const u8 *);
void func_L01_005BDA40(InputRecord5C0 *, const u8 *, s32);
#define RW(offset) (*(s32 *)((u8 *)record + (offset)))
#define SW(offset, value) (RW(offset) = (value))
#define MODE_A D_0021E824
#define MODE_B D_0021E828
#define PRIMARY (record == D_0021DDCC)
#define PRIMARY_FLAG D_0021DA60
#define CORE_STATE func_0013BA98
#define CORE_READ func_0013BA20
#define CORE_MODES func_0013BD50
#define CORE_MODE func_0013BE88
#define CORE_REQUEST func_0013BBD0
#define CORE_SELECT func_0013BC30
#define CORE_ALIGN func_0013C008
#define CORE_PRESSURE func_0013C190
#define CORE_QUERY_PRESSURE func_0013C210
#define CORE_HINT func_0013B0F8
#define CORE_FEEDBACK func_0013BF40
#define POLL_RETURN return
#endif

#define RB(offset) (*((u8 *)record + (offset)))
typedef char PollRecordSize[sizeof(InputRecord5C0) == 0x5C0 ? 1 : -1];

#ifdef DEADLOCKED_NATIVE
int dreadzone_poll_record(DreadzonePollContext *host, u32 index) {
    const DreadzonePollPlatform *api;
    void *user;
    InputRecord5C0 *record;
    DreadzoneInputContext1E0 *context = NULL;
    u32 callback, i, address;
    if (!host || !host->pool || index >= host->pool_count || index > 0xFFFFFFFFU / 0x5C0 ||
        host->pool_guest_base > 0xFFFFFFFFU - index * 0x5C0)
        return DREADZONE_POLL_INVALID_STORAGE;
    address = host->pool_guest_base + index * 0x5C0;
    if (address > 0xFFFFFFFFU - (0x5C0 - 1)) return DREADZONE_POLL_INVALID_STORAGE;
    record = &host->pool[index];
    api = host->platform;
    user = host->platform_context;
    if (!api || !api->query_state || !api->read_packet || !api->query_modes ||
        !api->request_mode || !api->request_state || !api->select_config ||
        !api->set_alignment || !api->request_pressure || !api->query_pressure ||
        !api->port_hint || !api->feedback) return DREADZONE_POLL_NEEDS_PLATFORM;
    callback = (u32)RW(0x56C);
    if (callback != 0) {
        if (callback != 0x006B5CB8) return DREADZONE_POLL_NEEDS_CALLBACK;
        if (host->context_count != 0 && !host->contexts) return DREADZONE_POLL_INVALID_STORAGE;
        for (i = 0; i < host->context_count; i++) {
            if (host->contexts[i].guest_address == (u32)RW(0x570)) {
                if (context || !host->contexts[i].storage || host->contexts[i].guest_address >
                    0xFFFFFFFFU - (sizeof(*context) - 1)) return DREADZONE_POLL_INVALID_STORAGE;
                context = host->contexts[i].storage;
            }
        }
        if (!context) return DREADZONE_POLL_NEEDS_CONTEXT;
    }
#else
void func_L01_005BE338(InputRecord5C0 *record) {
#endif
    s32 port, slot, state, result, length, j;
    const u8 *sample;
    SW(0x1A0, 0);
    state = RW(0x1B0);
    port = RB(0x54C);
    slot = RB(0x558);
    SW(0x1AC, RW(0x348));
    SW(0x1B0, 0);
    SW(0x1BC, state);
    result = CORE_STATE(port, slot);
    SW(0x19C, result);
    if (result == 0) { SW(0x198, 0); RB(0x551) = 1; }
    if (MODE_A == 0 && MODE_B == 0 && PRIMARY) PRIMARY_FLAG = 0;
    state = RW(0x198);
    /* The retail unsigned table guard branches to the packet path for states
     * outside 0..78, including the configured state99 and negative states. */
    switch (state) {
    case 0:
        if (MODE_A == 0 && MODE_B == 0 && PRIMARY) PRIMARY_FLAG = 1;
        if (RW(0x19C) != 6 && RW(0x19C) != 2) {
            sample = D_003B8340;
            length = 32;
            goto neutral;
        }
        result = CORE_MODES(port, slot, 1, 0);
        SW(0x560, result);
        if (result == 0) { POLL_RETURN; }
        result = CORE_MODES(port, slot, 2, 0);
        SW(0x564, result);
        if (result > 0) SW(0x560, result);
        SW(0x55C, 0);
        SW(0x198, RW(0x560) == 4 ? 40 : RW(0x560) == 7 ? 70 : 99);
        POLL_RETURN;
    case 40:
        if (CORE_MODES(port, slot, 2, 0) == 0) { SW(0x198, 99); POLL_RETURN; }
        SW(0x198, RW(0x198) + 1);
        /* Actual state40 falls through directly to state41's request. */
    case 41:
        if (CORE_MODE(port, slot, 1, 3) == 1) SW(0x198, RW(0x198) + 1);
        POLL_RETURN;
    case 42:
        if (CORE_REQUEST(port, slot) == 1) SW(0x198, RW(0x198) - 1);
        if (CORE_REQUEST(port, slot) == 0) SW(0x198, 0);
        POLL_RETURN;
    case 70:
        if (CORE_SELECT(port, slot, -1, 0) == 0) SW(0x198, 99);
        RB(0x553) = 1;
        RB(0x552) = 0;
        for (j = 2; j < 6; j++) RB(0x552 + j) = 255;
        if (CORE_ALIGN(port, slot, (u8 *)record + 0x552) != 0) SW(0x198, RW(0x198) + 1);
        POLL_RETURN;
    case 71:
    case 77:
        if (CORE_REQUEST(port, slot) == 1) SW(0x198, RW(0x198) - 1);
        if (CORE_REQUEST(port, slot) == 0) SW(0x198, RW(0x198) + 1);
        POLL_RETURN;
    case 72:
        if (CORE_PRESSURE(port, slot) != 0) SW(0x198, 76);
        else { SW(0x55C, 0); SW(0x198, 99); }
        POLL_RETURN;
    case 76:
        if (CORE_QUERY_PRESSURE(port, slot) == 0) { SW(0x198, 72); POLL_RETURN; }
        SW(0x198, RW(0x198) + 1);
        POLL_RETURN;
    case 78:
        SW(0x198, 99);
        POLL_RETURN;
    default:
        break;
    }
    if (RB(0x559) == 0) RB(0x550) = (u8)CORE_HINT(port);
    RB(0x559) = 1;
    if (RW(0x19C) != 6 && RW(0x19C) != 2) { POLL_RETURN; }
    if (CORE_READ(port, slot, (u8 *)record + 0x574) == 0) { POLL_RETURN; }
    RB(0x551) = 0;
    if (RB(0x574) == 0) {
        sample = (u8 *)record + 0x574;
        length = 2 + 2 * (RB(0x575) & 15);
        if (length > 20) length = 20;
#ifdef DEADLOCKED_NATIVE
        if (callback != 0) func_L01_006B5CB8(record, sample, length, context);
#else
        if (RW(0x56C) != 0)
            ((void (*)(InputRecord5C0 *, const u8 *, s32, void *))RW(0x56C))
                (record, sample, length, (void *)RW(0x570));
#endif
        func_L01_005BDA40(record, sample, length);
        SW(0x1DC, RB(0x575));
    }
    if (RW(0x1DC) > 0 && RW(0x55C) != 0 && RW(0x1DC) != RW(0x55C)) {
        SW(0x55C, 0);
        SW(0x198, 0);
    } else if (RW(0x1DC) > 0) SW(0x55C, RW(0x1DC));
    CORE_FEEDBACK(port, slot, (u8 *)record + 0x188);
    RB(0x189) = RB(0x188) = 0;
    POLL_RETURN;

neutral:
#ifdef DEADLOCKED_NATIVE
    if (callback != 0) func_L01_006B5CB8(record, sample, length, context);
#else
    if (RW(0x56C) != 0)
        ((void (*)(InputRecord5C0 *, const u8 *, s32, void *))RW(0x56C))
            (record, sample, length, (void *)RW(0x570));
#endif
    func_L01_005BDA40(record, sample, length);
    RB(0x559) = 0;
    POLL_RETURN;
}
