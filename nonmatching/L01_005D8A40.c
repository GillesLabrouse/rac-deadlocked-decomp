/* Complete original160 bytes, default SN compiled160 bytes: NONMATCHING.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

s32 func_L01_005D8A40(UI_CTX s32 index,s32 item) {
    s32 other,okay;
    if(UI_R32(0x0021E824)!=0 || UI_R32(0x0021E828)!=1) return 1;
    if(item==14) return 1;
    other=index==0;
    okay=func_L01_005D8A08(UI_PASS other)!=item;
    if(item==2 && func_L01_005D89E8(UI_PASS other)) okay=1;
    return okay;
}
