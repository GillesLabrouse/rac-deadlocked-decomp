#include "dreadzone_client_history_lookup.h"
#include <stddef.h>
#include <string.h>

int dreadzone_client_history_lookup_005353C8(const uint8_t *storage,
    uint32_t guest_base, uint32_t storage_bytes, uint32_t index,
    int32_t marker, uint32_t *selected)
{
    uint32_t table, choice = 0, i;
    uintptr_t begin = (uintptr_t)storage, output = (uintptr_t)selected;
    if (!storage || !selected || !storage_bytes || storage_bytes > 0x1000000U ||
        guest_base > 0x7FFFFFFFU || storage_bytes > 0x80000000U - guest_base)
        return -1;
    if (index >= 4)
        return -2;
    if (begin > UINTPTR_MAX - storage_bytes || output > UINTPTR_MAX - sizeof(*selected))
        return -1;
    if (output < begin + storage_bytes && begin < output + sizeof(*selected))
        return -1;
    table = 0x231AD0U + index * 0x2100U;
    if (guest_base > table || (uint64_t)guest_base + storage_bytes < (uint64_t)table + 0x2100U)
        return -1;
    for (i = 0; i < 48; i++) {
        const uint8_t *p = storage + (table + i * 0xB0U + 0x86U - guest_base);
        uint32_t half = (uint32_t)p[0] | ((uint32_t)p[1] << 8);
        int32_t key = (int32_t)half - (half & 0x8000U ? 65536 : 0);
        if (key == marker) {
            choice = table + i * 0xB0U;
            break;
        }
    }
    memcpy(selected, &choice, sizeof(choice));
    return 1;
}
