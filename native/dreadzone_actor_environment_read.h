#ifndef DREADZONE_ACTOR_ENVIRONMENT_READ_H
#define DREADZONE_ACTOR_ENVIRONMENT_READ_H
#include <stdint.h>
typedef struct DreadzoneActorEnvironmentReadRequest {
    uint8_t *storage;
    uint32_t guest_base,storage_bytes,actor_guest,output_guest[3];
} DreadzoneActorEnvironmentReadRequest;
/* Full L01 58D4A0 / resident4700C8 RAM effects only; caller-supplied mapped
 * memory, <=16 MiB below80000000, excluding scratch70000000..70003FFF.
 * Read actor3C once, then store byte0/1/2 as little-endian words in order.
 * Outputs may alias actor/each other/request. Request is captured before
 * stores. All reached accesses must be mapped/aligned; refusal is atomic.
 * Returns1 on completion,-1 for invalid host/span,-2 for unsupported mapping.
 * No actor initialization, original register ABI or exact-code claim. */
int dreadzone_actor_environment_read_0058D4A0(const DreadzoneActorEnvironmentReadRequest *);
#endif
