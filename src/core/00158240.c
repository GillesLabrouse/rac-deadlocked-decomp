#include "common.h"

extern s32 *D_0021DCA8;
extern s32 D_0021DCAC;
extern void func_00127438(void *, void *);

void func_00158240(s32 *results, s32 index) {
    D_0021DCAC = index;
    D_0021DCA8 = results;
    results[index + 1] = 0;
    results[0] = 0;
    func_00127438(results, (u8 *)results + 3);
    results = &results[index + 1];
    func_00127438(results, (u8 *)results + 3);
}
