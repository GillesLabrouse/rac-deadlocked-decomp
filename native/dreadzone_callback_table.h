#ifndef DEADLOCKED_DREADZONE_CALLBACK_TABLE_H
#define DEADLOCKED_DREADZONE_CALLBACK_TABLE_H
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DreadzoneCallbackTableMemory {
    uint8_t *bytes;
    uint32_t guest_base;
    size_t length;
} DreadzoneCallbackTableMemory;

/* Complete final-RAM effects of actual L01 006C1C00/CC.
 * Registers, original stack stores, timing and callback execution are outside
 * this API. Original reference comparisons use a separate disjoint stack;
 * this function does not emulate stack/table aliasing.
 * Values stored in the table are original guest code identifiers, not host
 * callable pointers. No callback/resource is created or invoked here.
 * The descriptor must be readable and disjoint from exclusive stable backing.
 * Returns 0 on success or -1 before mutation if the mapped table is invalid.
 */
int dreadzone_callback_table_006C1C00(const DreadzoneCallbackTableMemory *ram);
#ifdef __cplusplus
}
#endif
#endif
