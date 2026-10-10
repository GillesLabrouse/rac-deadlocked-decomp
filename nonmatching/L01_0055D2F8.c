/* Complete 0055D2F8..0055D3B8,192 bytes. Retail uses handwritten stores;
 * this C reconstruction is a nonmatching draft, not an SDK source copy. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_inventory_reset.h"
#include <stdint.h>
static void clear_store(u8 *p, u32 size) {
    u32 i;
    for (i=0; i<size; ++i) p[i]=0;
}
#define CLEAR_AT(address,size) clear_store(storage+(address-start),(size))
int dreadzone_clear_0055D2F8(const DreadzoneInventoryMemory *view,s32 bytes) {
    u32 start,cursor,end,end16,end8,end4;
    u8 *storage;
    if (!view || !view->storage || (view->guest_address&15) || bytes<0 ||
        ((u32)bytes&15) || view->bytes<(u32)bytes ||
        (u64)view->guest_address+view->bytes>0x80000000ULL ||
        (u64)view->guest_address+(u32)bytes>=0x80000000ULL ||
        (uintptr_t)view->storage>UINTPTR_MAX-view->bytes)
        return DREADZONE_INVENTORY_RESET_INVALID;
    start=cursor=view->guest_address;
    storage=view->storage;
#else
typedef unsigned int InventoryClearQuad __attribute__((mode(TI)));
#define CLEAR_AT(address,size) do { \
    if ((size)==2) *(u16 *)(address)=0; \
    else if ((size)==4) *(u32 *)(address)=0; \
    else if ((size)==8) *(u64 *)(address)=0; \
    else *(InventoryClearQuad *)(address)=0; \
} while(0)
void func_L01_0055D2F8(u8 *destination,s32 bytes) {
    u32 cursor=(u32)destination,end,end16,end8,end4;
#endif
    end=cursor+(u32)bytes; end16=end&~15U; end8=end&~7U; end4=end&~3U;
    if (cursor&2) { CLEAR_AT(cursor,2); cursor+=2; }
    if (cursor&4) { CLEAR_AT(cursor,4); cursor+=4; }
    if (cursor&8) { CLEAR_AT(cursor,8); cursor+=8; }
    while (cursor!=end16) {
        CLEAR_AT(cursor,16); cursor+=16;
        if (cursor==end16) break;
        CLEAR_AT(cursor,16); cursor+=16;
    }
    if (cursor!=end8) { CLEAR_AT(cursor,8); cursor+=8; }
    if (cursor!=end4) { CLEAR_AT(cursor,4); cursor+=4; }
    if (cursor!=end) CLEAR_AT(cursor,2);
#ifdef DEADLOCKED_NATIVE
    return DREADZONE_INVENTORY_RESET_COMPLETED;
#endif
}
#undef CLEAR_AT
