#ifndef DEADLOCKED_DREADZONE_STATE_INIT_H
#define DEADLOCKED_DREADZONE_STATE_INIT_H

#include "common.h"

enum { DREADZONE_STATE_SLOTS = 3 };

/* The table header is a signed count. Each following record is eight bytes;
 * only f0 is modified by this initializer. All stored addresses are guest32.
 */
typedef struct DreadzoneStateEntry {
    u32 f0;
    u32 f4;
} DreadzoneStateEntry;

#ifdef DEADLOCKED_NATIVE
#define DREADZONE_STATE_ABS
#else
#define DREADZONE_STATE_ABS __attribute__((section(".data")))
#endif
extern s32 D_0021DE10 DREADZONE_STATE_ABS;
extern u8 D_0021E868;
extern u32 D_0021DE98[DREADZONE_STATE_SLOTS];
extern u32 D_0021DEA8[DREADZONE_STATE_SLOTS];
extern u32 D_0021DEB8[DREADZONE_STATE_SLOTS];
extern u32 D_0021DE88 DREADZONE_STATE_ABS;
extern u32 D_0021DE8C DREADZONE_STATE_ABS;
extern u32 D_0021DE90 DREADZONE_STATE_ABS;
extern s32 D_0021DEC4 DREADZONE_STATE_ABS;
extern u32 D_0021E734;
extern u32 D_0021E738;
#undef DREADZONE_STATE_ABS

void func_L01_0055DCF8(s32 slot, u32 base, u32 table);
void func_L01_0055E0A0(s32 slot);
void func_L01_0055DFE8(void);

#ifdef DEADLOCKED_NATIVE
/* The caller provides at least bytes readable/writable bytes of host storage
 * and keeps that allocation alive while bound. Guest addresses map only into
 * this allocation; they are never cast to host pointers.
 */
typedef struct DreadzoneStateArena {
    u32 guest_base;
    u32 bytes;
    u8 *storage;
} DreadzoneStateArena;

int dreadzone_state_bind(const DreadzoneStateArena *arena);
/* Validate the signed-count table and every object read before publishing
 * this slot or modifying its records. Return -1 for an unmapped dependency.
 * The actual void entry/helper fail explicitly if their mapping is missing.
 */
int dreadzone_state_init_slot(s32 slot, u32 base, u32 table);
#endif

#endif
