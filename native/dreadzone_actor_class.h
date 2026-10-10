#ifndef DEADLOCKED_DREADZONE_ACTOR_CLASS_H
#define DEADLOCKED_DREADZONE_ACTOR_CLASS_H
#include "common.h"

/* Read-only guest mappings for actual L01 class/model tables. Actor storage
 * is separately supplied host memory; all serialized pointers remain u32. */
typedef struct DreadzoneClassView {
    u32 guest_base;
    u32 bytes;
    const u8 *storage;
} DreadzoneClassView;
typedef struct DreadzoneClassContext {
    const DreadzoneClassView *views;
    u32 view_count;
    u32 environment;
} DreadzoneClassContext;
enum DreadzoneClassResult {
    DREADZONE_CLASS_INVALID = 0,
    DREADZONE_CLASS_INITIALIZED = 1,
    DREADZONE_CLASS_NEEDS_CLEANUP = 2,
    DREADZONE_CLASS_NEEDS_MODEL_SETUP = 3
};

/* Supports class mismatch and mapped definitions with byte 8 == 0. Other
 * branches return their real outstanding dependency before changing actor.
 * Success initializes one 0x100-byte actor; it does not allocate an actor,
 * create model state or execute an arbitrary callback stored in the actor. */
int dreadzone_actor_initialize_class_slice(const DreadzoneClassContext *context,
    u8 *actor, u32 actor_bytes, s32 object_class, s32 skip_cleanup);
const u8 *dreadzone_class_translate(const DreadzoneClassContext *, u32, u32);
void dreadzone_actor_pack_environment(u8 *, u32, u32, u32, u32);
void dreadzone_actor_initialize_environment(u8 *, u32, u8, int);
#endif
