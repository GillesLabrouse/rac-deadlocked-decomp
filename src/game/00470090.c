#include "common.h"

u64 func_00470090(u8 *actor, u64 upper, u64 component0, u64 component1, u64 component2) {
    register u64 result __asm__("$2");
    register u64 high __asm__("$5") = upper;
    register u64 first __asm__("$6") = component0;
    register u64 second __asm__("$7") = component1;
    register u64 third __asm__("$8") = component2;
    high <<= 32;
    /* Empty compiler barriers retain the original ordering of independent
     * shifts. The packing operation and store are expressed in C. */
    __asm__ volatile("" : "+r"(high), "+r"(second), "+r"(third));
    second <<= 8;
    __asm__ volatile("" : "+r"(high), "+r"(second), "+r"(third));
    third <<= 16;
    __asm__ volatile("" : "+r"(high), "+r"(first), "+r"(second), "+r"(third));
    result = high | first | second | third;
    *(u64 *)(actor + 0x38) = result;
    return result;
}
