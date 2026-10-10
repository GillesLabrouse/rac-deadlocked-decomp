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
s32 func_L01_006C0768(SETUP_CTX u8 *inventory,s32 item,s32 slot);
#ifndef DEADLOCKED_NATIVE
const u8 *func_L01_006BFFC0(s32 slot);
void func_L01_006BFF10(s32 message,s32 cost,s32 duration);
#endif
void func_L01_006C01F8(SETUP_CTX u8 *inventory,s32 slot,s32 cost,s32 item) {
    if((u32)slot-1U<8) {
        inventory[0x20+(u32)slot]++;
        if(cost>=0) {
#ifdef DEADLOCKED_NATIVE
            /* Paid-message branches are refused by public preflight. */
            dreadzone_setup_unavailable();
#else
            func_L01_006BFF10(SETUP_HALF(func_L01_006BFFC0(slot)+0x20),cost,200);
#endif
        }
        if(item>=2) (void)func_L01_006C0768(SETUP_PASS inventory,item,slot);
    }
}
