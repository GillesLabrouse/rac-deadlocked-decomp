#include "common.h"

typedef struct CommandQueue {
    s32 capacity;
    s32 count;
    u8 *read;
    u8 *write;
} CommandQueue;

void func_00127E30(CommandQueue *queue) {
    queue->count--;
    queue->read++;
    if (queue->read == (u8 *)queue + (queue->capacity + 0x10)) {
        queue->read = (u8 *)queue + 0x10;
    }
}
