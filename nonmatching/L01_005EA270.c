/* Complete original80 bytes, default SN compiled84 bytes: NONMATCHING.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

UiWordPointer func_L01_005EA270(UI_CTX s32 index) {
    u32 row=0x00330148U+(u32)index*0xB8U;
    if(UI_R32(row+0x24)) return UI_POINTER(row+0x24);
    if(UI_R32(row+0x28)) return UI_POINTER(row+0x28);
    return UI_POINTER(0x0020C690U+(u32)index*0xCU);
}
