/* Complete original48 bytes, default SN compiled48 bytes: NONMATCHING.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

void func_L01_005F0A40(UI_ONLY) {
    u32 first=UI_R32(0x003312F0);
    u32 second=UI_R32(0x00331354);
    UI_W32(0x003312F0,first|1U);
    UI_W32(0x00331354,second|1U);
}
