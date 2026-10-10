/* Complete 006C0D88..006C0E54,204 instruction bytes plus4 padding bytes.
 * Constructor inventory reset. This draft is excluded from matching coverage. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_inventory_reset.h"
#include <stdint.h>
static void reset_half(u8 *record,u32 offset,u16 value) {
    record[offset]=(u8)value; record[offset+1]=(u8)(value>>8);
}
#define RESET_HALF(offset,value) reset_half(record,offset,(u16)(value))
#define RESET_WORD(offset) do { u32 n; for(n=0;n<4;++n) record[(offset)+n]=0; } while(0)
#define RESET_CLEAR(offset,length) do { \
    DreadzoneInventoryMemory part={record+(offset),view->guest_address+(offset),(length)}; \
    (void)dreadzone_clear_0055D2F8(&part,(length)); \
} while(0)
int dreadzone_inventory_006C0D88(const DreadzoneInventoryMemory *view) {
    u8 *record;
    s32 i;
    if (!view || !view->storage || view->bytes<0x12B0 || (view->guest_address&15) ||
        (u64)view->guest_address+view->bytes>0x80000000ULL ||
        (u64)view->guest_address+0x12B0>=0x80000000ULL ||
        (uintptr_t)view->storage>UINTPTR_MAX-view->bytes)
        return DREADZONE_INVENTORY_RESET_INVALID;
    record=view->storage;
#else
void func_L01_0055D2F8(u8 *,s32);
#define RESET_HALF(offset,value) (*(u16 *)(record+(offset))=(u16)(value))
#define RESET_WORD(offset) (*(u32 *)(record+(offset))=0)
#define RESET_CLEAR(offset,length) func_L01_0055D2F8(record+(offset),(length))
void func_L01_006C0D88(u8 *record) {
    s32 i;
#endif
    record[0x20]=0; RESET_HALF(0x2A,0); RESET_WORD(0x2C);
    RESET_CLEAR(0x30,0xA00); RESET_CLEAR(0xA30,0x880);
    for (i=31;i>=0;--i) record[0x32+(u32)i*0x50]=0xFF;
    for (i=31;i>=0;--i) RESET_HALF(0xA30+(u32)i*0x44,0xFFFF);
    for (i=0;i<10;++i) { record[2+i]=0; RESET_HALF(0xC+(u32)i*2,0); }
    record[0]=1;
#ifdef DEADLOCKED_NATIVE
    return DREADZONE_INVENTORY_RESET_COMPLETED;
#endif
}
#undef RESET_HALF
#undef RESET_WORD
#undef RESET_CLEAR
