/* Complete original168 bytes, default SN compiled168 bytes: NONMATCHING.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

void func_L01_005E97D8(UI_CTX s32 index,s32 count,s32 shift) {
    u32 copy[3];
    u32 row=0x0020C690U+(u32)index*0xCU;
    s32 i;
    UI_COPY(copy,row,(u32)count*4U);
    for(i=0;i<count;++i) {
        s32 position=func_L01_00652FD0((const void *)0x002239D0,(s32)((u32)i+(u32)shift),0,count-1);
        UI_W32(row+(u32)i*4U,copy[position]);
    }
}
