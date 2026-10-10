/* cflags: -mno-split-addresses */
/* Real eight-record polling wrapper, 005BE7B0..005BE808 (88 bytes).
 * NONMATCHING. Button repetition00614EE8 requires explicit host state binding.
 */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_input_poll.h"
int dreadzone_poll_pool(DreadzonePollContext *host) {
    u32 index;
    int result;
    if (!host || !host->pool || host->pool_count < 8) return DREADZONE_POLL_INVALID_STORAGE;
    if (!host->platform || !host->platform->after_pool) return DREADZONE_POLL_NEEDS_POST_POOL;
    for (index = 0; index < 8; index++) {
        result = dreadzone_poll_record(host, index);
        if (result != DREADZONE_POLL_COMPLETE) return result;
    }
    host->platform->after_pool(host->platform_context);
    return DREADZONE_POLL_COMPLETE;
}
#else
typedef struct InputRecord5C0 { u8 bytes[0x5C0]; } InputRecord5C0;
extern InputRecord5C0 D_001EE600[8];
void func_L01_005BE338(InputRecord5C0 *);
void func_L01_00614EE8(void);
void func_L01_005BE7B0(void) {
    InputRecord5C0 *record = D_001EE600;
    InputRecord5C0 *end = record + 8;
    do {
        func_L01_005BE338(record);
        record++;
    } while ((s32)record < (s32)end);
    func_L01_00614EE8();
}
#endif
