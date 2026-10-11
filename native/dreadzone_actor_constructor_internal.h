#ifndef DEADLOCKED_DREADZONE_ACTOR_CONSTRUCTOR_INTERNAL_H
#define DEADLOCKED_DREADZONE_ACTOR_CONSTRUCTOR_INTERNAL_H
#include "dreadzone_actor_constructor.h"
/* Internal regions; only the complete outer constructor publishes to callers. */
int dreadzone_constructor_prefix(u8 *,u32,u32,u32,u32,u32,const u32[3]);
int dreadzone_constructor_orientation(const float[3],const float[4],float[32]);
#endif
