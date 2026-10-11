#ifndef DEADLOCKED_DREADZONE_INITIAL_POOL_H
#define DEADLOCKED_DREADZONE_INITIAL_POOL_H
#include <stdint.h>
typedef struct DreadzoneInitialPoolRequest {
    uint8_t *storage;
    uint32_t guest_base,storage_bytes,header_guest,pool_pointer_guest,gp;
} DreadzoneInitialPoolRequest;
/* Full0057B6B8 RAM effects with genuine55D2A8 and57B6B0 semantics.
 * Caller owns one <=16MiB RAM range below2GiB, excluding scratchpad.
 * Request is captured and can alias RAM; stores retain their original order.
 * Completion=1; invalid host descriptor=-1; unsupported mapping/trap=-2;
 * transaction allocation failure=-3. All refusals leave RAM unchanged.
 * No actor placement, class/model initialization or playable frame is supplied.
 * The filler writes at least one word, even for zero/negative byte counts.
 * GP is an explicit original input, not a manufactured fixed console value.
 */
int dreadzone_initial_pool_0057B6B8(const DreadzoneInitialPoolRequest *request);
#endif
