#ifndef DEADLOCKED_DREADZONE_STARTUP_INITIALIZER_H
#define DEADLOCKED_DREADZONE_STARTUP_INITIALIZER_H
#include "dreadzone_startup_pool.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef DreadzoneStartupPoolMemory DreadzoneStartupInitializerMemory;

typedef struct DreadzoneStartupCallbackRecord {
    uint8_t reserved_00[0x18];
    uint32_t field_18;
    uint16_t field_1c;
    uint8_t reserved_1e[2];
    uint32_t field_20;
    uint32_t field_24;
    uint32_t field_28;
    uint8_t field_2c;
    uint8_t reserved_2d[3];
    uint32_t field_30;
    uint32_t field_34;
    uint32_t field_38;
    uint32_t field_3c;
    uint32_t field_40;
    uint32_t field_44;
    uint32_t field_48;
    uint32_t field_4c;
    uint8_t field_50;
    uint8_t field_51;
    uint8_t reserved_52[2];
} DreadzoneStartupCallbackRecord;

/* RAM-only effect of actual L01 006B4078, bound by the index-6
 * provider descriptor. Invocation and live selection are separate work. Requires one truthful, stable, exclusive writable
 * guest mapping below2GiB, at most16MiB, and a guest record address aligned
 * to four bytes. Host backing may have any byte alignment. The complete 0x54-byte record must map;
 * the mapping descriptor itself must not alias any backing byte. Validates
 * every condition before changing RAM. Existing unrelated record bytes remain
 * unchanged. No allocation, live object selection/provenance, callback ABI,
 * CPU-register/stack effect, or partial guest-fault behavior is supplied.
 * Return0 on the exact RAM field initialization, -1 on atomic refusal.
 */
int dreadzone_startup_initializer_006B4078(const DreadzoneStartupInitializerMemory *, uint32_t record_guest_address);
#ifdef __cplusplus
}
#endif
#endif
