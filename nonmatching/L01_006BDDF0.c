/* Complete original64 bytes, default SN compiled52 bytes: NONMATCHING.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

s32 func_L01_006BDDF0(UI_CTX s32 item) {
    if(item==1) return 0;
    return (s8)UI_DEFINITION(item)[0x2C]==0;
}
