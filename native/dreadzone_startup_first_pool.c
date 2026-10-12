#include "dreadzone_startup_pool.h"
#include <limits.h>
#include <string.h>

#define GUEST_LIMIT UINT64_C(0x80000000)
#define MAX_RAM UINT32_C(0x01000000)
#define MAX_COUNT UINT32_C(4096)

static int guest_span(const DreadzoneStartupPoolMemory *m, uint32_t address, size_t size, uint8_t **out) {
    uint64_t end = (uint64_t)address + (uint64_t)size;
    uint64_t mapped_end = (uint64_t)m->guest_base + (uint64_t)m->length;
    if (!size || address < m->guest_base || end > GUEST_LIMIT || end > mapped_end) return 0;
    *out = m->bytes + (size_t)(address - m->guest_base);
    return 1;
}

static uint32_t load_u32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void store_u32(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)value; p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16); p[3] = (uint8_t)(value >> 24);
}

static int overlaps(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1) {
    return a0 < b1 && b0 < a1;
}

int dreadzone_startup_first_pool_leaf(const DreadzoneStartupPoolMemory *request, uint32_t index) {
    static const uint32_t node_addresses[6] = {
        UINT32_C(0x00389C70), UINT32_C(0x00389C68), UINT32_C(0x00389C78),
        UINT32_C(0x00389C80), UINT32_C(0x00389C88), UINT32_C(0x00389C90)
    };
    DreadzoneStartupPoolMemory m;
    uintptr_t request_address, backing_address;
    uint32_t count_bits, stride_bits = 0, count, stride = 0, pool_address;
    static const uint32_t descriptor_addresses[6] = { UINT32_C(0x003B00E0), UINT32_C(0x003B00F8), UINT32_C(0x003B00C8), UINT32_C(0x003B15F0), UINT32_C(0x003B00B0), UINT32_C(0x003B1608) };
    uint32_t i, n;
    uint8_t *root_zero, *root_node, *pool_slot, *count_ptr, *stride_ptr = NULL;
    uint8_t *records = NULL;
    uint64_t record_end = 0, metadata[5];

    if (!request || index < 1 || index > 6) return -1;
    request_address = (uintptr_t)(const void *)request;
    if (request_address > UINTPTR_MAX - sizeof(m)) return -1;
    memcpy(&m, request, sizeof(m));
    if (!m.bytes || !m.length || m.length > MAX_RAM || m.guest_base >= GUEST_LIMIT ||
        (uint64_t)m.guest_base + (uint64_t)m.length > GUEST_LIMIT) return -1;
    backing_address = (uintptr_t)(void *)m.bytes;
    if (backing_address > UINTPTR_MAX - m.length ||
        overlaps((uint64_t)request_address, (uint64_t)request_address + sizeof(m),
                 (uint64_t)backing_address, (uint64_t)backing_address + m.length)) return -1;

    n = index - 1;
    if (!guest_span(&m, UINT32_C(0x00368B78) + 4u * index, 4, &root_zero) ||
        !guest_span(&m, UINT32_C(0x00368B78) + 4u * index + 0x1Cu, 4, &root_node) ||
        !guest_span(&m, UINT32_C(0x00368BB0) + 4u * index, 4, &pool_slot) ||
        !guest_span(&m, UINT32_C(0x003B0048) + 4u * index, 4, &count_ptr)) return -1;
    count_bits = load_u32(count_ptr);
    count = count_bits;
    if ((int32_t)count <= 0) {
        /* Retail BLEZ delay slot still publishes the pool pointer. */
        store_u32(root_zero, 0);
        store_u32(root_node, node_addresses[n]);
        store_u32(pool_slot, node_addresses[n]);
        return 0;
    }
    if (count > MAX_COUNT ||
        !guest_span(&m, UINT32_C(0x003B0110) + 4u * index, 4, &stride_ptr)) return -1;
    stride_bits = load_u32(stride_ptr);
    stride = stride_bits;
    if ((int32_t)stride < 0 || (stride & 3u)) return -1;
    pool_address = node_addresses[n];
    record_end = (uint64_t)pool_address + (uint64_t)(count - 1u) * stride + 0x10u;
    if (record_end > GUEST_LIMIT ||
        !guest_span(&m, pool_address, (size_t)(record_end - pool_address), &records)) return -1;
    metadata[0] = UINT32_C(0x00368B78) + 4u * index;
    metadata[1] = metadata[0] + 0x1Cu;
    metadata[2] = UINT32_C(0x00368BB0) + 4u * index;
    metadata[3] = UINT32_C(0x003B0048) + 4u * index;
    metadata[4] = UINT32_C(0x003B0110) + 4u * index;
    for (i = 0; i < 5; i++) {
        if (overlaps(pool_address, record_end, metadata[i], metadata[i] + 4u)) return -1;
    }

    /* Preserve the original publication order; loop writes retain +0C,+08,+04 order. */
    store_u32(root_zero, 0);
    store_u32(root_node, node_addresses[n]);
    store_u32(pool_slot, node_addresses[n]);
    for (i = 0; i < count; i++) {
        uint32_t offset = i * stride;
        uint8_t *record = records + offset;
        store_u32(record + 0x0C, descriptor_addresses[n]);
        store_u32(record + 0x08, i == 0 ? 0 : pool_address + (i - 1u) * stride);
        store_u32(record + 0x04, i + 1u == count ? 0 : pool_address + (i + 1u) * stride);
    }
    return 0;
}
