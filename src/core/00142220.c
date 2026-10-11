#include "common.h"

typedef struct Type2 {
    s32 value;
    s32 flag;
} Type2;

typedef struct Type1 {
    u8 pad[0x40];
    Type2 *unk40;
} Type1;

s32 func_00142220(Type1 *arg0) {
    return arg0->unk40->value;
}
