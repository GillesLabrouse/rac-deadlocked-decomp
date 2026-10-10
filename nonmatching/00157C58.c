#include "common.h"

typedef void (*EntryPoint)(void);

extern u8 D_0021DC90[];
extern u8 D_0021DC98[];
extern u8 D_0021DCA0[];
extern u8 D_0021DE69;
extern u8 D_0021DE69_[];
extern s32 D_0021DE80;
extern s32 D_0021DC84;
extern s32 func_0011AE00(u8 *, u8 *, s32);
extern s32 func_0011A8A0(u8 *, u8 *);
extern void func_00157B30(u8 *, u8 *);
extern void func_0012EEE0(void);
extern void func_0045CB88(void);
extern void func_00437EA8(void);
extern EntryPoint func_001579A0(void);
extern void func_00126E20(s32);

/* Draft of argument parsing and the resident entry-point loop.
 * Global-address materialization and register scheduling still differ from
 * retail; this draft is excluded from matching and native build targets.
 */
void func_00157C58(s32 count, u8 **arguments) {
    EntryPoint entry;
    u8 changed;
    func_0012EEE0();
    D_0021DE69 = 0;
    if (count > 0) {
        do {
            count--;
            if (func_0011AE00(*arguments, D_0021DC90, 5) == 0) {
                func_00157B30(*arguments + 5, D_0021DC98);
                changed = D_0021DE69;
                goto changed_argument;
            }
            if (func_0011A8A0(*arguments, D_0021DC98) == 0) {
                changed = D_0021DE69_[0];
                D_0021DE80 = 2;
                goto changed_argument;
            }
            if (func_0011A8A0(*arguments, D_0021DCA0) == 0) {
                D_0021DC84 = 1;
                changed = D_0021DE69;
changed_argument:
                D_0021DE69_[0] = changed + 1;
            }
            arguments++;
        } while (count != 0);
    }
    func_0045CB88();
    entry = (EntryPoint)((u8 *)func_00437EA8 + 8);
    for (;;) {
        entry();
        entry = func_001579A0();
        func_00126E20(0);
        func_00126E20(2);
    }
}
