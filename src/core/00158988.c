#include "common.h"

extern s32 func_001589A8(s32, s32, s32, s32, s32, s32, void (*)(s32, s64), s64);

s32 func_00158988(s32 first, s32 second, s32 third, s32 fourth, s32 fifth, s32 sixth) {
    return func_001589A8(first, second, third, fourth, fifth, sixth, NULL, 0);
}
