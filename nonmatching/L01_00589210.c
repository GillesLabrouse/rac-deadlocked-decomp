#include "common.h"
#ifndef DEADLOCKED_NATIVE
typedef char class_packed_width_check[sizeof(u64) == 8 ? 1 : -1];
extern u8 *D_00222C50 __attribute__((section(".data")));
u64 func_L01_0058D468(u8 *, u64, u64, u64, u64);
void func_L01_00589210(u8 *actor) {
    u8 *table = D_00222C50;
    if (table == NULL) {
        *(u64 *)(actor + 0x38) = (u64)0x00404040U << 32;
    } else {
        u8 *entry = table + *(u16 *)table;
        func_L01_0058D468(actor, *(u32 *)(entry + 0x40), entry[0x4A], entry[0x4A], 0);
    }
}
#else
#include "../native/dreadzone_actor_class.h"
#include <string.h>
void dreadzone_actor_initialize_environment(u8 *actor, u32 upper, u8 component, int has_table) {
    if (has_table) {
        dreadzone_actor_pack_environment(actor, upper, component, component, 0);
    } else {
        const u32 low = 0, high = 0x00404040U;
        memcpy(actor + 0x38, &low, 4);
        memcpy(actor + 0x3C, &high, 4);
    }
}
#endif
