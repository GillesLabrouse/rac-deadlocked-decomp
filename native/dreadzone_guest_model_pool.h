#ifndef DEADLOCKED_GUEST_MODEL_POOL_H
#define DEADLOCKED_GUEST_MODEL_POOL_H
#include <stdint.h>

typedef struct DreadzoneGuestModelPool {
    uint8_t *storage;
    uint32_t guest_base, bytes, gp, actor;
    uint32_t operation;
} DreadzoneGuestModelPool;

enum {
    DREADZONE_GUEST_MODEL_ALLOCATE = 1,
    DREADZONE_GUEST_MODEL_RELEASE = 2,
    DREADZONE_GUEST_MODEL_COMPACT = 3
};

/* RAM effects of L01 0058B148/0058B268/0058B338. Caller owns one <=16MiB
 * range below 2GiB, excluding scratchpad, with GP0022597F. Request may alias
 * RAM. Globals00222D78..9C, complete descriptor/matrix banks, live100-byte
 * actors and nine-byte definition prefixes must satisfy validated pool
 * invariants and disjoint writable footprints. Shared definitions are allowed.
 * Allocate requires a nonzero definition count and a previously unallocated
 * actor; release requires a live descriptor or an original early-return path.
 * Release preserves actor+58. Compaction can move every live actor's matrices.
 * Success=1; invalid host=-1; mapping=-2; unsupported state=-3;
 * insufficient supported capacity=-4; host allocation failure=-5.
 * All refusals leave RAM and result unchanged. Capacity/state refusals are
 * host support limits, not error returns of the original functions.
 * Allocate publishes its matrix address to a result outside request and RAM.
 * Other operations ignore result. Complete RAM publishes atomically; no
 * legacy host globals or bindings are accessed. Original stack/register ABI,
 * hardware timing and partial writes on original faults are outside this API.
 * This memory adapter does not earn original-compiler matching credit. */
int dreadzone_guest_model_pool_apply(const DreadzoneGuestModelPool *request,
                                   uint32_t *matrix_result);
#endif
