#include "common.h"



void func_0014D2C0(s32);
void func_0014EDE0(s32 *);
void func_001516D8(void *, void *, void *);


void func_00162C50(s32 *arg0) {
    if (arg0 != NULL) {
        func_0014D2C0(*arg0);
        func_001516D8(arg0 + 1, arg0 + 2, arg0 + 3);
        func_0014EDE0(arg0);
    }
}
