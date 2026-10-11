#include "dreadzone_actor_record.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

#define GUEST_LIMIT 0x80000000u
#define MAX_GUEST_RAM 0x01000000u
#define REGISTRY_SENTINEL 0x003C5E00u
#define DEFAULT_METADATA 0x003C6CACu
#define SENTINEL_METADATA 0x003B8320u
#define NORMAL_CLASS_TABLE 0x0026B2B0u
#define NORMAL_INDEX_TABLE 0x0026B1C0u
#define ALTERNATE_CLASS_TABLE 0x0026B490u
#define ALTERNATE_INDEX_TABLE 0x0026DF90u
#define GETTER_666140 0x00666140u

static int ranges_overlap(uintptr_t a, size_t a_size,
                         uintptr_t b, size_t b_size) {
    if (a_size > UINTPTR_MAX - a || b_size > UINTPTR_MAX - b) {
        return 1;
    }
    return a < b + b_size && b < a + a_size;
}

static int ram_has(const DreadzoneActorRecordRequest *r,
                   uint32_t address, uint32_t size) {
    uint64_t ram_end = (uint64_t)r->ram_base + r->ram_size;
    uint64_t access_end = (uint64_t)address + size;
    return size != 0 && address >= r->ram_base && access_end <= ram_end;
}

static int guest_address(uint32_t base, uint32_t offset, uint32_t size,
                         uint32_t *address) {
    uint64_t value = (uint64_t)base + offset;
    if (value >= GUEST_LIMIT || value + size > GUEST_LIMIT) return 0;
    *address = (uint32_t)value;
    return 1;
}

static int guest_u8(const DreadzoneActorRecordRequest *r,
                    uint32_t address, uint8_t *value) {
    if (!ram_has(r, address, 1)) return 0;
    *value = r->ram[address - r->ram_base];
    return 1;
}

static int guest_u16(const DreadzoneActorRecordRequest *r,
                     uint32_t address, uint16_t *value) {
    uint32_t offset;
    if ((address & 1u) || !ram_has(r, address, 2)) return 0;
    offset = address - r->ram_base;
    *value = (uint16_t)r->ram[offset] |
             (uint16_t)((uint16_t)r->ram[offset + 1] << 8);
    return 1;
}

static int guest_u32(const DreadzoneActorRecordRequest *r,
                     uint32_t address, uint32_t *value) {
    uint32_t offset;
    if ((address & 3u) || !ram_has(r, address, 4)) return 0;
    offset = address - r->ram_base;
    *value = (uint32_t)r->ram[offset] |
             ((uint32_t)r->ram[offset + 1] << 8) |
             ((uint32_t)r->ram[offset + 2] << 16) |
             ((uint32_t)r->ram[offset + 3] << 24);
    return 1;
}

DreadzoneActorRecordStatus dreadzone_actor_record_00667008(
    const DreadzoneActorRecordRequest *request,
    DreadzoneActorRecordInfo *result) {
    DreadzoneActorRecordRequest r;
    DreadzoneActorRecordInfo pending;
    uintptr_t result_addr, request_addr, ram_addr;
    uint32_t end, metadata, method, registry_word;
    uint32_t actor_class, selector, table_address, index_address;
    uint32_t actor_selector_address, actor_class_address, actor_payload_address;
    uint32_t payload_value_address, metadata_method_address;
    uint8_t selector_byte;
    uint32_t payload, raw_value;
    uint16_t class_value, table_class;
    uint8_t index;

    if (request == NULL || result == NULL) return DREADZONE_ACTOR_RECORD_INVALID;
    request_addr = (uintptr_t)request;
    result_addr = (uintptr_t)result;
    if (sizeof(*request) > UINTPTR_MAX - request_addr ||
        sizeof(*result) > UINTPTR_MAX - result_addr ||
        ranges_overlap(result_addr, sizeof(*result), request_addr, sizeof(*request))) {
        return DREADZONE_ACTOR_RECORD_INVALID;
    }
    /* Capture only after checking both host spans. Request may alias guest RAM. */
    r = *request;
    ram_addr = (uintptr_t)r.ram;
    if (r.ram == NULL || r.ram_size == 0 || r.ram_size > MAX_GUEST_RAM ||
        r.ram_base >= GUEST_LIMIT ||
        (uint64_t)r.ram_base + r.ram_size > GUEST_LIMIT ||
        r.ram_size > UINTPTR_MAX - ram_addr ||
        ranges_overlap(result_addr, sizeof(*result), ram_addr, r.ram_size)) {
        return DREADZONE_ACTOR_RECORD_INVALID;
    }

    end = (uint32_t)((uint64_t)r.ram_base + r.ram_size);
    if (r.ram_base < 0x70004000u && end > 0x70000000u) {
        return DREADZONE_ACTOR_RECORD_INVALID;
    }
    memset(&pending, 0, sizeof(pending));
    if (r.actor == 0) {
        pending.status = DREADZONE_ACTOR_RECORD_OK;
        *result = pending;
        return DREADZONE_ACTOR_RECORD_OK;
    }
    if (r.actor < r.ram_base || r.actor >= end || r.actor >= GUEST_LIMIT) {
        return DREADZONE_ACTOR_RECORD_UNSUPPORTED;
    }
    if (!guest_u32(&r, REGISTRY_SENTINEL, &registry_word)) {
        return DREADZONE_ACTOR_RECORD_UNSUPPORTED;
    }

    if (registry_word == UINT32_MAX) {
        metadata = SENTINEL_METADATA;
    } else {
        if (!guest_address(r.actor, 0x22u, 1, &actor_selector_address) ||
            !guest_address(r.actor, 0xBCu, 2, &actor_class_address) ||
            !guest_u8(&r, actor_selector_address, &selector_byte) ||
            !guest_u16(&r, actor_class_address, &class_value)) {
            return DREADZONE_ACTOR_RECORD_UNSUPPORTED;
        }
        selector = selector_byte;
        actor_class = (uint32_t)class_value;
        table_address = NORMAL_CLASS_TABLE + selector * 2u;
        index_address = NORMAL_INDEX_TABLE + selector;
        if (!guest_u16(&r, table_address, &table_class)) {
            return DREADZONE_ACTOR_RECORD_UNSUPPORTED;
        }
        if (actor_class == (uint32_t)table_class) {
            if (!guest_u8(&r, index_address, &index)) {
                return DREADZONE_ACTOR_RECORD_UNSUPPORTED;
            }
            metadata = DEFAULT_METADATA + (uint32_t)index * 32u;
        } else {
            table_address = ALTERNATE_CLASS_TABLE + selector * 2u;
            index_address = ALTERNATE_INDEX_TABLE + selector;
            if (!guest_u16(&r, table_address, &table_class)) {
                return DREADZONE_ACTOR_RECORD_UNSUPPORTED;
            }
            if (actor_class == (uint32_t)table_class) {
                if (!guest_u8(&r, index_address, &index)) {
                    return DREADZONE_ACTOR_RECORD_UNSUPPORTED;
                }
                metadata = DEFAULT_METADATA + (uint32_t)index * 32u;
            } else {
                metadata = DEFAULT_METADATA;
            }
        }
    }

    pending.metadata_address = metadata;
    if (!guest_address(metadata, 4u, 4, &metadata_method_address) ||
        !guest_u32(&r, metadata_method_address, &method)) {
        return DREADZONE_ACTOR_RECORD_UNSUPPORTED;
    }
    pending.method_address = method;
    if (method != GETTER_666140) {
        return DREADZONE_ACTOR_RECORD_UNSUPPORTED;
    }

    if (!guest_address(r.actor, 0xACu, 4, &actor_payload_address) ||
        !guest_u32(&r, actor_payload_address, &payload)) {
        return DREADZONE_ACTOR_RECORD_UNSUPPORTED;
    }
    if (payload != 0) {
        if (!guest_address(payload, 0xE0u, 4, &payload_value_address) ||
            !guest_u32(&r, payload_value_address, &raw_value)) {
            return DREADZONE_ACTOR_RECORD_UNSUPPORTED;
        }
        pending.value = (raw_value & 0x80000000u)
            ? (int64_t)raw_value - 0x100000000LL
            : (int64_t)raw_value;
    }
    pending.status = DREADZONE_ACTOR_RECORD_OK;
    *result = pending;
    return DREADZONE_ACTOR_RECORD_OK;
}

