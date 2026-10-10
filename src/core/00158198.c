#include "common.h"

extern s32 *D_0021DCA8;
extern s32 D_0021DCAC;
extern s32 D_0021DD20;
extern u8 D_001BD480[];
extern u8 D_001A4768[];
extern s32 func_0012A918(void *);
extern void func_00127568(void *, void *);
extern void func_0011D5D8(u8 *, ...);

s32 func_00158198(void) {
    if (D_0021DCA8 == NULL) {
        return 1;
    }
    if (func_0012A918(D_001BD480) != 0) {
        return 0;
    }
    func_00127568(D_0021DCA8, (u8 *)(D_0021DCAC * 4 + (u32)D_0021DCA8) + 8);
    if (D_0021DCA8[0] == 0xFFFFFFFF &&
        D_0021DCA8[D_0021DCAC + 1] == D_0021DCA8[0]) {
        D_0021DCA8 = NULL;
        return 1;
    }
    if (D_0021DD20 == 0) {
        func_0011D5D8(D_001A4768);
    }
    return 0;
}
