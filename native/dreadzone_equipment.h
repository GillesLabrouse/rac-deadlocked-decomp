#ifndef DEADLOCKED_DREADZONE_EQUIPMENT_H
#define DEADLOCKED_DREADZONE_EQUIPMENT_H
#include "dreadzone_inventory_setup.h"
typedef struct DreadzoneEquipmentMemory {
    u8 *storage;
    u32 guest_address,bytes;
} DreadzoneEquipmentMemory;
typedef struct DreadzoneEquipmentContext {
    DreadzoneInventorySetupContext inventory;
    const DreadzoneEquipmentMemory *mutable_views;
    u32 mutable_count;
} DreadzoneEquipmentContext;
enum DreadzoneEquipmentResult {
    DREADZONE_EQUIPMENT_COMPLETED=0,
    DREADZONE_EQUIPMENT_SELECTION_REJECTED=1,
    DREADZONE_EQUIPMENT_INVALID_STORAGE=-1,
    DREADZONE_EQUIPMENT_NEEDS_MEMORY=-2,
    DREADZONE_EQUIPMENT_NEEDS_INVENTORY=-3,
    DREADZONE_EQUIPMENT_UNSUPPORTED_UI=-4,
    DREADZONE_EQUIPMENT_UNSUPPORTED_ITEM=-5,
    DREADZONE_EQUIPMENT_UNSAFE_FIELD_ALIAS=-6
};
/* Complete CPU equipment maintenance 00690B60, actual slot deletion 006C0A58,
 * marker clear 00690C88, pure reset 005E9258, and bounded selection 00688578.
 * Both mode predicates are real recovered CPU routines. Existing inventory
 * grant/level helpers execute actual algorithms with verified no-message or
 * level -1 arguments; no UI calls or effects are captured.
 *
 * Record and inventory pool use inventory's guest32 bindings. Immutable views
 * supply only the globals/tables actually accessed on the selected path.
 * Mutable views map markers 00171B67..6C, selectors 00220810..1F, recent rows
 * 0020C690 (C stride), and state words 003301E0 (B8 stride). All complete guest
 * and host spans, including descriptors, must be disjoint and nonwrapping.
 * Context and descriptor objects must be readable actual allocations otherwise.
 * Record and pool bindings stay below guest address 80000000, as required by
 * the inventory interface. Inputs remain stable; errors refuse before mutation.
 *
 * Maintenance supports real 266D=1 when 2318 differs from every active marker.
 * A setter retaining 266D!=0 after its real store needs unresolved UI dispatch
 * and is refused. Signed category -2 genuinely overwrites and clears that byte
 * before the retail read; it is supported. Flag 0 selection is CPU work, not
 * fresh-player constructor proof: real 006A6598 sets 266D=1. Its full PS2 source
 * retains virtual +4C and
 * 005D7558, whose 005E93D8 UI dependency chain remains unrecovered.
 * No actor construction, rendering, transport or playable frame is substituted.
 */
int dreadzone_equipment_00690B60(const DreadzoneEquipmentContext *);
int dreadzone_equipment_00688578(const DreadzoneEquipmentContext *,s32 item);
int dreadzone_equipment_006C0A58(const DreadzoneEquipmentContext *,s32 item,s32 restore_slots);
int dreadzone_equipment_00690C88(const DreadzoneEquipmentContext *);

/* Internal CPU implementations require public preflight; direct calls are
 * outside the bounded API, and unsupported invariants fail closed. */
#define EQ_CTX const DreadzoneEquipmentContext *ctx,
#define EQ_ONLY const DreadzoneEquipmentContext *ctx
#define EQ_PASS ctx,
#define EQ_ARG ctx
void func_L01_00690B60(EQ_CTX u8 *record);
s32 func_L01_00688578(EQ_CTX u8 *record,s32 item);
s32 func_L01_006C0A58(EQ_CTX u8 *inventory,s32 item,s32 restore_slots);
void func_L01_00690C88(EQ_ONLY);
void func_L01_005E9258(EQ_CTX s32 index);
s32 func_L01_006BDDC0(s32 item);
s32 func_L01_006BE040(s32 item);
u8 *dreadzone_equipment_mutable(const DreadzoneEquipmentContext *,u32 address,u32 bytes);
u8 *dreadzone_equipment_inventory(const DreadzoneEquipmentContext *,const u8 *record);
u32 dreadzone_equipment_mutable_word(const DreadzoneEquipmentContext *,u32 address);
void dreadzone_equipment_store_word(const DreadzoneEquipmentContext *,u32 address,u32 value);
#endif
