/* Complete original72 bytes, default SN compiled80 bytes: NONMATCHING.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

void func_L01_005EA6A8(UI_ONLY) {
    s32 i;
    if(UI_R32(0x0021E824)==0 && UI_R32(0x0021E828)==1)
        for(i=0;i<2;++i) {
            u32 address=0x003301E0U+(u32)i*0xB8U;
            UI_W32(address,UI_R32(address)|0xCU);
        }
}
