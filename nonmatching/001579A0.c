/* cflags: -mno-split-addresses */
#include "common.h"

/* Draft: the function-address low half is scheduled before the ra save,
 * whereas retail saves ra first. Keep this outside src until scheduling matches. */
typedef struct Type1 {
    s32 f0;
    s32 f4;
    u32 f8;
} Type1;

extern Type1 *D_0021DA24_[];
extern s32 D_0021DE10_[];
extern s32 func_00157738(void *, void (*)(void));
extern void func_00157888(s32);
extern void func_01E9A000(void);

s32 func_001579A0(void) {
    Type1 *image = D_0021DA24_[0];
    s32 entry = func_00157738((void *)(image->f8 + (u32)image), func_01E9A000);
    func_00157888(D_0021DE10_[0]);
    return entry;
}
