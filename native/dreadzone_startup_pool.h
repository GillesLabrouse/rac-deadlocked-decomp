#ifndef DEADLOCKED_DREADZONE_STARTUP_POOL_H
#define DEADLOCKED_DREADZONE_STARTUP_POOL_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct DreadzoneStartupPoolMemory {
    uint8_t *bytes;
    uint32_t guest_base;
    size_t length;
} DreadzoneStartupPoolMemory;

/* Game-RAM effects of actual L01 00667518 with A0=6. Captures a readable
 * descriptor disjoint from truthful, stable, exclusive writable backing.
 * One nonwrapping linear guest mapping below2GiB, at most16MiB. The tables
 * at366138/366154/366170 and count/stride at3B0060/3B00A8 must be mapped.
 * The original code publishes the real BSS workspace address37F468, then
 * builds its ordered +14/+10/+0C links. Existing unrelated bytes are retained.
 * Positive counts are <=4096, stride is divisible by4, and the entire record
 * footprint ends before3B0060. This prevents writes altering the original
 * reread count/stride globals; other domains refuse. Nonpositive signed count
 * preserves the original table-only path. The +14 word is a guest table identifier, not a host pointer.
 * No memory is allocated or supplied.
 * Return0 on complete game-RAM publication, -1 on atomic mapping/domain refusal.
 * Original GPR/HI/LO and stack effects, partial guest faults, host callback
 * invocation and the enclosing initializer are outside this contract.
 */
int dreadzone_startup_pool_00667518_index6(const DreadzoneStartupPoolMemory *);
/* Independently verified L01 006670B8 with A0=2, using the same mapping and
 * atomic-refusal preconditions. Tables are366128/366144/366160, count3B0050,
 * stride3B0098, workspace37F420 and descriptor3B0068. Positive counts<=4096
 * use word-divisible strides; the full record footprint ends before3B0050.
 * Signed nonpositive counts publish only the three table words. The actual
 * serialized index2 count is0 and stride24. No runtime setup call is implied.
 */
int dreadzone_startup_pool_006670B8_index2(const DreadzoneStartupPoolMemory *);
#ifdef __cplusplus
}
#endif
#endif
