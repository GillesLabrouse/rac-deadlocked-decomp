#ifndef DEADLOCKED_DREADZONE_EQUIPMENT_SELECTION_H
#define DEADLOCKED_DREADZONE_EQUIPMENT_SELECTION_H
#include "dreadzone_equipment_ui.h"

enum DreadzoneEquipmentSelectionResult {
    DREADZONE_EQUIPMENT_SELECTION_COMPLETED=0,
    DREADZONE_EQUIPMENT_SELECTION_TRANSACTION_REJECTED=1,
    DREADZONE_EQUIPMENT_SELECTION_INVALID_STORAGE=-10,
    DREADZONE_EQUIPMENT_SELECTION_NEEDS_MEMORY=-11,
    DREADZONE_EQUIPMENT_SELECTION_UNSUPPORTED_CATEGORY=-12,
    DREADZONE_EQUIPMENT_SELECTION_UNSUPPORTED_HANDLER=-13,
    DREADZONE_EQUIPMENT_SELECTION_UNSUPPORTED_ACTOR=-14,
    DREADZONE_EQUIPMENT_SELECTION_NO_TRANSACTION_MEMORY=-15
};
/* Transactional composition of the complete 00688578 CPU setter with its
 * real virtual +4C getter and recovered 005D7558/005E93D8 queue/state effects.
 * No transport, renderer, audio, callback capture or actor construction runs.
 *
 * Borrow the UI context's validated, disjoint, nonwrapping guest32 mappings.
 * Both UI rows initially have zero +24/+28. The actor guest base is 00368F60
 * or 0036C150; record byte266D must be nonzero. A selected category-zero item
 * uses actual definition003BF460 + item*B0 and the real mode/level predicates.
 * The mapped vtable word +4C must resolve to genuine006A9630 or00690EA0. The
 * former returns byte2FEA (0..1 for the supported UI path); the latter really
 * returns -1, whose unresolved UI path is refused without publishing stores.
 * The actor base and returned player index need not agree: reset006A6598 clears
 * byte2FEA, whereas constructor00668070 writes its index to word2F10.
 *
 * Every mapped RAM image is copied into transaction storage, retaining the
 * exact guest addresses and read/write permissions. Actual ordered setter,
 * getter and UI algorithms execute on those images; errors discard them.
 * A validation-only public UI call with item1 first checks descriptors and
 * fresh state without stores or gameplay dispatch. Original LW of the virtual
 * table entry requires a naturally aligned guest table pointer.
 * Successful writable images are then published. Caller exclusive access and
 * stable readable allocations are required; publication is not synchronization
 * for concurrent observers. Total mapped bytes are bounded to16MiB. Returned
 * 1 is a genuine setter rejection; a UI rejection is ignored by the setter as
 * in retail and can still leave the genuine category-zero record store.
 * Negative UI statuses retain their UI enum values; selection-specific errors
 * use -10..-15. Full constructor, other categories and secondary/reentrant UI
 * domains remain unsupported. Complete original PS2 setter stays separate.
 */
int dreadzone_equipment_selection_00688578(const DreadzoneEquipmentUiContext *,s32 item);
s32 func_L01_006A9630(const u8 *record);
s32 func_L01_00690EA0(const u8 *record);
#endif
