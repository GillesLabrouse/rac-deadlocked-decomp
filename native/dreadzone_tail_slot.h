#ifndef DEADLOCKED_DREADZONE_TAIL_SLOT_H
#define DEADLOCKED_DREADZONE_TAIL_SLOT_H
#include "common.h"
typedef struct DreadzoneTailSlotRequest {
    u8 *storage;
    u32 guest_base,storage_bytes,index;
} DreadzoneTailSlotRequest;
typedef struct DreadzoneTailSlotInfo {
    u32 slot_guest,entry_guest,entry_pointer,clear_guest,return_low;
    u32 read_count,read_guest,did_clear;
} DreadzoneTailSlotInfo;
/* Bounded RAM/low-return adapter for the actual L01 words at 0x00535A30.
 * The selected caller passes the unsigned-byte result of 0x006A9630, so only
 * indices 0..255 are accepted. The leaf reads P from
 * 0x230280 + index*0x470 + 0x124; if P is nonzero it clears P+0x98 and returns
 * low32(P); otherwise it returns zero and does not store. All guest arithmetic
 * follows 32-bit wrap and each reached word must be aligned/mapped. Storage is
 * one stable exclusive writable guest range <=16 MiB below 0x10000000. The
 * request is copied before accesses and may reside in backing RAM; info must
 * be disjoint from request and backing. Refusal leaves RAM and info unchanged.
 * This models RAM and low32 V0 only, not original stack/banks/HI/LO/timing or
 * a canonical source identity. */
int dreadzone_tail_slot_00535A30(const DreadzoneTailSlotRequest *,DreadzoneTailSlotInfo *);
#endif
