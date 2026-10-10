/* Actual Dreadzone pre-loop table initialization and selection. */
#include "common.h"

#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_state_init.h"
#include <stdlib.h>
static DreadzoneStateArena state_arena;
static int state_is_bound;

static u8 *state_translate(u32 address, u64 bytes) {
    u64 end = (u64)address + bytes;
    u64 arena_end = (u64)state_arena.guest_base + state_arena.bytes;
    if (!state_is_bound || address < state_arena.guest_base ||
        end > arena_end || end > 0x100000000ULL) {
        return NULL;
    }
    return state_arena.storage + ((u64)address - state_arena.guest_base);
}

static u32 state_read_word(const u8 *p) {
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}

static void state_write_word(u8 *p, u32 word) {
    p[0] = (u8)word;
    p[1] = (u8)(word >> 8);
    p[2] = (u8)(word >> 16);
    p[3] = (u8)(word >> 24);
}

int dreadzone_state_bind(const DreadzoneStateArena *arena) {
    state_is_bound = 0;
    if (arena == NULL || arena->storage == NULL ||
        (u64)arena->guest_base + arena->bytes > 0x100000000ULL) {
        return -1;
    }
    state_arena = *arena;
    state_is_bound = 1;
    return 0;
}

int dreadzone_state_init_slot(s32 slot, u32 base, u32 table) {
    u8 *header;
    u32 raw_count;
    s32 count;
    s32 index;
    u32 end;
    if (slot < 0 || slot >= DREADZONE_STATE_SLOTS || (table & 3U) != 0) {
        return -1;
    }
    header = state_translate(table, 4);
    if (header == NULL) {
        return -1;
    }
    raw_count = state_read_word(header);
    /* Decode signed guest32 without an implementation-defined unsigned cast. */
    count = (raw_count & 0x80000000U) != 0 ? -1 - (s32)(~raw_count) : (s32)raw_count;
    end = table + 4U + (raw_count << 3);
    if (count > 0) {
        if (state_translate(table, 4ULL + (u64)(u32)count * 8) == NULL) {
            return -1;
        }
        /* Validate every object byte before changing the slot or a record.
         * These checks belong to the host adapter, not to the retail routine.
         */
        for (index = 0; index < count; index++) {
            u8 *record = state_translate(table + 4U + (u32)index * 8, 8);
            u32 object = end + ((state_read_word(record) >> 2) & 0x3FFCU);
            if (object != 0 && state_translate(object, 4) == NULL) {
                return -1;
            }
        }
    }
    D_0021DE98[slot] = base;
    D_0021DEA8[slot] = table + 4U;
    D_0021DEB8[slot] = end;
    for (index = 0; index < count; index++) {
        u8 *record = state_translate(D_0021DEA8[slot] + (u32)index * 8, 8);
        u32 word = state_read_word(record);
        u32 object = D_0021DEB8[slot] + ((word >> 2) & 0x3FFCU);
        if (object != 0 && state_translate(object, 4)[3] != 0) {
            state_write_word(record, word & 0xFFFF000FU);
        }
    }
    return 0;
}

void func_L01_0055DCF8(s32 slot, u32 base, u32 table) {
    if (dreadzone_state_init_slot(slot, base, table) != 0) {
        abort();
    }
}
#else
typedef struct DreadzoneStateEntry {
    u32 f0;
    u32 f4;
} DreadzoneStateEntry;
extern s32 D_0021DE10 __attribute__((section(".data")));
extern u32 D_0021DE98[3];
extern u32 D_0021DEA8[3];
extern u32 D_0021DEB8[3];
extern u32 D_0021DE88 __attribute__((section(".data")));
extern u32 D_0021DE8C __attribute__((section(".data")));
extern u32 D_0021DE90 __attribute__((section(".data")));
extern s32 D_0021DEC4 __attribute__((section(".data")));
extern u32 D_0021E734;
extern u32 D_0021E738;
extern u8 D_0021E868_[];

void func_L01_0055DCF8(s32 slot, u32 base, u32 table) {
    s32 count = *(s32 *)table;
    s32 index;
    D_0021DE98[slot] = base;
    D_0021DEA8[slot] = table + 4;
    D_0021DEB8[slot] = table + 4 + ((u32)count << 3);
    for (index = 0; index < count; index++) {
        DreadzoneStateEntry *entry = (DreadzoneStateEntry *)(D_0021DEA8[slot] + (u32)index * 8);
        u32 word = entry->f0;
        u8 *object = (u8 *)(D_0021DEB8[slot] + ((word >> 2) & 0x3FFC));
        if (object != NULL && object[3] != 0) {
            entry->f0 = word & 0xFFFF000F;
        }
    }
}
#endif

void func_L01_0055E0A0(s32 slot) {
#ifdef DEADLOCKED_NATIVE
    if (slot < 0 || slot >= DREADZONE_STATE_SLOTS) {
        abort();
    }
#endif
    if (D_0021DE10 >= 0 &&
#ifdef DEADLOCKED_NATIVE
        D_0021E868 == 0 &&
#else
        D_0021E868_[0] == 0 &&
#endif
        slot == 2) {
        return;
    }
    D_0021DE88 = D_0021DE98[slot];
    D_0021DE8C = D_0021DEA8[slot];
    D_0021DE90 = D_0021DEB8[slot];
    D_0021E734 = 0x003B6DB0U + ((u32)slot << 6);
    D_0021E738 = 0x003B6E70U + ((u32)slot << 6);
    D_0021DEC4 = slot;
}
