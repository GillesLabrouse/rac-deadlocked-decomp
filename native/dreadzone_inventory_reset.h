#ifndef DEADLOCKED_DREADZONE_INVENTORY_RESET_H
#define DEADLOCKED_DREADZONE_INVENTORY_RESET_H
#include "common.h"

typedef struct DreadzoneInventoryMemory {
    u8 *storage;
    u32 guest_address, bytes;
} DreadzoneInventoryMemory;
enum {
    DREADZONE_INVENTORY_RESET_BYTES = 0x12B0,
    DREADZONE_INVENTORY_RESET_COMPLETED = 0,
    DREADZONE_INVENTORY_RESET_INVALID = -1
};
/* Complete 006C0D88 reset, including the actual 0055D2F8 clear algorithm.
 * Guest storage must be 16-aligned and its declared range must fit below
 * 0x80000000, avoiding signed address-add exceptions. Host storage may be
 * unaligned. Unwritten bytes remain intact. This neither grants items nor
 * constructs an actor. Errors do not mutate storage. */
int dreadzone_inventory_006C0D88(const DreadzoneInventoryMemory *);
/* Bounded aligned domain of the complete PS2 helper: nonnegative lengths
 * divisible by16 and guest16 alignment. An empty aligned clear is a no-op.
 * Prefix/tail stores for other layouts remain in the PS2 draft only. */
int dreadzone_clear_0055D2F8(const DreadzoneInventoryMemory *, s32 bytes);
#endif
