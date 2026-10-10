/* Complete original32 bytes, default SN compiled32 bytes: NONMATCHING.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

s32 func_L01_005D89E8(UI_CTX s32 index) {
    return UI_R8(0x0031B314U+(u32)index*0x54U);
}
