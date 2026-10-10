#ifndef DEADLOCKED_DREADZONE_BULK_CLEAR_H
#define DEADLOCKED_DREADZONE_BULK_CLEAR_H

#include "common.h"

enum {
    DREADZONE_BULK_START = 0x00222900,
    DREADZONE_BULK_END = 0x003A0B40,
    DREADZONE_BULK_BYTES = DREADZONE_BULK_END - DREADZONE_BULK_START
};

/* This descriptor is a host interface, not a serialized guest structure.
 * Guest addresses and extents stay 32-bit; storage is a host allocation.
 * The caller supplies at least bytes readable/writable bytes of storage and
 * keeps that allocation alive until the binding is replaced or invalidated.
 */
typedef struct DreadzoneBulkArena {
    u32 guest_base;
    u32 bytes;
    u8 *storage;
} DreadzoneBulkArena;

u8 *dreadzone_bulk_translate(const DreadzoneBulkArena *arena, u32 address, u32 bytes);
int dreadzone_bulk_bind(const DreadzoneBulkArena *arena);
int dreadzone_bulk_clear_range(const DreadzoneBulkArena *arena, u32 start, u32 end);
void func_L01_00529280(void);
/* The retail helper has no capacity argument. Provide at least 16 bytes for
 * nonpositive counts, or bytes rounded up to 16 for positive counts; counts
 * must permit signed subtraction by 16. The bounded arena wrapper preflights
 * every actual startup store before invoking it.
 */
void func_L01_0055D2D0(u8 *destination, s32 bytes);

#endif
