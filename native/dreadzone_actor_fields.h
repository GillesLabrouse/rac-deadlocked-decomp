#ifndef DEADLOCKED_DREADZONE_ACTOR_FIELDS_H
#define DEADLOCKED_DREADZONE_ACTOR_FIELDS_H
#include "common.h"

/* Independent bounded views. Addresses in the actor remain guest32 values;
 * host storage is never installed into a guest pointer field. */
typedef struct DreadzoneActorFieldView {
    u8 *record;
    u32 record_size, record_guest;
    const u8 *client;
    u32 client_size, client_guest;
    u8 *mode_bytes_00171DE0;
    u32 mode_count;
    s32 flag_0021E878;
} DreadzoneActorFieldView;

enum DreadzoneActorFieldResult {
    DREADZONE_ACTOR_FIELDS_EXACT_MODE_ZERO = 0,
    DREADZONE_ACTOR_FIELDS_APPROXIMATE_VU = 1,
    DREADZONE_ACTOR_FIELDS_INVALID_STORAGE = -1,
    DREADZONE_ACTOR_FIELDS_NEEDS_CLIENT = -2,
    DREADZONE_ACTOR_FIELDS_UNSUPPORTED_FLOAT_DOMAIN = -3
};

/* Actual0068E3B0 mutations, not a constructor or gameplay substitute.
 * Mode0 is exact and does not access the client. Nonzero modes copy/normalize
 * client+390 XYZ, preserve W bits, clear two vectors and flagbit0, and then
 * perform the real mode1/2 writes. Each nonzero XYZ component must have absolute
 * magnitude in [2^-30,2^30]; either signed zero is accepted. W is only copied.
 * Unsupported domains/bindings fail before any mutation.
 *
 * Nonzero mode returns APPROXIMATE_VU even for a zero vector. Native sqrt and
 * floating-point arithmetic are not claimed to reproduce VU rounding, ACC
 * precision, registers, status flags, or instruction scheduling. The real
 * qmfc2/beqz uses the lower64bits, including vf2.y==1.f, so zero input reaches
 * VRSQRT and a finite saturated Q; zero XYZ is multiplied by that Q, retaining
 * signed zero. It must not become NaN from a host infinite reciprocal.
 */
int dreadzone_actor_0068E3B0(DreadzoneActorFieldView *, s32 mode);

#endif
