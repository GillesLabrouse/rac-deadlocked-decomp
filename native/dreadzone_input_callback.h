#ifndef DEADLOCKED_DREADZONE_INPUT_CALLBACK_H
#define DEADLOCKED_DREADZONE_INPUT_CALLBACK_H

#include "dreadzone_input.h"

/* Actual callback context begins at actor+2FEC. Fixed-width guest fields and
 * four-element snapshots preserve its observed 1E0-byte span. The stream at
 * byte19 overlaps the upper bytes of word18; word18's low nibble is the four
 * frame flags. There are no host pointers stored in this context.
 */
typedef struct DreadzoneInputContext1E0 {
    u16 f0[4];
    u16 f8[4];
    u16 f10[4];
    u32 f18;
    u8 pad1C[0x20];
    s32 f3C;
    u8 pad40[0x198];
    s32 f1D8;
    s32 f1DC;
} DreadzoneInputContext1E0;

extern s32 D_002208A8;
extern s32 D_002208AC;

void func_L01_006B5CB8(InputRecord5C0 *, const u8 *, s32,
                      DreadzoneInputContext1E0 *);
/* Actual bounded MSB-first bit writer; no capacity check is present in retail.
 * Host tests require nonnegative cursor, sufficient output and widths <=32.
 */
void func_L01_006B59C8(u8 *, s32, s32 *, u8);

#endif
