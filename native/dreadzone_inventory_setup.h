#ifndef DEADLOCKED_DREADZONE_INVENTORY_SETUP_H
#define DEADLOCKED_DREADZONE_INVENTORY_SETUP_H
#include "common.h"
#include "dreadzone_inventory_reset.h"

typedef struct DreadzoneInventorySetupReadMemory {
    const u8 *storage;
    u32 guest_address,bytes;
} DreadzoneInventorySetupReadMemory;
typedef struct DreadzoneInventorySetupContext {
    u8 *record;
    u32 record_guest,record_bytes;
    DreadzoneInventoryMemory pool;
    const DreadzoneInventorySetupReadMemory *reads;
    u32 read_count;
} DreadzoneInventorySetupContext;
enum DreadzoneInventorySetupResult {
    DREADZONE_INVENTORY_SETUP_COMPLETED=0,
    DREADZONE_INVENTORY_SETUP_INVALID_STORAGE=-1,
    DREADZONE_INVENTORY_SETUP_UNSUPPORTED_MODE=-2,
    DREADZONE_INVENTORY_SETUP_NEEDS_READ_MEMORY=-3,
    DREADZONE_INVENTORY_SETUP_NEEDS_POOL=-4,
    DREADZONE_INVENTORY_SETUP_UNSUPPORTED_TABLE=-5
};
/* Complete 006909C8 CPU setup for mode0 fresh default loadout, existing mode0
 * inventory, and mode1 indexed reset. Executes genuine CPU grant, ammunition,
 * slot allocation/consumption and root inventory reset algorithms.
 *
 * Pool guest base is001D3F90; records contain guest32 pointers. Caller-supplied
 * immutable read views map actual globals and tables, at most64 disjoint views.
 * Global bindings: mode0021E824, level0021E828, selection0021DF64 and byte00171B66.
 * Tables: definitions003BF460 withB0 stride; progression003BE158 and slots003BDE38
 * with28 stride (ten words); ammunition bonus003BDDE8 with4 stride. Table contents
 * are supplied by the caller, never embedded as extracted game data.
 * All record/pool/read backing and guest spans must be disjoint and nonwrapping.
 * Context and descriptor objects must be readable and outside every backing.
 * Numeric wrapping objects are rejected before access; nonwrapping pointers
 * must still refer to real allocations. No concurrent input changes are allowed.
 * Public preflight refuses errors without changing record, pool or read views.
 * Shared slot counts at inventory+21..28 are preserved by reset. A fresh BSS
 * pool must be zero-initialized by its owner; the reset is not credited for it.
 *
 * Fresh grants use real level0/no-message arguments. Native positive progression
 * 006BE4E8 and paid notifications006BFFC0/006BFF10 remain unavailable; no fake
 * implementations substitute them. Other modes are refused. This prepares an
 * inventory dependency; it does not construct an actor or execute a game frame.
 */
int dreadzone_inventory_006909C8(const DreadzoneInventorySetupContext *);

/* Internal implementations require the public entry's complete preflight.
 * Calling these directly is outside the bounded native API. */
#define SETUP_CTX const DreadzoneInventorySetupContext *ctx,
#define SETUP_ONLY const DreadzoneInventorySetupContext *ctx
#define SETUP_PASS ctx,
#define SETUP_ARG ctx
s32 func_L01_005688A0(SETUP_ONLY);
s32 func_L01_006BE050(SETUP_CTX s32 item);
s32 func_L01_006BE198(SETUP_CTX const u8 *,s32 item);
void func_L01_006BE208(SETUP_CTX u8 *,s32 item,s32 amount);
s32 func_L01_006BE240(SETUP_CTX const u8 *,s32 item);
s32 func_L01_006BE270(SETUP_CTX const u8 *,s32 item);
void func_L01_006BE480(SETUP_CTX u8 *,s32 item,s32 value);
void func_L01_006BE0D0(SETUP_CTX u8 *,s32 item,s32 level);
const u8 *func_L01_006BF200(SETUP_CTX s32 item,s32 variant);
u32 func_L01_006BF278(SETUP_CTX const u8 *,s32 item);
void func_L01_006BF2D0(SETUP_CTX u8 *,s32 item,s32 level,s32 preserve_amount,s32 force_slot);
s32 func_L01_006C0090(SETUP_CTX const u8 *,s32 slot);
void func_L01_006C01F8(SETUP_CTX u8 *,s32 slot,s32 cost,s32 item);
s32 func_L01_006C04A8(SETUP_CTX u8 *,s32 slot);
s32 func_L01_006C0740(SETUP_CTX s32 item,s32 level);
s32 func_L01_006C0768(SETUP_CTX u8 *,s32 item,s32 slot);
s32 func_L01_006C0C60(SETUP_CTX const u8 *,s32 item,s32 slot);
void func_L01_006909C8(SETUP_CTX u8 *record);
u8 dreadzone_setup_read8(const DreadzoneInventorySetupContext *,u32 address);
u32 dreadzone_setup_read32(const DreadzoneInventorySetupContext *,u32 address);
const u8 *dreadzone_setup_table(const DreadzoneInventorySetupContext *,u32 address);
u32 dreadzone_setup_word(const u8 *);
s32 dreadzone_setup_half(const u8 *);
void dreadzone_setup_store16(u8 *,u16);
void dreadzone_setup_store32(u8 *,u32);
void dreadzone_setup_reset(const DreadzoneInventorySetupContext *,u8 *);
void dreadzone_setup_unavailable(void);
#endif
