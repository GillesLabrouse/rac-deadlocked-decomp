/* Actual native L01 0055D2A8 word-fill, separated from pool initialization
 * so the actor reset does not link unrelated pool global storage. */
#include "common.h"
void func_L01_0055D2A8(u32 *destination, u32 value, s32 bytes) {
    do {
        *destination++ = value;
        bytes -= 4;
    } while (bytes > 0);
}
