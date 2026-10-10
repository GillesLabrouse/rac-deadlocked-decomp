#include "common.h"

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_byte_fill.h"
#include <stdlib.h>
#else
extern u32 D_002437C4 __attribute__((section(".data")));
extern void *func_0011A518(void *destination, s32 value, u32 count);
#endif

void func_L01_005D6CB0(void) {
#ifdef DEADLOCKED_NATIVE
    if (dreadzone_byte_fill_guest(D_002437C4, -559038737, DREADZONE_BYTE_FILL_BYTES) != 0) {
        abort();
    }
#else
    func_0011A518((void *)D_002437C4, -559038737, 0x40000);
#endif
}
