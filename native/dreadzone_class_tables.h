#ifndef DEADLOCKED_DREADZONE_CLASS_TABLES_H
#define DEADLOCKED_DREADZONE_CLASS_TABLES_H
#include "common.h"

/* Writable, disjoint guest ranges. Serialized addresses are always u32. */
typedef struct DreadzoneTableView {
    u32 guest_base,bytes;
    u8 *storage;
} DreadzoneTableView;
typedef struct DreadzoneTableContext {
    const DreadzoneTableView *views;
    u32 view_count;
} DreadzoneTableContext;
enum DreadzoneTableResult {
    DREADZONE_TABLE_INVALID=0,
    DREADZONE_TABLE_COMPLETED=1,
    DREADZONE_TABLE_NEEDS_GEOMETRY=2
};
/* Only the verified fill/clear writes of 005742C8. Its other initialization
 * callees and global publications remain outstanding in the PS2 source. */
int dreadzone_class_reset_table_slice(const DreadzoneTableContext *);
int dreadzone_class_publish_dispatch(const DreadzoneTableContext *,s32,s32);
/* Actual 00576BC0 relocation, bounded to 8192 writes per operation. Geometry
 * conversion 00576AA8 is preserved as a dependency, refused before mutation. */
int dreadzone_class_relocate_definition(const DreadzoneTableContext *,u32,u32,u32,s32);
/* Actual 00576EF8 publication and relocation, including dispatch selection.
 * 10240 classes, 240 normal slots, 128 alternative slots are retail limits. */
int dreadzone_class_publish_definition(const DreadzoneTableContext *,u32,u32,u32,s32);
#endif
