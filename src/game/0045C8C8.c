/* cflags: -mno-split-addresses */
#include "common.h"

typedef struct StartupItem {
    s32 value;
    s32 kind;
} StartupItem;
typedef struct StartupQueue {
    s32 f0;
    s32 f4;
    StartupItem items[10];
    s32 f58;
    s32 f5C;
    s32 f60;
    u8 f64;
    u8 gap65[3];
    s32 f68;
    u8 f6C;
} StartupQueue;
extern void func_0045C768(StartupQueue *);
extern s32 func_0045C7B0(StartupQueue *, s32 *, s32 *);
extern s32 func_0047F568(s32, s32, s32, s32, u8 *);
extern s32 D_0021DDB4_[];

/* Reset the queue through its initialization routine. */
void func_0045C8C8(StartupQueue *queue) {
    func_0045C768(queue);
}

/* Dispatch one queued request; a negative handler result resets the queue.
 * The handler and its request identifiers still require gameplay interpretation.
 */
s32 func_0045C990(StartupQueue *queue) {
    s32 value;
    s32 kind;
    s32 result;
    s32 mode;
    if (queue->f60 != 0) {
        queue->f64 = 0;
        queue->f6C = 1;
        value = 0;
        result = 0;
        kind = 0;
        func_0045C7B0(queue, &value, &kind);
        mode = (u32)D_0021DDB4_[0] - 1U <= 1U ? 2 : 1;
        queue->f68 = mode;
        if (kind == 0) result = func_0047F568(1, mode, 0, value, 0);
        else if (kind == 1) result = func_0047F568(2, mode, value, 0, 0);
        if (result < 0) func_0045C8C8(queue);
        return result == 0;
    }
    return 0;
}
