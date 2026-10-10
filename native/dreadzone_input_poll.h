#ifndef DEADLOCKED_DREADZONE_INPUT_POLL_H
#define DEADLOCKED_DREADZONE_INPUT_POLL_H
#include "dreadzone_input_callback.h"

/* Explicit platform boundary, with signatures recovered from core binaries.
 * Implementations are not supplied here. Test captures are fixtures, not real
 * controller transport, configuration or feedback. The recovered button-repeat
 * pass can be explicitly bound through after_pool; fixtures label captures.
 */
typedef struct DreadzonePollPlatform {
    s32 (*query_state)(void *, s32, s32);                 /* 0013BA98 */
    s32 (*read_packet)(void *, s32, s32, u8 *);           /* 0013BA20 */
    s32 (*query_modes)(void *, s32, s32, s32, s32);       /* 0013BD50 */
    s32 (*request_mode)(void *, s32, s32, s32, s32);      /* 0013BE88 */
    s32 (*request_state)(void *, s32, s32);               /* 0013BBD0 */
    s32 (*select_config)(void *, s32, s32, s32, s32);     /* 0013BC30 */
    s32 (*set_alignment)(void *, s32, s32, const u8 *);   /* 0013C008, six bytes */
    s32 (*request_pressure)(void *, s32, s32);            /* 0013C190 */
    s32 (*query_pressure)(void *, s32, s32);              /* 0013C210 */
    s32 (*port_hint)(void *, s32);                       /* 0013B0F8 */
    s32 (*feedback)(void *, s32, s32, const u8 *);        /* 0013BF40, six bytes */
    void (*after_pool)(void *);                          /* 00614EE8 button repetition */
} DreadzonePollPlatform;

typedef struct DreadzonePollContextView {
    u32 guest_address;
    DreadzoneInputContext1E0 *storage;
} DreadzonePollContextView;

typedef struct DreadzonePollContext {
    InputRecord5C0 *pool;
    u32 pool_count;
    u32 pool_guest_base;
    const DreadzonePollContextView *contexts;
    u32 context_count;
    s32 mode_0021E824;
    s32 mode_0021E828;
    u32 primary_0021DDCC;
    u8 flag_0021DA60;
    const DreadzonePollPlatform *platform;
    void *platform_context;
} DreadzonePollContext;

enum DreadzonePollResult {
    DREADZONE_POLL_COMPLETE = 0,
    DREADZONE_POLL_INVALID_STORAGE = 1,
    DREADZONE_POLL_NEEDS_PLATFORM = 2,
    DREADZONE_POLL_NEEDS_CALLBACK = 3,
    DREADZONE_POLL_NEEDS_CONTEXT = 4,
    DREADZONE_POLL_NEEDS_POST_POOL = 5
};

/* Executes the actual recovered state machine after validating host mappings.
 * Pool and context initialization, decoder finite-domain/ring preconditions,
 * readable driver packet storage and nonoverflowing counters remain required.
 * Every guest callback/context field stays u32. Only verified006B5CB8 is bound.
 */
int dreadzone_poll_record(DreadzonePollContext *, u32 index);
int dreadzone_poll_pool(DreadzonePollContext *);

#endif
