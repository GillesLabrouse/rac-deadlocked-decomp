#include "dreadzone_class_tables.h"

/* Source-only bridge for the bounded integer differential checker. This calls
 * genuine recovered implementations; it supplies no retail callee substitute. */
#ifdef _WIN32
#define DIFFERENTIAL_EXPORT __declspec(dllexport)
#else
#define DIFFERENTIAL_EXPORT
#endif

DIFFERENTIAL_EXPORT u32 dreadzone_class_differential_abi(void) {
    return 1;
}
DIFFERENTIAL_EXPORT s32 dreadzone_class_differential_publish(
    const DreadzoneTableView *views,u32 count,u32 definition,u32 textures,
    u32 remap,s32 objectClass) {
    DreadzoneTableContext context;
    context.views=views;
    context.view_count=count;
    return dreadzone_class_publish_definition(&context,definition,textures,remap,objectClass);
}
