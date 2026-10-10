#include "dreadzone_animation_clock.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static uint32_t read32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
        ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static uint16_t read16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}
static float as_float(uint32_t bits) {
    float result;
    memcpy(&result, &bits, sizeof(result));
    return result;
}
static uint32_t float_bits(float value) {
    uint32_t result;
    memcpy(&result, &value, sizeof(result));
    return result;
}
static void write32(volatile uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}
/* All operands in this entry are finite and bounded. A double represents a
 * product of two binary32 operands exactly. Narrow each separate EE operation
 * and correct a nearest host conversion that rounded away from zero. This is
 * an explicit finite arithmetic approximation, not a physical EE emulator. */
static float chop(double value) {
    float narrowed = (float)value;
    uint32_t bits = float_bits(narrowed);
    if (fabs((double)narrowed) > fabs(value)) {
        narrowed = as_float(bits - 1U);
    }
    return narrowed;
}
static float fraction(float value) {
    float accumulator = chop(-12582912.0 - (double)value);
    accumulator = chop((double)accumulator - (-12582912.0));
    return chop((double)accumulator + (double)value);
}
static int host_span(const void *pointer, uint32_t bytes, uintptr_t *start) {
    *start = (uintptr_t)pointer;
    return pointer != NULL && bytes <= UINTPTR_MAX - *start;
}
static int guest_span(uint32_t address, uint32_t bytes) {
    return bytes != 0 && (uint64_t)address + bytes <= UINT64_C(0x100000000);
}

int dreadzone_animation_clock_advance(
    const DreadzoneAnimationClockRequest *request) {
    DreadzoneAnimationClockRequest input;
    uintptr_t actor_start, definition_start;
    uint32_t count, index, initial_duration, duration, rate_bits;
    int32_t elapsed;
    float frame, rate, subframe, remainder, elapsed_float, updated;
    uint8_t flags;
    volatile uint8_t *publication;
    uint32_t steps = 0;

    if (request == NULL) return DREADZONE_ANIMATION_CLOCK_INVALID;
    /* Capture the descriptor before writes; it may reside in output backing. */
    input = *request;
    if (input.actor_bytes < 0x56 || input.definition_bytes < 0x24 ||
        !host_span(input.actor, input.actor_bytes, &actor_start) ||
        !host_span(input.definition, input.definition_bytes, &definition_start) ||
        !guest_span(input.actor_guest, input.actor_bytes) ||
        !guest_span(input.definition_guest, input.definition_bytes) ||
        (input.actor_guest & 3U) != 0 || (input.definition_guest & 3U) != 0 ||
        (actor_start < definition_start + input.definition_bytes &&
         definition_start < actor_start + input.actor_bytes) ||
        ((uint64_t)input.actor_guest < (uint64_t)input.definition_guest + input.definition_bytes &&
         (uint64_t)input.definition_guest < (uint64_t)input.actor_guest + input.actor_bytes) ||
        read32(input.actor + 0x40) != input.definition_guest) {
        return DREADZONE_ANIMATION_CLOCK_INVALID;
    }
    if (read32(input.actor + 0x50) != 0) {
        return DREADZONE_ANIMATION_CLOCK_UNSUPPORTED;
    }
    count = input.definition[0x10];
    if (count == 0 || input.definition_bytes < 0x20U + count * 4U) {
        return DREADZONE_ANIMATION_CLOCK_INVALID;
    }
    frame = as_float(read32(input.actor + 0x44));
    rate_bits = read32(input.actor + 0x48);
    rate = as_float(rate_bits);
    initial_duration = read16(input.actor + 0x4C);
    if (!isfinite(frame) || !isfinite(rate) || frame < 0 || frame >= (float)count ||
        fabs((double)rate) > 1048576.0 || initial_duration == 0 || initial_duration > 32767) {
        return DREADZONE_ANIMATION_CLOCK_UNSUPPORTED;
    }
    for (index = 0; index < count; ++index) {
        duration = read16(input.definition + 0x20U + index * 4U);
        if (duration == 0 || duration > 32767) {
            return DREADZONE_ANIMATION_CLOCK_UNSUPPORTED;
        }
    }
    if (rate_bits == 0) return DREADZONE_ANIMATION_CLOCK_APPROXIMATE;
    index = (uint32_t)frame; /* Bounded nonnegative CVT.W.S domain. */
    subframe = fraction(frame);
    elapsed_float = chop((double)rate + (double)chop(
        (double)subframe * (double)(int32_t)initial_duration));
    elapsed = (int32_t)elapsed_float;
    /* A negative fractional operand near the large extraction constant
     * depends on the physical EE adder's lost alignment bits. Do not turn
     * that missing hardware proof into a plausible but reversed advance. */
    if (elapsed_float < 0 && elapsed_float != (float)elapsed) {
        return DREADZONE_ANIMATION_CLOCK_UNSUPPORTED;
    }
    remainder = fraction(elapsed_float);
    flags = (uint8_t)((input.actor[0x55] & 0xFD) | 0x20);
    duration = read16(input.definition + 0x20U + index * 4U);
    if ((rate_bits & UINT32_C(0x80000000)) == 0) {
        if ((flags & 0x10) != 0) {
            while (index != count - 1U && elapsed >= (int32_t)duration) {
                elapsed -= (int32_t)duration;
                ++index;
                duration = read16(input.definition + 0x20U + index * 4U);
            }
            if (index == count - 1U) {
                flags |= 2; elapsed = 0; remainder = 0;
            }
        } else {
            while (elapsed >= (int32_t)duration) {
                if (++steps > 1048576U) return DREADZONE_ANIMATION_CLOCK_UNSUPPORTED;
                elapsed -= (int32_t)duration;
                if (index == count - 1U) { flags |= 2; index = 0; }
                else ++index;
                duration = read16(input.definition + 0x20U + index * 4U);
            }
        }
    } else {
        if ((float_bits(remainder) & UINT32_C(0x80000000)) != 0) {
            --elapsed;
            remainder = chop((double)remainder + 1.0);
        }
        if ((flags & 0x10) != 0) {
            while (elapsed < 0 && index != 0) {
                --index;
                /* Original reverse clamp uses the OLD cursor's duration. */
                elapsed += (int32_t)duration;
                duration = read16(input.definition + 0x20U + index * 4U);
            }
            if (elapsed < 0) { flags |= 2; elapsed = 0; remainder = 0; }
        } else {
            while (elapsed < 0) {
                if (++steps > 1048576U) return DREADZONE_ANIMATION_CLOCK_UNSUPPORTED;
                if (index == 0) { flags |= 2; index = count - 1U; }
                else --index;
                duration = read16(input.definition + 0x20U + index * 4U);
                elapsed += (int32_t)duration;
            }
        }
    }
    updated = chop((double)index + (double)chop(
        (double)chop((double)elapsed + (double)remainder) / (double)duration));
    /* No unresolved reads or checks remain after the first publication. */
    publication = input.actor;
    publication[0x55] = flags;
    publication[0x4C] = (uint8_t)duration;
    publication[0x4D] = (uint8_t)(duration >> 8);
    write32(publication + 0x44, float_bits(updated));
    return DREADZONE_ANIMATION_CLOCK_APPROXIMATE;
}
