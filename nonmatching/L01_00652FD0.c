/* Complete original88 bytes, default SN compiled88 bytes: private exact.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

s32 func_L01_00652FD0(const void *unused,s32 value,s32 lower,s32 upper) {
    u32 width=(u32)upper-(u32)lower+1U;
    (void)unused;
    while(value<lower) value=(s32)((u32)value+width);
    while(value>upper) value=(s32)((u32)value-width);
    return value;
}
