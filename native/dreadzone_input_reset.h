#ifndef DEADLOCKED_DREADZONE_INPUT_RESET_H
#define DEADLOCKED_DREADZONE_INPUT_RESET_H

#include "dreadzone_input.h"

enum { DREADZONE_INPUT_RECORD_COUNT = 8 };

/* Actual core.bss pool. Native storage must begin zero-filled separately from
 * the selective reset helper: the helper does not initialize all these bytes.
 */
extern InputRecord5C0 D_001EE600[DREADZONE_INPUT_RECORD_COUNT];

void func_L01_005BD950(InputRecord5C0 *record);
void func_L01_005BD710(InputRecord5C0 *record);
void func_L01_005BD790(InputRecord5C0 *record, s32 delay);

#endif
