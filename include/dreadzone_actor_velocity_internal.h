#ifndef DEADLOCKED_ACTOR_VELOCITY_INTERNAL_H
#define DEADLOCKED_ACTOR_VELOCITY_INTERNAL_H
/* Include common.h first so native and original compiler widths stay explicit. */
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_velocity.h"
#include <math.h>
#include <string.h>
#define V_FLOAT(p,o) dreadzone_velocity_float((const u8 *)(p)+(o))
#define V_STORE(p,o,x) dreadzone_velocity_store((u8 *)(p)+(o),(x))
#define V_MIN(a,b) dreadzone_velocity_min((a),(b))
#define V_MAX(a,b) dreadzone_velocity_max((a),(b))
#define V_ABS(x) fabsf(x)
#else
typedef struct __attribute__((aligned(16))) VelocityVector { f32 v[4]; } VelocityVector;
typedef unsigned int VelocityQuad __attribute__((mode(TI)));
typedef union VelocityLanes { VelocityQuad quad; f32 v[4]; } VelocityLanes;
typedef union VelocityBits { u32 word; f32 value; } VelocityBits;
#define V_FLOAT(p,o) (*(f32 *)((u8 *)(p)+(o)))
#define V_STORE(p,o,x) (V_FLOAT(p,o)=(x))
#define V_MIN(a,b) ((a)<(b)?(a):(b))
#define V_MAX(a,b) ((a)>(b)?(a):(b))
#define V_ABS(x) __builtin_fabsf(x)
#define V_ANGLE_PAIR(a,s,c) do { VelocityBits input; VelocityLanes pair; \
    input.value=(a); \
    __asm__ volatile("qmtc2.ni %1,$vf15\n\tvcallms 0xC60\n\tqmfc2.i %0,$vf15" \
        : "=r"(pair.quad) : "r"(input.word) : "memory"); \
    (s)=pair.v[0]; (c)=pair.v[1]; \
} while(0)
f32 func_L01_00594AE8(f32 *,f32,f32);
void func_L01_0067D350(u8 *,f32,f32);
VelocityQuad func_L01_00652978(const u8 *,f32,f32,f32);
void func_L01_0067D3A0(u8 *,f32);
void func_L01_0067D480(u8 *,const VelocityVector *);
f32 func_L01_0068A920(const u8 *,const VelocityVector *);
f32 func_L01_0068AB40(const u8 *,const VelocityVector *);
void func_L01_0068AC58(const u8 *,VelocityVector *,const VelocityVector *,f32);
#endif
#endif
