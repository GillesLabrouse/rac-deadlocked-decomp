/* Complete original120 bytes, default SN compiled120 bytes: NONMATCHING.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

s32 func_L01_005D8AE0(UI_CTX s32 index,s32 item) {
    if(func_L01_005D89E8(UI_PASS index)) return 0;
    if(func_L01_005D8A08(UI_PASS index)==item) return 0;
    if(!func_L01_005D8A40(UI_PASS index,item)) return 0;
#ifdef DEADLOCKED_NATIVE
    /* Reentrant setters are refused by the bounded entry before mutation. */
    dreadzone_setup_unavailable();
#else
    (void)func_L01_00688578(func_L01_006658C0(index),item);
#endif
    return 1;
}
