/* Complete original104 bytes, default SN compiled104 bytes: NONMATCHING.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

void func_L01_005E9350(UI_CTX s32 index) {
    u32 row=0x00330148U+(u32)index*0xB8U;
    s32 old=(s32)UI_R32(row+0x28);
    UI_W32(row+0x28,0);
    if(func_L01_005D8A08(UI_PASS index)==old)
        (void)func_L01_005D8AE0(UI_PASS index,(s32)UI_R32(UI_ADDRESS(func_L01_005EA270(UI_PASS index))));
}
