#ifndef DEADLOCKED_DREADZONE_BYTE_FILL_H
#define DEADLOCKED_DREADZONE_BYTE_FILL_H

#include "common.h"

enum { DREADZONE_BYTE_FILL_BYTES = 0x40000 };

typedef struct DreadzoneByteArena {
    u32 guest_base;
    u32 bytes;
    u8 *storage;
} DreadzoneByteArena;

/* The caller owns at least bytes writable bytes and keeps the allocation alive
 * until the binding is replaced or invalidated. No guest layout is widened.
 */
int dreadzone_byte_fill_bind(const DreadzoneByteArena *arena);
int dreadzone_byte_fill_guest(u32 address, s32 value, u32 count);

/* The actual core helper returns the original destination and writes exactly
 * count bytes, using the low byte of value. The caller supplies the capacity.
 */
void *func_0011A518(void *destination, s32 value, u32 count);
void func_L01_005D6CB0(void);
s32 func_L01_00666600(void);
s32 func_00510EB0(void);

extern u32 D_002437C4;
extern s32 D_0021DE10;

#endif
