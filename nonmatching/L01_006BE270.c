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
const u8 *func_L01_006BF200(SETUP_CTX s32 item,s32 variant);
s32 func_L01_006C0C60(SETUP_CTX const u8 *inventory,s32 item,s32 slot);
s32 func_L01_006BE270(SETUP_CTX const u8 *inventory,s32 item) {
    const u8 *definition=func_L01_006BF200(SETUP_PASS item,0);
    s32 amount=SETUP_HALF(definition+0x42),count;
    u32 bits;
    if(SETUP_READ32(0x0021E824U)==1)
        amount=SETUP_HALF(func_L01_006BF200(SETUP_PASS item,0)+0x44);
    count=func_L01_006C0C60(SETUP_PASS inventory,item,2);
    bits=(u32)amount+(u32)count*SETUP_READ32(0x003BDDE8U+(u32)item*4U);
    return (s32)((bits&0xFFFFU)^0x8000U)-0x8000;
}
