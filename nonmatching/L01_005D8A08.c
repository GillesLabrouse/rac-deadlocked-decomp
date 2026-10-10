/* Complete original48 bytes, default SN compiled48 bytes: private exact.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

s32 func_L01_005D8A08(UI_CTX s32 index) {
    u32 actor=UI_ACTOR(index);
    u32 selected=UI_R32(actor+0x2674);
    if(!selected) selected=UI_R32(actor+0x268C);
    return (s32)selected;
}
