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
/* Independently verified L01 006671D0 with A0=3; same mapping and refusal
 * preconditions. Tables are36612C/366148/366164, count3B0054, stride3B009C,
 * workspace37F450 and node identifier3B1510. The positive record footprint
 * ends before3B0054. Serialized count is0 and stride84; this does not prove
 * a live initialization call. Original CPU and partial fault effects remain
 * outside the RAM-only contract.
 */
int dreadzone_startup_pool_006671D0_index3(const DreadzoneStartupPoolMemory *);
/* Independently verified L01 006672E8 with A0=4; same mapping/refusal rules.
 * Tables366130/36614C/366168, count3B0058, stride3B00A0, workspace37F458,
 * node identifier3B1538. Positive record footprint ends before3B0058.
 * Serialized count0/stride80 does not imply a completed live initializer.
 */
int dreadzone_startup_pool_006672E8_index4(const DreadzoneStartupPoolMemory *);
/* Independently verified L01 00667400 with A0=5; same mapping/refusal rules.
 * Tables366134/366150/36616C, count3B005C, stride3B00A4, workspace37F460,
 * node identifier3B1588. Positive record footprint ends before3B005C.
 * Serialized count0/stride24 is a leaf input, not proof of live startup.
 */
int dreadzone_startup_pool_00667400_index5(const DreadzoneStartupPoolMemory *);
/* Selected actual first-family leaf called by L01 00667668, index1..6:
 * 667988,667AA0,667BB8,667CD0,667DE8,667F00. Same exclusive truthful
 * backing/descriptor rules; one linear <=16MiB mapping below2GiB.
 * Tables are368B78/368BB0, count3B0048+4*index, stride3B0110+4*index.
 * Each leaf has its own independently verified pool address and +C tag.
 * Positive count<=4096 requires a nonnegative stride divisible by4; mapped
 * record footprint ends at the last record+10 and may not overlap this
 * leaf's root/count/stride/pool cells. Strides0/4/8 retain ordered overlapping
 * record writes (+C tag,+8 previous,+4 next). Signed nonpositive count
 * publishes only three actual table words and does not require mapped stride.
 * Return0 complete RAM effects,-1 atomic refusal. No allocation, runtime BSS
 * origin, CPU/stack ABI, partial faults or enclosing wrapper is supplied.
 */
int dreadzone_startup_first_pool_leaf(const DreadzoneStartupPoolMemory *, uint32_t index);
#ifdef __cplusplus
}
#endif
#endif
