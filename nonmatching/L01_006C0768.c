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
s32 func_L01_006C0090(SETUP_CTX const u8 *inventory,s32 slot);
s32 func_L01_006BE198(SETUP_CTX const u8 *inventory,s32 item);
s32 func_L01_006C04A8(SETUP_CTX u8 *inventory,s32 slot);
s32 func_L01_006C0768(SETUP_CTX u8 *inventory,s32 item,s32 slot) {
    s32 limit,i;
    u8 *entries;
    if((u32)slot-1U>=8) return 0;
    if(!func_L01_006C0090(SETUP_PASS inventory,slot)) return 0;
    limit=func_L01_006BE198(SETUP_PASS inventory,item)+1;
    if(SETUP_READ32(0x0021E824U)==1) limit=10;
    if(limit<=0) return 0;
    entries=inventory+0xA44+(u32)item*0x44;
    for(i=0;i<limit && i<10;++i) {
        if(!entries[(u32)i*4]) {
            entries[(u32)i*4]=(u8)slot;
            (void)func_L01_006C04A8(SETUP_PASS inventory,slot);
            return 1;
        }
    }
    return 0;
}
