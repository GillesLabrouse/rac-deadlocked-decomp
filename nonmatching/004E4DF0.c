#include "common.h"

/* Experimental request-to-transition binding. The current compile is
 * 192 bytes versus 200 in retail, with different table addressing/register
 * choices after the bitmask update. Valid callers must supply indices covered
 * by D_00171C08 and the transition table. This draft is excluded from matching
 * coverage; queue processing marks pending request bits rather than clearing
 * the queue. The five-argument final call is verified from its callee.
 */

typedef struct TransitionRecord16 {
    s32 f0;
    s32 f4;
    s32 f8;
    s32 fC;
} TransitionRecord16;

typedef struct TransitionTable {
    s32 f0;
    s32 f4;
    TransitionRecord16 records[1];
} TransitionTable;

extern s32 D_00221E04;
extern u32 D_00171C08[];
extern u8 D_00171DF5[];
extern s32 D_0023A800[];
extern TransitionTable D_001C2040[];
extern s32 func_004E2AF8(s32);
extern void func_0045C8E8(void *);
extern void func_004E4EB8(s32, s32, TransitionRecord16 *, u32, s32);

typedef char transition_record_size_check[sizeof(TransitionRecord16) == 16 ? 1 : -1];
typedef char transition_table_records_check[
    (u32)&((TransitionTable *)0)->records == 8 ? 1 : -1];

void func_004E4DF0(s32 request) {
    s32 index;
    u32 *bits;
    s32 offset;
    u32 flag;
    if (request < 0) {
        D_00221E04 = 0;
        return;
    }
    index = func_004E2AF8(request);
    bits = &D_00171C08[(u32)request >> 5];
    *bits |= 1U << (request & 31);
    func_0045C8E8(D_0023A800);
    offset = (index * 16) | 8;
    flag = D_00171DF5[0];
    if (D_0023A800[0] != 0) flag = 0;
    func_004E4EB8(*(s32 *)((u8 *)D_001C2040 + offset + 8) + D_001C2040[0].f4,
                 *(s32 *)((u8 *)D_001C2040 + offset + 12),
                 (TransitionRecord16 *)((u8 *)D_001C2040 + 8 + index * 16),
                 flag, index == 24);
}
