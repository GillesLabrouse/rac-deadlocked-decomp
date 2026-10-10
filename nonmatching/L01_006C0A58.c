/* Complete L01 retail CPU routine; baseline SN build, draft-only coverage. */
/* Nonmatching: both extents136 bytes; instruction allocation differs. */
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
void func_L01_006C01F8(u8 *,s32,s32,s32);
#endif
s32 func_L01_006C0A58(EQ_CTX u8 *inventory,s32 item,s32 restore_slots) {
    s32 i,found=0;
    u8 *entry=inventory+0xA44+(u32)item*0x44;
    for(i=0;i<10;++i,entry+=4) {
        s32 slot=*entry;
        if(slot>0) {
            *entry=0;
            if(restore_slots) func_L01_006C01F8(EQ_SETUP_PASS inventory,slot,-1,0);
            found=1;
        }
    }
    return found;
}
