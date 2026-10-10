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
u32 func_L01_006BF278(SETUP_CTX const u8 *inventory,s32 item);
void func_L01_006BE0D0(SETUP_CTX u8 *inventory,s32 item,s32 level);
const u8 *func_L01_006BF200(SETUP_CTX s32 item,s32 variant);
s32 func_L01_006C0740(SETUP_CTX s32 item,s32 level);
s32 func_L01_006C0C60(SETUP_CTX const u8 *inventory,s32 item,s32 slot);
s32 func_L01_006C0090(SETUP_CTX const u8 *inventory,s32 slot);
void func_L01_006C01F8(SETUP_CTX u8 *inventory,s32 slot,s32 cost,s32 item);
s32 func_L01_006C0768(SETUP_CTX u8 *inventory,s32 item,s32 slot);
s32 func_L01_006BE240(SETUP_CTX const u8 *inventory,s32 item);
void func_L01_006BE208(SETUP_CTX u8 *inventory,s32 item,s32 amount);
s32 func_L01_006BE270(SETUP_CTX const u8 *inventory,s32 item);
void func_L01_006BF2D0(SETUP_CTX u8 *inventory,s32 item,s32 level,s32 preserve_amount,s32 force_slot) {
    const u8 *definition;
    s32 slot;
    if(func_L01_006BF278(SETUP_PASS inventory,item)) return;
    func_L01_006BE0D0(SETUP_PASS inventory,item,level);
    definition=func_L01_006BF200(SETUP_PASS item,0);
    slot=func_L01_006C0740(SETUP_PASS item,0);
    if(SETUP_READ32(0x0021E824U)!=1 && level==0 &&
       !func_L01_006C0C60(SETUP_PASS inventory,item,slot)) {
        if(force_slot || !func_L01_006C0090(SETUP_PASS inventory,slot))
            func_L01_006C01F8(SETUP_PASS inventory,slot,-1,item);
        else (void)func_L01_006C0768(SETUP_PASS inventory,item,slot);
    }
    if(definition[0x28]) {
        if(!func_L01_006BE240(SETUP_PASS inventory,item) || !preserve_amount)
            func_L01_006BE208(SETUP_PASS inventory,item,func_L01_006BE270(SETUP_PASS inventory,item));
    }
}
