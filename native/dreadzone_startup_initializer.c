#include "dreadzone_startup_initializer.h"
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

_Static_assert(sizeof(DreadzoneStartupCallbackRecord) == 0x54, "record size");
_Static_assert(offsetof(DreadzoneStartupCallbackRecord, field_18) == 0x18, "field_18 offset");
_Static_assert(offsetof(DreadzoneStartupCallbackRecord, field_1c) == 0x1C, "field_1c offset");
_Static_assert(offsetof(DreadzoneStartupCallbackRecord, field_2c) == 0x2C, "field_2c offset");
_Static_assert(offsetof(DreadzoneStartupCallbackRecord, field_51) == 0x51, "field_51 offset");

static int request_overlaps_backing(uintptr_t request, uintptr_t backing, size_t length) {
    uintptr_t request_end;
    uintptr_t backing_end;
    if (request > UINTPTR_MAX - sizeof(DreadzoneStartupInitializerMemory) ||
        backing > UINTPTR_MAX - length) return 1;
    request_end = request + sizeof(DreadzoneStartupInitializerMemory);
    backing_end = backing + length;
    return request < backing_end && backing < request_end;
}

static void store16le(uint8_t *p, uint16_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

static void store32le(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

int dreadzone_startup_initializer_006B4078(const DreadzoneStartupInitializerMemory *input,
                                           uint32_t record_guest_address) {
    DreadzoneStartupInitializerMemory view;
    uintptr_t request;
    uintptr_t backing;
    uint64_t guest_end;
    uint64_t record_end;
    size_t offset;
    uint8_t *record;
    if (input == NULL) return -1;
    request = (uintptr_t)(const void *)input;
    if (request > UINTPTR_MAX - sizeof(view)) return -1;
    memcpy(&view, input, sizeof(view));
    if (view.bytes == NULL || view.length < sizeof(DreadzoneStartupCallbackRecord) ||
        view.length > 0x01000000u || view.guest_base >= 0x80000000u ||
        (view.guest_base & 3u) != 0u || (record_guest_address & 3u) != 0u) return -1;
    guest_end = (uint64_t)view.guest_base + (uint64_t)view.length;
    record_end = (uint64_t)record_guest_address + sizeof(DreadzoneStartupCallbackRecord);
    if (guest_end > 0x80000000u || record_guest_address < view.guest_base ||
        record_end > guest_end) return -1;
    backing = (uintptr_t)(void *)view.bytes;
    if (request_overlaps_backing(request, backing, view.length)) return -1;
    offset = (size_t)(record_guest_address - view.guest_base);
    record = view.bytes + offset;

    store32le(record + 0x4C, UINT32_MAX);
    record[0x51] = 1u;
    store32le(record + 0x18, 0u);
    store32le(record + 0x20, 0u);
    store16le(record + 0x1C, 0u);
    store32le(record + 0x24, UINT32_MAX);
    store32le(record + 0x28, UINT32_MAX);
    record[0x2C] = 0u;
    store32le(record + 0x30, 0u);
    store32le(record + 0x34, UINT32_MAX);
    store32le(record + 0x48, UINT32_MAX);
    store32le(record + 0x38, UINT32_MAX);
    store32le(record + 0x44, UINT32_MAX);
    store32le(record + 0x3C, UINT32_MAX);
    store32le(record + 0x40, UINT32_MAX);
    record[0x50] = 0u;
    return 0;
}
