#include "common.h"

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_model_pool.h"
#include <stdint.h>
#include <stdlib.h>

int dreadzone_model_pool_init(void) {
    u8 *config = dreadzone_model_resolve(DREADZONE_MODEL_CONFIG, 0x20);
    u32 matrices, descriptors;
    if (config == NULL) return DREADZONE_MODEL_UNMAPPED;
    matrices = dreadzone_model_read32(config + 0x18);
    descriptors = dreadzone_model_read32(config + 0x1C);
    if (matrices == 0 || descriptors == 0 || (matrices & 15) != 0 || (descriptors & 7) != 0) {
        return DREADZONE_MODEL_CORRUPT;
    }
    if ((uint64_t)matrices < (uint64_t)descriptors + DREADZONE_MODEL_DESCRIPTOR_BYTES &&
        (uint64_t)descriptors < (uint64_t)matrices + DREADZONE_MODEL_MATRIX_BYTES) return DREADZONE_MODEL_CORRUPT;
    if (dreadzone_model_resolve(matrices, DREADZONE_MODEL_MATRIX_BYTES) == NULL ||
        dreadzone_model_resolve(descriptors, DREADZONE_MODEL_DESCRIPTOR_BYTES) == NULL ||
        dreadzone_model_resolve(DREADZONE_MODEL_ANIMATION, 32 * 8) == NULL) {
        return DREADZONE_MODEL_UNMAPPED;
    }
    D_00222D78 = descriptors;
    D_00222D7C = matrices;
    func_L01_0058B050();
    func_L01_0058B4D0();
    return DREADZONE_MODEL_OK;
}

void func_L01_0058B098(void) {
    if (dreadzone_model_pool_init() != 0) abort();
}
#else
typedef struct ModelPoolConfig {
    u8 f0[0x18];
    void *f18;
    void *f1C;
} ModelPoolConfig;
extern ModelPoolConfig D_00243700 __attribute__((section(".data")));
extern void *D_00222D78 __attribute__((section(".data")));
extern void *D_00222D7C;
extern void func_L01_0058B050(void);
extern void func_L01_0058B4D0(void);

void func_L01_0058B098(void) {
    D_00222D78 = D_00243700.f1C;
    D_00222D7C = D_00243700.f18;
    func_L01_0058B050();
    func_L01_0058B4D0();
}
#endif
