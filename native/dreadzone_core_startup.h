#ifndef DEADLOCKED_DREADZONE_CORE_STARTUP_H
#define DEADLOCKED_DREADZONE_CORE_STARTUP_H
#include "common.h"

/* Storage at00172160. Only the accessed extent is represented; this is not a
 * claim that the whole game global has this type or size. Guest pointers stay
 * 32-bit even when the host uses 64-bit pointers. */
typedef union DreadzoneCoreStartupBlock {
    u8 bytes[0x1688];
    u32 words[0x1688 / 4];
} DreadzoneCoreStartupBlock;

typedef struct DreadzoneCoreStartupContext {
    DreadzoneCoreStartupBlock *block;
    s32 level_0021DE10;
    u32 state_0021DFE8;
    u32 callback_0021DB94;
    s32 selected_002120F4;
    u8 flag_0021DFA4;
} DreadzoneCoreStartupContext;

enum DreadzoneCoreStartupResult {
    DREADZONE_CORE_STARTUP_COMPLETE = 0,
    DREADZONE_CORE_STARTUP_INVALID_STORAGE = -1,
    DREADZONE_CORE_STARTUP_NEEDS_LEVEL_ZERO = -2,
    DREADZONE_CORE_STARTUP_NEEDS_NETWORK = -3,
    DREADZONE_CORE_STARTUP_NEEDS_CALLBACK = -4
};

/* The actual Dreadzone entry stores00223E40 into21DFE8 at005281E4; the
 * pre-loop initializer sets mode258 to1 at00665D78 and calls A170/AA58 at
 * 00665D84/00665D8C. The null-callback condition is an explicit supported
 * domain, not an assertion that the complete game startup leaves it null.
 * These are bounded slices of actual functions, not a complete boot chain.
 * A170 always clears byte1681 before checking its domain, like retail. Other
 * unsupported paths fail before modifying storage. The returned status is a
 * host boundary status; the supported retail A170/AA58 return value is zero.
 * B498 accepts mode258==1, an existing nonzero guest state, and a null callback.
 * The existing-state helper executes the actual F4A0 early branch (return1);
 * it does not create, deserialize, or otherwise dereference that state. */
int dreadzone_core_0015A170_slice(DreadzoneCoreStartupContext *);
int dreadzone_core_0015AA58_slice(DreadzoneCoreStartupContext *);
int dreadzone_core_0015B498_slice(DreadzoneCoreStartupContext *);
int dreadzone_core_0015F4A0_existing_slice(const DreadzoneCoreStartupContext *, s32 *);

#endif
