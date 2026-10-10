#include "common.h"
/* Recovered packed-field operation at 0058D468..0058D488. Inputs two through
 * four are not masked in retail; the caller 00589210 supplies byte values. */
#ifndef DEADLOCKED_NATIVE
typedef char class_packed_width_check[sizeof(u64) == 8 ? 1 : -1];
u64 func_L01_0058D468(u8 *actor, u64 upper, u64 component0, u64 component1, u64 component2) {
    u64 packed = (upper << 32) | component0 | (component1 << 8) | (component2 << 16);
    *(u64 *)(actor + 0x38) = packed;
    return packed;
}
#else
#include "../native/dreadzone_actor_class.h"
#include <stdint.h>
#include <string.h>
void dreadzone_actor_pack_environment(u8 *actor, u32 upper, u32 component0, u32 component1, u32 component2) {
    uint64_t packed = ((uint64_t)upper << 32) | component0
        | ((uint64_t)component1 << 8) | ((uint64_t)component2 << 16);
    memcpy(actor + 0x38, &packed, 8);
}
#endif
