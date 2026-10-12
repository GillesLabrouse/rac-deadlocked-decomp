#include "dreadzone_startup_pool.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define STARTUP_RAM_MAX ((size_t)0x01000000u)
#define GUEST_RAM_LIMIT UINT64_C(0x80000000)

static int ranges_overlap(uintptr_t a, size_t an, uintptr_t b, size_t bn) {
    return a < b + bn && b < a + an;
}

static void store_le32(uint8_t *bytes, size_t offset, uint32_t value) {
    bytes[offset + 0] = (uint8_t)value;
    bytes[offset + 1] = (uint8_t)(value >> 8);
    bytes[offset + 2] = (uint8_t)(value >> 16);
    bytes[offset + 3] = (uint8_t)(value >> 24);
}

static int mapped_offset(const DreadzoneStartupPoolMemory *memory,
                         uint32_t address, size_t size, size_t *offset) {
    uint64_t end = (uint64_t)address + (uint64_t)size;
    uint64_t mapped_end = (uint64_t)memory->guest_base + (uint64_t)memory->length;
    if (address < memory->guest_base || end > GUEST_RAM_LIMIT || end > mapped_end) {
        return 0;
    }
    *offset = (size_t)(address - memory->guest_base);
    return 1;
}

/* RAM effects only. The caller provides an exclusive, stable, writable linear
 * guest mapping; no live startup, BSS origin, CPU, register, or stack behavior
 * is claimed. All effects are staged so any refusal leaves caller RAM intact.
 */
int dreadzone_startup_compose_006669D8_ram_effects(
    const DreadzoneStartupPoolMemory *request) {
    DreadzoneStartupPoolMemory input, staged;
    uintptr_t request_address, backing_address, staged_address;
    size_t first_store, second_store;
    uint32_t pool_index;
    int status = -1;

    if (!request) return -1;
    request_address = (uintptr_t)(const void *)request;
    if (request_address > UINTPTR_MAX - sizeof(input)) return -1;
    memcpy(&input, request, sizeof(input));
    if (!input.bytes || !input.length || input.length > STARTUP_RAM_MAX ||
        input.guest_base >= GUEST_RAM_LIMIT ||
        (uint64_t)input.guest_base + (uint64_t)input.length > GUEST_RAM_LIMIT) {
        return -1;
    }

    backing_address = (uintptr_t)(void *)input.bytes;
    if (backing_address > UINTPTR_MAX - input.length ||
        ranges_overlap(request_address, sizeof(input), backing_address, input.length) ||
        !mapped_offset(&input, 0x00223E00u, 4, &first_store) ||
        !mapped_offset(&input, 0x00223E04u, 4, &second_store)) {
        return -1;
    }

    staged.bytes = (uint8_t *)malloc(input.length);
    if (!staged.bytes) return -1;
    staged.guest_base = input.guest_base;
    staged.length = input.length;
    staged_address = (uintptr_t)(void *)staged.bytes;
    if (staged_address > UINTPTR_MAX - staged.length ||
        ranges_overlap(staged_address, staged.length, backing_address, input.length) ||
        ranges_overlap(staged_address, staged.length, request_address, sizeof(input))) {
        free(staged.bytes);
        return -1;
    }
    memcpy(staged.bytes, input.bytes, input.length);

    /* Retail writes these in order; the second write occupies the first call's
     * delay slot and therefore precedes the first leaf's RAM effects. */
    store_le32(staged.bytes, first_store, UINT32_MAX);
    store_le32(staged.bytes, second_store, UINT32_MAX);

    for (pool_index = 1; pool_index <= 6; ++pool_index) {
        if (dreadzone_startup_first_pool_leaf(&staged, pool_index) != 0) goto done;
    }
    if (dreadzone_startup_pool_006670B8_index2(&staged) != 0 ||
        dreadzone_startup_pool_006671D0_index3(&staged) != 0 ||
        dreadzone_startup_pool_006672E8_index4(&staged) != 0 ||
        dreadzone_startup_pool_00667400_index5(&staged) != 0 ||
        dreadzone_startup_pool_00667518_index6(&staged) != 0) {
        goto done;
    }

    /* Commit only after every callee accepts the same evolving snapshot. */
    memcpy(input.bytes, staged.bytes, input.length);
    status = 0;

done:
    free(staged.bytes);
    return status;
}
