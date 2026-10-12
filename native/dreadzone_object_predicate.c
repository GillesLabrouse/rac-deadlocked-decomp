#include "dreadzone_object_predicate.h"
#include <limits.h>
#include <stddef.h>
#include <string.h>

static int overlap(uintptr_t a, size_t an, uintptr_t b, size_t bn) {
    return a < b + bn && b < a + an;
}

static int read_guest(const DreadzoneObjectPredicateRequest *r,
                      uint32_t address, uint32_t width, uint32_t *value) {
    uint32_t offset;
    uint32_t bits = 0;
    if ((width == 2u && (address & 1u)) ||
        (width == 4u && (address & 3u)) || address < r->guest_base)
        return 0;
    offset = address - r->guest_base;
    if (offset > r->storage_bytes || width > r->storage_bytes - offset)
        return 0;
    bits = r->storage[offset];
    if (width > 1u)
        bits |= (uint32_t)r->storage[offset + 1u] << 8;
    if (width > 2u)
        bits |= (uint32_t)r->storage[offset + 2u] << 16;
    if (width > 3u)
        bits |= (uint32_t)r->storage[offset + 3u] << 24;
    *value = bits;
    return 1;
}

static int add_guest(uint32_t base, uint32_t offset, uint32_t *address) {
    if (base > UINT32_MAX - offset)
        return 0;
    *address = base + offset;
    return 1;
}

static int in_signed_class_range(uint32_t raw_half) {
    int16_t value;
    uint16_t bits = (uint16_t)raw_half;
    memcpy(&value, &bits, sizeof(value));
    return value >= 0x1F4 && value <= 0x21C;
}

int dreadzone_object_predicate_00596E48(
    const DreadzoneObjectPredicateRequest *request,
    DreadzoneObjectPredicateInfo *info) {
    DreadzoneObjectPredicateRequest r;
    uint64_t end;
    uint32_t lower, upper, mode, object, kind, address;
    uint32_t result = 0;

    if (request == NULL || info == NULL ||
        (uintptr_t)request > UINTPTR_MAX - sizeof(r) ||
        (uintptr_t)info > UINTPTR_MAX - sizeof(*info))
        return DREADZONE_OBJECT_PREDICATE_INVALID;
    memcpy(&r, request, sizeof(r));
    if (overlap((uintptr_t)info, sizeof(*info), (uintptr_t)request,
                sizeof(r)) ||
        (r.storage != NULL && r.storage_bytes != 0 &&
         ((uintptr_t)r.storage > UINTPTR_MAX - r.storage_bytes ||
          overlap((uintptr_t)info, sizeof(*info), (uintptr_t)r.storage,
                  r.storage_bytes))))
        return DREADZONE_OBJECT_PREDICATE_INVALID;
    end = (uint64_t)r.guest_base + r.storage_bytes;
    if (r.storage == NULL || r.storage_bytes == 0 ||
        r.guest_base >= 0x80000000u || end > 0x80000000u)
        return DREADZONE_OBJECT_PREDICATE_INVALID;

    /* Null A0 is the original first early return; it reads no guest RAM. */
    if (r.packet_guest != 0) {
        if (!read_guest(&r, 0x00222D14u, 4u, &lower))
            return DREADZONE_OBJECT_PREDICATE_UNSUPPORTED;
        if (r.packet_guest < lower)
            goto publish;
        if (!read_guest(&r, 0x00222D30u, 4u, &upper))
            return DREADZONE_OBJECT_PREDICATE_UNSUPPORTED;
        if (r.packet_guest <= upper) {
            if (!read_guest(&r, 0x0021E824u, 4u, &mode))
                return DREADZONE_OBJECT_PREDICATE_UNSUPPORTED;
            if (mode == 1u) {
                if (!add_guest(r.packet_guest, 0x24u, &address) ||
                    !read_guest(&r, address, 4u, &object))
                    return DREADZONE_OBJECT_PREDICATE_UNSUPPORTED;
                /* Exact BEQL early return: null object does not read +0xBC. */
                if (object == 0)
                    goto publish;
                if (!add_guest(object, 0x46u, &address) ||
                    !read_guest(&r, address, 1u, &kind))
                    return DREADZONE_OBJECT_PREDICATE_UNSUPPORTED;
                if (kind == 9u) {
                    result = 1u;
                    goto publish;
                }
                /* BNE at 0x596EA8 returns false for every non-nine kind. */
                goto publish;
            }
            /* Only mode != 1 reaches the JAL and packet halfword delay slot. */
            if (!add_guest(r.packet_guest, 0xBCu, &address) ||
                !read_guest(&r, address, 2u, &kind))
                return DREADZONE_OBJECT_PREDICATE_UNSUPPORTED;
            result = (uint32_t)in_signed_class_range(kind);
        }
    }

publish:
    info->original_return = result;
    return DREADZONE_OBJECT_PREDICATE_COMPLETE;
}
