/* Complete 32-byte selected-item getter, NONMATCHING baseline SN draft. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_direction.h"
#endif
#include "../include/dreadzone_actor_direction_internal.h"
s32 func_L01_006739B0(const u8 *record) {
    return D_WORD(record,0x2314)==2?(s32)D_WORD(record,0x2318):-1;
}
