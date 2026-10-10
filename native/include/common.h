#ifndef DEADLOCKED_NATIVE_COMMON_H
#define DEADLOCKED_NATIVE_COMMON_H

#include <stddef.h>
#include <stdint.h>

/* Preserve game scalar widths without assuming the PS2 compiler's long ABI. */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;

#endif
