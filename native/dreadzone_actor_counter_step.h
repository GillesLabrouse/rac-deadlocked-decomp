#ifndef DREADZONE_ACTOR_COUNTER_STEP_H
#define DREADZONE_ACTOR_COUNTER_STEP_H
#include "common.h"
/* Complete L01 word-counter leaf. Returns -1 for a null host pointer;
 * original successful low-word returns are 0, 1 or 2. No full-GPR contract. */
s32 dreadzone_actor_counter_step_0055D148(s32 *counter);
#endif
