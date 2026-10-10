#ifndef DREADZONE_ANIMATION_CLOCK_H
#define DREADZONE_ANIMATION_CLOCK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CPU memory effects of the genuine 589CE0 primary-layer path. The caller
 * supplies the selected animation and effective rate in actor+40/+48. This
 * function neither selects a clip nor assigns a wall-clock frequency.
 * Secondary layers require the actual cleanup/release subsystem and are
 * refused before mutation. Event dispatch and DMA are outside this entry. */
typedef struct DreadzoneAnimationClockRequest {
    uint8_t *actor;
    uint32_t actor_bytes;
    uint32_t actor_guest;
    const uint8_t *definition;
    uint32_t definition_bytes;
    uint32_t definition_guest;
} DreadzoneAnimationClockRequest;

enum DreadzoneAnimationClockStatus {
    DREADZONE_ANIMATION_CLOCK_INVALID = -1,
    DREADZONE_ANIMATION_CLOCK_UNSUPPORTED = -2,
    /* Finite chopped binary32 model; physical EE multiplier/divider edge
     * behavior remains approximate. Successful CPU publication only. */
    DREADZONE_ANIMATION_CLOCK_APPROXIMATE = 1
};

/* Supported domain: aligned guest actor/definition, 1..255 frame records,
 * finite frame in [0,count), durations 1..32767, effective finite rate within
 * +/-1048576, and actor.word50==0. Negative fractional elapsed values require
 * the physical EE adder's operand-alignment rule and are refused; negative
 * integral elapsed values and nonnegative elapsed values are supported.
 * Metadata and actor must be disjoint.
 * Refusal leaves every output byte intact. Output publication follows the
 * original order: byte55, half4C, float44. A zero rate word writes nothing;
 * negative-zero has the distinct original nonzero-word behavior. */
/* Negative elapsed fractions are refused atomically: their sign extraction
 * depends on EE adder alignment behavior not yet independently established.
 * Supported reverse advances therefore require integral computed elapsed time. */
int dreadzone_animation_clock_advance(
    const DreadzoneAnimationClockRequest *request);

#ifdef __cplusplus
}
#endif

#endif
