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

s32 func_L01_006C0740(SETUP_CTX s32 item,s32 level) {
    return (s32)SETUP_READ32(0x003BDE38U+(u32)item*0x28U+(u32)level*4U);
}
