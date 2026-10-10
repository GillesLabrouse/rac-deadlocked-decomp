/* Complete original640 bytes, default SN compiled644 bytes: NONMATCHING.
 * Draft-only source, outside accepted coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment_ui.h"
#endif
#include "../include/dreadzone_equipment_ui_internal.h"

s32 func_L01_005E93D8(UI_CTX s32 index,s32 item,s32 force) {
    u32 row=0x00330148U+(u32)index*0xB8U;
    u32 chosen=UI_ADDRESS(func_L01_005EA270(UI_PASS index));
    s32 other;
    if(!func_L01_006BDDF0(UI_PASS item)) return 0;
    if(UI_R32(chosen)!=(u32)item && !func_L01_005D89E8(UI_PASS index)) {
        const u8 *definition=UI_DEFINITION(item);
        if(!definition[0x28]) {
            if(UI_DEFINITION((s32)UI_R32(chosen))[0x28]) {
                UI_W32(row+0x28,UI_R32(chosen));
                chosen=row+0x28;
            }
        } else if(UI_DEFINITION((s32)UI_R32(chosen))[0x28]) {
            u32 recent=0x0020C690U+(u32)index*0xCU;
            s32 j,position=2;
            for(j=0;j<3;++j) {
                u32 value=UI_R32(recent+(u32)j*4U);
                if(value==(u32)item || !value) {position=j;break;}
            }
            while(position>0) {
                UI_W32(recent+(u32)position*4U,UI_R32(recent+(u32)(position-1)*4U));
                --position;
            }
        }
    }
    UI_W32(chosen,item);
    UI_W32(row+0x98,UI_R32(row+0x98)|9U);
    if(UI_R32(0x0021E824)!=0 || UI_R32(0x0021E828)!=1) return 1;
    other=index==0;
    if(UI_R32(0x0020C690U+(u32)other*0xCU)==UI_R32(0x0020C690U+(u32)index*0xCU)) {
        func_L01_005E97D8(UI_PASS other,func_L01_005EA230(UI_PASS other),1);
        if(func_L01_005D8A08(UI_PASS other)!=item &&
           !UI_R32(0x00330148U+(u32)other*0xB8U+0x28) &&
           !UI_R32(0x00330148U+(u32)other*0xB8U+0x24) && !force)
            (void)func_L01_005D8AE0(UI_PASS other,(s32)UI_R32(UI_ADDRESS(func_L01_005EA270(UI_PASS other))));
    } else {
        u32 value=UI_R32(row+0x28);
        if(value==UI_R32(0x00330148U+(u32)other*0xB8U+0x28) && value)
            func_L01_005E9350(UI_PASS other);
    }
    func_L01_005EA6A8(UI_ARG);
    func_L01_005F0A40(UI_ARG);
    return 1;
}
