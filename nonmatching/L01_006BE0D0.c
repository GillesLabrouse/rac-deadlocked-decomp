/* Complete CPU routine recovered from the actual L01 retail overlay.
 * Draft-only source, outside matching coverage. Uses baseline SN flags. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_inventory_setup.h"
#define SETUP_READ8(address) dreadzone_setup_read8(ctx,(address))
#define SETUP_READ32(address) dreadzone_setup_read32(ctx,(address))
#define SETUP_TABLE(address) dreadzone_setup_table(ctx,(address))
#define SETUP_HALF(p) dreadzone_setup_half(p)
#define SETUP_STORE16(p,value) dreadzone_setup_store16((p),(u16)(value))
#define SETUP_STORE32(p,value) dreadzone_setup_store32((p),(u32)(value))
#else
#define SETUP_CTX
#define SETUP_ONLY void
#define SETUP_PASS
#define SETUP_ARG
#define SETUP_READ8(address) (*(const u8 *)(address))
#define SETUP_READ32(address) (*(const u32 *)(address))
#define SETUP_TABLE(address) ((const u8 *)(address))
#define SETUP_HALF(p) (*(const s16 *)(p))
#define SETUP_STORE16(p,value) (*(u16 *)(p)=(u16)(value))
#define SETUP_STORE32(p,value) (*(u32 *)(p)=(u32)(value))
#endif
s32 func_L01_005688A0(SETUP_ONLY);
void func_L01_006BE480(SETUP_CTX u8 *inventory,s32 item,s32 value);
s32 func_L01_006BE050(SETUP_CTX s32 item);
s32 func_L01_006BE198(SETUP_CTX const u8 *inventory,s32 item);
#ifndef DEADLOCKED_NATIVE
s32 func_L01_006BE4E8(s32 item,s32 level);
#endif
void func_L01_006BE0D0(SETUP_CTX u8 *inventory,s32 item,s32 level) {
    s32 maximum;
    (void)func_L01_005688A0(SETUP_ARG);
    if(level==-1) func_L01_006BE480(SETUP_PASS inventory,item,0);
    else {
        maximum=func_L01_006BE050(SETUP_PASS item);
        if(maximum<level) level=maximum;
        if(func_L01_006BE198(SETUP_PASS inventory,item)!=level) {
            if(level>0) {
#ifdef DEADLOCKED_NATIVE
                /* Positive progression is refused by public preflight. */
                dreadzone_setup_unavailable();
#else
                func_L01_006BE480(inventory,item,func_L01_006BE4E8(item,level-1));
#endif
            } else func_L01_006BE480(SETUP_PASS inventory,item,0);
        }
    }
    SETUP_STORE16(inventory+0xA30+(u32)item*0x44,level);
}
