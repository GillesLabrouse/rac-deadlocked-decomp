#ifndef DEADLOCKED_DREADZONE_SLOT_WORD_H
#define DEADLOCKED_DREADZONE_SLOT_WORD_H
#include "common.h"
typedef struct DreadzoneSlotWordRequest {
    u8 *storage;
    u32 guest_base,storage_bytes,index;
} DreadzoneSlotWordRequest;
typedef struct DreadzoneSlotWordInfo {
    u32 slot_guest,entry_guest,entry_pointer,child_pointer_guest,child_pointer;
    u32 value_guest,store_guest,return_low,read_count,read_guest[3],did_store;
} DreadzoneSlotWordInfo;
/* Complete bounded RAM/low-V0 effect of L01 535A58. Caller index0..255 is
 * obtained from an unsigned byte. Slot230280+index*470 contains P at+124.
 * Null P leaves V0=230280 (not the indexed slot). Otherwise read Q=P[24],
 * copy the raw word Q[10] to P[98], and return low Q. Offsets are bytes;
 * these accesses establish no original record/type or allocation ownership.
 * Actual guest32 address arithmetic is modulo32; every reached LW/SW must
 * be aligned/mapped. One stable exclusive writable main range<=16MiB below
 * 10000000 separates original stack. Request is captured and may alias RAM;
 * info is disjoint from request/RAM. Reads and checks precede the sole store.
 * Refusal preserves RAM/info. Original stack, full banks, HI/LO and physical
 * timing/traps are excluded. No canonical matching source/percentage credit. */
int dreadzone_slot_word_00535A58(const DreadzoneSlotWordRequest *,DreadzoneSlotWordInfo *);
#endif
