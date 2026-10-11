#ifndef DEADLOCKED_DREADZONE_POOL_BOUNDS_H
#define DEADLOCKED_DREADZONE_POOL_BOUNDS_H
#include <stdint.h>
typedef struct DreadzonePoolBoundsRequest {
    uint8_t *storage;
    uint32_t guest_base, storage_bytes, cursor_guest, gp;
} DreadzonePoolBoundsRequest;
/* Complete 0057BAC0 RAM effects, including genuine 00578928, 00586F78
 * and 0055D2A8. One caller-owned <=16MiB range below 2GiB, excluding
 * scratchpad. Request may alias RAM. Success=1; invalid host=-1;
 * unsupported mapping/trap=-2; allocation failure=-3. Refusals are atomic.
 * No actor construction, model setup or original return-register ABI. */
int dreadzone_pool_bounds_0057BAC0(const DreadzonePoolBoundsRequest *request);
#endif
