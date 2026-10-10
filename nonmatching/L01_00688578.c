/* Complete L01 retail CPU routine; baseline SN build, draft-only coverage. */
/* Nonmatching: compiled196 bytes versus184 retail bytes. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment.h"
#define EQ_RWORD(p,o) dreadzone_setup_word((p)+(o))
#define EQ_WWORD(p,o,value) dreadzone_setup_store32((p)+(o),(u32)(value))
#define EQ_READ(address) dreadzone_setup_read32(&ctx->inventory,(address))
#define EQ_BYTE(address) (*dreadzone_equipment_mutable(ctx,(address),1))
#define EQ_MWORD(address) dreadzone_equipment_mutable_word(ctx,(address))
#define EQ_STORE(address,value) dreadzone_equipment_store_word(ctx,(address),(u32)(value))
#define EQ_INVENTORY(record) dreadzone_equipment_inventory(ctx,(record))
#define EQ_SETUP_PASS &ctx->inventory,
#else
#define EQ_CTX
#define EQ_ONLY void
#define EQ_PASS
#define EQ_ARG
#define EQ_RWORD(p,o) (*(volatile u32 *)((p)+(o)))
#define EQ_WWORD(p,o,value) (*(volatile u32 *)((p)+(o))=(u32)(value))
#define EQ_READ(address) (*(volatile u32 *)(address))
#define EQ_BYTE(address) (*(volatile u8 *)(address))
#define EQ_MWORD(address) (*(volatile u32 *)(address))
#define EQ_STORE(address,value) (*(volatile u32 *)(address)=(u32)(value))
#define EQ_INVENTORY(record) (*(u8 **)((record)+0x26E4))
#define EQ_SETUP_PASS
#endif
#ifndef DEADLOCKED_NATIVE
s32 func_L01_006BDDC0(s32);
s32 func_L01_006BE040(s32);
const u8 *func_L01_006BF200(s32,s32);
s32 func_L01_005D7558(s32 index,s32 item,s32 force);
#endif
s32 func_L01_00688578(EQ_CTX u8 *record,s32 item) {
    const u8 *definition;
    s32 category;
    if(EQ_READ(0x0021E824)==0 && EQ_READ(0x0021E828)==0 && func_L01_006BDDC0(item)) return 0;
    if(!func_L01_006BE040(item)) return 0;
    definition=func_L01_006BF200(EQ_SETUP_PASS item,0);
    category=(s8)definition[0x2C];
    EQ_WWORD(record,0x2674+category*4,item);
    if(record[0x266D]) {
#ifdef DEADLOCKED_NATIVE
        /* Public preflight rejects unresolved UI dispatch before all stores. */
        dreadzone_setup_unavailable();
#else
        s32 index=((s32 (*)(u8 *))EQ_RWORD((u8 *)EQ_RWORD(record,0x14),0x4C))(record);
        (void)func_L01_005D7558(index,item,1);
#endif
    }
    return 1;
}
