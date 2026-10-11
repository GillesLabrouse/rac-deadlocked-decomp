/* cflags: -G0 */
#include "common.h"

extern s32 D_00163E74;
extern s64 func_001264E8(s32 global, s32 text, s32 position, s32 base);

s64 func_00126718(s32 text, s32 position, s32 base) {
    return func_001264E8(D_00163E74, text, position, base);
}
