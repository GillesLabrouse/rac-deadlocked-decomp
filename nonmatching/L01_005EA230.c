/* Complete original64 bytes, default SN compiled64 bytes: NONMATCHING.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

s32 func_L01_005EA230(UI_CTX s32 index) {
    u32 address=0x0020C690U+(u32)index*0xCU;
    s32 count;
    for(count=0;count<3;++count) if(!UI_R32(address+(u32)count*4U)) return count;
    return 3;
}
