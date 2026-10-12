#include "common.h"

void func_00138590(u32 arg0, u32 arg1, u32 arg2, u32 arg3, u32 arg4) {
    register u32 syscall_code __asm__("$3") = 0x80;
    register u32 syscall_a0 __asm__("$4") = arg0;
    register u32 syscall_a1 __asm__("$5") = arg1;
    register u32 syscall_a2 __asm__("$6") = arg2;
    register u32 syscall_a3 __asm__("$7") = arg3;
    register u32 syscall_t0 __asm__("$8") = arg4;
    __asm__ volatile ("syscall 0" : "+r" (syscall_code)
                      : "r" (syscall_a0), "r" (syscall_a1), "r" (syscall_a2),
                        "r" (syscall_a3), "r" (syscall_t0) : "memory");
}
