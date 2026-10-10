#ifndef DEADLOCKED_DREADZONE_EQUIPMENT_UI_H
#define DEADLOCKED_DREADZONE_EQUIPMENT_UI_H
#include "dreadzone_equipment.h"
typedef struct DreadzoneEquipmentUiContext {
    const DreadzoneEquipmentContext *equipment;
    const DreadzoneEquipmentMemory *extra_writes;
    u32 extra_write_count;
} DreadzoneEquipmentUiContext;
enum DreadzoneEquipmentUiResult {
    DREADZONE_EQUIPMENT_UI_COMPLETED=0,
    DREADZONE_EQUIPMENT_UI_ITEM_REJECTED=1,
    DREADZONE_EQUIPMENT_UI_INVALID_STORAGE=-1,
    DREADZONE_EQUIPMENT_UI_NEEDS_MEMORY=-2,
    DREADZONE_EQUIPMENT_UI_UNSUPPORTED_SECONDARY=-3,
    DREADZONE_EQUIPMENT_UI_UNSUPPORTED_FORCE=-4,
    DREADZONE_EQUIPMENT_UI_INVALID_ITEM=-5
};
/* Complete 005D7558/005E93D8 CPU queue/state algorithms in a bounded fresh
 * selection domain: index 0..1, force 1, initially zero state fields +24/+28,
 * and a category-zero definition with nonzero +28 (including actual item 2).
 * Category rejection returns the real item-rejected result without mutation.
 * All selected-path requirements are checked before stores. The existing
 * equipment/inventory records, reads and writable views are borrowed; extra
 * writable views supply UI summary flags as needed. All host/guest backing,
 * contexts and descriptor arrays must be disjoint and nonwrapping. Otherwise
 * nonwrapping pointers must designate actual readable/writable allocations.
 * The caller keeps all inputs stable throughout a call.
 *
 * Bindings: state rows 00330148 (B8 stride), recent queues 0020C690 (C stride),
 * mode 0021E824, level 0021E828, query bytes 0031B314 (54 stride), definitions
 * 003BF460 (B0 stride), summary words 003312F0 and 00331354. Reached actor reads
 * retain the real selector's 00368F60 + index*31F0 and fields +2674/+268C.
 * Queue rotation executes the actual wrap-index algorithm, not an RNG capture.
 * The borrowed forward copy preserves disjoint scratch/queue byte effects for
 * lengths 0..12. Original paired-word reads need up to 16 source bytes; their
 * unused lookahead word is also preflighted. Vector alignment/prefetch flags,
 * hardware, renderer and audio behavior are outside this CPU RAM domain.
 * Secondary or reentrant setter paths remain unsupported natively. Complete
 * PS2 sources retain those real callees; this API does not construct an actor
 * or connect the frozen flag-1 setter automatically.
 */
int dreadzone_equipment_ui_005D7558(const DreadzoneEquipmentUiContext *,s32 index,s32 item,s32 force);
int dreadzone_equipment_ui_005E93D8(const DreadzoneEquipmentUiContext *,s32 index,s32 item,s32 force);

/* Internal routines require public preflight; misuse fails closed. */
typedef u32 UiWordPointer;
#define UI_CTX const DreadzoneEquipmentUiContext *ctx,
#define UI_ONLY const DreadzoneEquipmentUiContext *ctx
#define UI_PASS ctx,
#define UI_ARG ctx
s32 func_L01_005D7558(UI_CTX s32,s32,s32);
s32 func_L01_005E93D8(UI_CTX s32,s32,s32);
UiWordPointer func_L01_005EA270(UI_CTX s32);
s32 func_L01_006BDDF0(UI_CTX s32);
s32 func_L01_005D89E8(UI_CTX s32);
s32 func_L01_005EA230(UI_CTX s32);
void func_L01_005E97D8(UI_CTX s32,s32,s32);
s32 func_L01_005D8A08(UI_CTX s32);
s32 func_L01_005D8A40(UI_CTX s32,s32);
s32 func_L01_005D8AE0(UI_CTX s32,s32);
void func_L01_005E9350(UI_CTX s32);
void func_L01_005EA6A8(UI_ONLY);
void func_L01_005F0A40(UI_ONLY);
s32 func_L01_00652FD0(const void *unused,s32 value,s32 lower,s32 upper);
u32 dreadzone_ui_read32(const DreadzoneEquipmentUiContext *,u32);
u8 dreadzone_ui_read8(const DreadzoneEquipmentUiContext *,u32);
void dreadzone_ui_write32(const DreadzoneEquipmentUiContext *,u32,u32);
const u8 *dreadzone_ui_data(const DreadzoneEquipmentUiContext *,u32,u32);
u32 dreadzone_ui_actor(const DreadzoneEquipmentUiContext *,s32);
#endif
