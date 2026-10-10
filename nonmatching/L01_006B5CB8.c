/* cflags: -mno-split-addresses */
/* Actual L01 controller callback, retail 006B5CB8..006B5FD8 (800 bytes).
 * NONMATCHING. Copies the raw packet's button/stick pairs to four snapshots
 * and emits a compact button-change stream in the actor's 1E0-byte context.
 * No controller, network transport or actor update is substituted here.
 * Preconditions: nonnegative frame/count fields, bounded stream capacity,
 * readable packet button pair, and the observed 2-byte nonoverlapping copies.
 */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_input_callback.h"
/* Observed neutral protocol values, independently read from L01 003BDD40. */
static const u8 D_003BDD40[20] = {
    0, 0x79, 255, 255, 128, 128, 128, 128, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
#else
typedef struct InputRecord5C0 {
    u8 pad0[0x1CC];
    s32 f1CC;
    u8 pad1D0[0x3F0];
} InputRecord5C0;
typedef struct DreadzoneInputContext1E0 {
    u16 f0[4], f8[4], f10[4];
    u32 f18;
    u8 pad1C[0x20];
    s32 f3C;
    u8 pad40[0x198];
    s32 f1D8, f1DC;
} DreadzoneInputContext1E0;
extern const u8 D_003BDD40[];
extern s32 D_002208A8;
extern s32 D_002208AC;
#endif

#ifdef DEADLOCKED_NATIVE
#define CALLBACK_OFFSET(type, field) offsetof(type, field)
#else
#define CALLBACK_OFFSET(type, field) ((u32)&((type *)0)->field)
#endif
typedef char CallbackInputSize[sizeof(InputRecord5C0) == 0x5C0 ? 1 : -1];
typedef char CallbackInputMode[CALLBACK_OFFSET(InputRecord5C0, f1CC) == 0x1CC ? 1 : -1];
typedef char CallbackContextSize[sizeof(DreadzoneInputContext1E0) == 0x1E0 ? 1 : -1];
typedef char CallbackAxes8[CALLBACK_OFFSET(DreadzoneInputContext1E0, f8) == 8 ? 1 : -1];
typedef char CallbackAxes10[CALLBACK_OFFSET(DreadzoneInputContext1E0, f10) == 0x10 ? 1 : -1];
typedef char CallbackFlags[CALLBACK_OFFSET(DreadzoneInputContext1E0, f18) == 0x18 ? 1 : -1];
typedef char CallbackCursor[CALLBACK_OFFSET(DreadzoneInputContext1E0, f3C) == 0x3C ? 1 : -1];
typedef char CallbackFrame[CALLBACK_OFFSET(DreadzoneInputContext1E0, f1D8) == 0x1D8 ? 1 : -1];
typedef char CallbackResult[CALLBACK_OFFSET(DreadzoneInputContext1E0, f1DC) == 0x1DC ? 1 : -1];
#undef CALLBACK_OFFSET

void func_L01_006B59C8(u8 *buffer, s32 value, s32 *cursor, u8 width) {
    s32 byte = *cursor / 8;
    s32 used = *cursor % 8;
    u32 remaining = width;
    u32 count, shift, mask, bits;
    do {
        count = remaining < (u32)(8 - used) ? remaining : (u32)(8 - used);
        shift = 8 - (u32)used - count;
        mask = (1U << count) - 1U;
        remaining -= count;
        bits = ((u32)value >> remaining) & mask;
        buffer[byte] = (u8)((buffer[byte] & ~(mask << shift)) | (bits << shift));
        *cursor += (s32)count;
        byte++;
        used = 0;
    } while (remaining != 0);
}

void func_L01_0055D3B8(void *, const void *, u32);
#ifdef DEADLOCKED_NATIVE
/* Actual forward copy's byte path: callback calls only use length2 and
 * disjoint packet/snapshot storage. Vector alignment/prefetch is not emulated.
 */
void func_L01_0055D3B8(void *destination, const void *source, u32 bytes) {
    u8 *out = destination;
    const u8 *in = source;
    u32 i;
    for (i = 0; i < bytes; i++) out[i] = in[i];
}
#endif

void func_L01_006B5CB8(InputRecord5C0 *record, const u8 *packet, s32 length,
                      DreadzoneInputContext1E0 *context) {
    const u8 *sample = record->f1CC != 0 ? D_003BDD40 : packet;
    s32 position = context->f1D8 % 4;
    s32 saved_count, changes, bit;
    u16 previous = position > 0 ? context->f0[position - 1] : 0xFFFF;
    u16 difference;
    u8 *stream = (u8 *)context + 0x19;
    func_L01_0055D3B8(&context->f0[position], sample + 2, 2);
    if (length < 8) sample = D_003BDD40;
    func_L01_0055D3B8(&context->f8[position], sample + 6, 2);
    func_L01_0055D3B8(&context->f10[position], sample + 4, 2);
    saved_count = context->f3C;
    difference = context->f0[position] ^ previous;
    changes = 0;
    for (bit = 0; bit < 16 && changes < 4; bit++) {
        if ((difference >> bit) & 1) {
            if (changes != 0) func_L01_006B59C8(stream, 0, &context->f3C, 1);
            func_L01_006B59C8(stream, bit, &context->f3C, 4);
            changes++;
        }
    }
    if (changes == 0) {
        context->f18 &= ~(1U << position);
    } else {
        if (changes >= 4) {
            context->f3C = saved_count;
            for (bit = 12; bit >= 0; bit -= 4) {
                func_L01_006B59C8(stream, (context->f0[position] >> bit) & 15,
                                  &context->f3C, 4);
                if (bit != 0) func_L01_006B59C8(stream, 0, &context->f3C, 1);
            }
        } else {
            func_L01_006B59C8(stream, 1, &context->f3C, 1);
        }
        context->f18 |= 1U << position;
    }
    context->f1D8++;
    if ((context->f1D8 & 3) == 0) {
        context->f1DC = context->f3C;
        D_002208AC += 4;
        D_002208A8 += 4 + context->f3C;
        context->f3C = 0;
    } else {
        context->f1DC = -1;
    }
}
