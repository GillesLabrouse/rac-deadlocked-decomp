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

extern void *func_0011A518(void *, s32, u32);

/* Initialize the ten-entry queue without changing its owner fields. */
void func_0045C768(StartupQueue *queue) {
    queue->f58 = 0;
    queue->f5C = 0;
    queue->f60 = 0;
    func_0011A518(queue->items, 0xCD, 0x50);
    queue->f6C = 0;
    queue->f68 = 1;
}

/* Pop one item, preserving untouched outputs when the queue is empty. */
s32 func_0045C7B0(StartupQueue *queue, s32 *value, s32 *kind) {
    s32 count = queue->f60;
    s32 index;
    u8 *values;
    u8 *kinds;
    if (count == 0) return 0;
    count--;
    queue->f60 = count;
    if (count == 0) queue->f6C = 0;
    values = (u8 *)queue + 8;
    kinds = (u8 *)queue + 12;
    *value = *(s32 *)(values + (queue->f5C << 3));
    *kind = *(s32 *)(kinds + (queue->f5C << 3));
    *(s32 *)(values + (queue->f5C << 3)) = -1;
    *(s32 *)(kinds + (queue->f5C << 3)) = -1;
    index = queue->f5C + 1;
    queue->f5C = index == 10 ? 0 : index;
    return 1;
}

/* Enqueue while capacity remains and dispatch has not locked the queue. */
s32 func_0045C850(StartupQueue *queue, s32 value) {
    s32 index;
    if (queue->f60 == 10 || queue->f6C != 0) return 0;
    queue->items[queue->f58].value = value;
    queue->items[queue->f58].kind = value >= 100;
    index = queue->f58 + 1;
    queue->f60++;
    queue->f58 = index == 10 ? 0 : index;
    return 1;
}
