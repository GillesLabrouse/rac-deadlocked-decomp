#ifndef DEADLOCKED_ACTOR_DIRECTION_INTERNAL_H
#define DEADLOCKED_ACTOR_DIRECTION_INTERNAL_H
/* Each C translation unit includes common.h before this shared private ABI. */
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_direction.h"
#include <math.h>
#include <string.h>
#define D_WORD(p,o) dreadzone_actor_input_word((p)+(o))
#define D_FLOAT(p,o) dreadzone_direction_float((p)+(o))
#define D_STORE_FLOAT(p,o,f) dreadzone_direction_store_float((p)+(o),(f))
#define D_STORE_WORD(p,o,w) dreadzone_actor_input_store32((p)+(o),(w))
#define D_ABS(x) fabsf(x)
#define D_MIN(a,b) dreadzone_direction_min(a,b)
#define D_MAX(a,b) dreadzone_direction_max(a,b)
#else
typedef struct __attribute__((aligned(16))) DirectionVector { f32 v[4]; } DirectionVector;
#define D_WORD(p,o) (*(u32 *)((p)+(o)))
#define D_FLOAT(p,o) (*(f32 *)((p)+(o)))
#define D_STORE_FLOAT(p,o,f) (D_FLOAT(p,o)=(f))
#define D_STORE_WORD(p,o,w) (D_WORD(p,o)=(w))
#define D_ABS(x) __builtin_fabsf(x)
#define D_MIN(a,b) ((a)<(b)?(a):(b))
#define D_MAX(a,b) ((a)>(b)?(a):(b))
f32 func_L01_0055D708(f32,f32);
f32 func_L01_0055D910(f32,f32);
f32 func_L01_0055D828(f32,f32);
f32 func_L01_0055D870(f32,f32);
f32 func_L01_00595188(s32,f32,f32);
f32 func_L01_00594A88(f32 *,f32,f32,f32,f32);
f32 func_L01_005952B8(f32 *,f32 *,s32,f32,f32,f32,f32);
s32 func_L01_005BD868(s32,s32,s32 *,u8 *);
s32 func_L01_006739B0(const u8 *);
void func_L01_0068ABA8(u8 *,DirectionVector *,const DirectionVector *,f32);
f32 func_L01_00689A10(u8 *);
void func_L01_0067CEF8(u8 *,s32,f32,f32,f32);
#endif
#endif
