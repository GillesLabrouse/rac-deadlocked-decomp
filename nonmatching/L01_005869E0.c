/* cflags: -mno-split-addresses */
#include "common.h"

/* L01 actor pool allocator. Requires the real class/table initializer
 * 00586B10; no constructor or rendering effects are substituted here. */
extern u8 *D_00222D2C __attribute__((section(".data")));
extern u8 *D_00222D30 __attribute__((section(".data")));
extern u32 D_0021DDB8 __attribute__((section(".data")));
extern s32 D_00222D10 __attribute__((section(".data")));
extern u8 D_0021EC28;
u8 *func_L01_0058D538(u32);
void func_L01_00586B10(u8 *, s32, s32);
void func_L01_0055D2A8(void *, u32, s32);
void func_L01_00588CC8(void);
void func_L01_00588D08(void);

u8 *func_L01_005869E0(s32 objectClass, s32 payloadBytes) {
    u8 *actor = D_00222D2C;
    u8 *end = D_00222D30;
    u8 *payload;
    while (actor < end) {
        if (actor[0x20] >= 0xFE && D_0021DDB8 >= *(u32 *)(actor + 0xA0)) {
            payload = NULL;
            if (payloadBytes != 0) payload = func_L01_0058D538((u32)payloadBytes);
            if (payload != NULL || payloadBytes == 0) {
                /* This retail write reaches the following 0x100-byte slot.
                 * Its producer must provide accessible backing storage. */
                if (actor[0x20] == 0xFF) actor[0x120] = actor[0x20];
                func_L01_00586B10(actor, objectClass, 1);
                *(u8 **)(actor + 0xAC) = payload;
                if (payloadBytes != 0) func_L01_0055D2A8(payload, 0, payloadBytes);
                if (D_00222D10 != 0) D_00222D10--;
                return actor;
            }
            break;
        }
        actor += 0x100;
    }
    if (D_0021EC28 != 0) {
        func_L01_00588CC8();
        func_L01_00588D08();
    }
    return NULL;
}
