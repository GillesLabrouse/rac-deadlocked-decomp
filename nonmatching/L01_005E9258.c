/* Complete L01 retail CPU routine; baseline SN build, draft-only coverage. */
/* Nonmatching: both extents96 bytes; instruction allocation differs. */
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
void func_L01_005E9258(EQ_CTX s32 index) {
    s32 i;
    for(i=2;i>=0;--i) EQ_STORE(0x0020C690U+(u32)index*0xCU+(u32)i*4U,0);
    EQ_STORE(0x003301E0U+(u32)index*0xB8U,EQ_MWORD(0x003301E0U+(u32)index*0xB8U)|0xDU);
}
