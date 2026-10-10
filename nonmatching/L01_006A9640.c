/* Dreadzone's record reset and free-list initializer: 006A9640 remains
 * nonmatching (136/128 bytes). Helper 006A9AE0 matches its 88-byte extent,
 * including four trailing zero bytes, after actual L01 address resolution.
 * Native execution preserves every guest pointer as a 32-bit address and maps
 * only the known pool storage; no gameplay callback is replaced by a stub.
 */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_memory.h"
#else
typedef struct DreadzonePoolRecord {
    u32 f0, f4, f8;
    u32 fC[16];
    u32 f4C;
    s32 f50;
    u8 f54, f55;
    u8 gap56[2];
    u32 f58;
} DreadzonePoolRecord;
extern DreadzonePoolRecord D_00375720[256];
extern u32 D_00220828;
extern void func_L01_0055D2A8(u32 *, u32, s32);
#endif

#ifdef DEADLOCKED_NATIVE
#define MEMORY_FIELD(type, field) offsetof(type, field)
#else
#define MEMORY_FIELD(type, field) ((u32)&((type *)0)->field)
#endif
typedef char check_pool5C[sizeof(DreadzonePoolRecord) == 0x5C ? 1 : -1];
typedef char check_pool4C[MEMORY_FIELD(DreadzonePoolRecord, f4C) == 0x4C ? 1 : -1];
typedef char check_pool50[MEMORY_FIELD(DreadzonePoolRecord, f50) == 0x50 ? 1 : -1];
typedef char check_pool54[MEMORY_FIELD(DreadzonePoolRecord, f54) == 0x54 ? 1 : -1];
typedef char check_pool55[MEMORY_FIELD(DreadzonePoolRecord, f55) == 0x55 ? 1 : -1];
typedef char check_pool58[MEMORY_FIELD(DreadzonePoolRecord, f58) == 0x58 ? 1 : -1];
#undef MEMORY_FIELD

#ifdef DEADLOCKED_NATIVE
/* Actual word-filling behavior recovered from L01 0055D2A8. This initializer
 * calls it with an aligned destination and the positive byte count 0x40.
 */
void func_L01_0055D2A8(u32 *destination, u32 value, s32 bytes) {
    do {
        *destination++ = value;
        bytes -= 4;
    } while (bytes > 0);
}
#endif

void func_L01_006A9AE0(DreadzonePoolRecord *record) {
    record->f8 = 0;
    record->f4 = 0;
    record->f0 = 0;
    func_L01_0055D2A8(record->fC, 0xCDCDCDCDU, 0x40);
    record->f55 = 0;
    record->f4C = 0;
    record->f54 = 0;
    record->f50 = -1;
}

#ifdef DEADLOCKED_NATIVE
#define POOL_GUEST_BASE ((u32)DREADZONE_POOL_GUEST_BASE)
#define POOL_LINK_AT(address) (&D_00375720[((address) - POOL_GUEST_BASE - 0x58) / 0x5C].f58)
#else
#define POOL_GUEST_BASE ((u32)D_00375720)
#define POOL_LINK_AT(address) ((u32 *)(address))
#endif

void func_L01_006A9640(void) {
    DreadzonePoolRecord *record = D_00375720;
    DreadzonePoolRecord *end = record + 256;
    s32 remaining;
    u32 current;
    u32 next;
    do {
        func_L01_006A9AE0(record);
        record++;
#ifdef DEADLOCKED_NATIVE
    } while (record < end);
#else
    } while ((s32)record < (s32)end);
#endif
    next = POOL_GUEST_BASE + 0x5BA4;
    current = POOL_GUEST_BASE + 0x5BA0;
    remaining = 254;
    do {
        *POOL_LINK_AT(current) = next;
        remaining--;
        current -= 0x5C;
        next -= 0x5C;
    } while (remaining >= 0);
    D_00220828 = POOL_GUEST_BASE;
}
