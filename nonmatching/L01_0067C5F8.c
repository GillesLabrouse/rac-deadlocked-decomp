/* cflags: -ffast-math */
/* asflags: --float-immediates */
/* Complete 1276-byte direction transform plus four-byte padding, NONMATCHING.
 * Real axis preparation, classification, matrix projection and normalization.
 * Native supports2659=0; no successful capture of the larger00689A10 branch. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_direction.h"
#endif
#include "../include/dreadzone_actor_direction_internal.h"
#ifdef DEADLOCKED_NATIVE
#define MATRIX(m,i,o) dreadzone_direction_matrix((m),(i),(o))
#define LIMIT_XY(v) dreadzone_direction_limit_xy(v)
#define NORMALIZE(v) dreadzone_direction_normalize(v)
#define LENGTH_XY(v,r) ((r)=dreadzone_direction_length_xy(v))
int func_L01_0067C5F8(const DreadzoneActorInputContext *ctx,u8 *record,s32 cardinal,f32 scale) {
    const u8 *body;s32 status;
#else
#define MATRIX(m,i,o) __asm__ volatile("lqc2 $vf4,0(%1)\n\tlqc2 $vf1,0(%0)\n\t" \
    "lqc2 $vf2,0x10(%0)\n\tvmulax.xyzw ACC,$vf1,$vf4x\n\t" \
    "lqc2 $vf1,0x20(%0)\n\tvmadday.xyzw ACC,$vf2,$vf4y\n\t" \
    "vmaddaz.xyzw ACC,$vf1,$vf4z\n\tvmaddw.xyzw $vf1,$vf0,$vf4w\n\t" \
    "sqc2 $vf1,0(%2)" : : "r"(m),"r"(i),"r"(o) : "memory")
#define LENGTH_XY(v,r) do { union {u32 u;f32 f;} bits; \
    __asm__ volatile("lqc2 $vf4,0(%1)\n\tvmul.xyzw $vf1,$vf4,$vf4\n\t" \
        "vaddy.x $vf1,$vf1,$vf1y\n\tnop\n\tnop\n\tvsqrt Q,$vf1x\n\t" \
        "vwaitq\n\tvaddq.x $vf1,$vf0,Q\n\tqmfc2.ni %0,$vf1" \
        : "=r"(bits.u) : "r"(v) : "memory"); (r)=bits.f; } while(0)
#define LIMIT_XY(v) do { f32 length;LENGTH_XY(v,length);if(length>1.f) { \
    union {u32 u;f32 f;} factor;factor.f=1.f/length; \
    __asm__ volatile("lqc2 $vf2,0(%0)\n\tqmtc2.ni %1,$vf1\n\t" \
        "vmulx.xyz $vf2,$vf2,$vf1x\n\tsqc2 $vf2,0(%0)" \
        : : "r"(v),"r"(factor.u) : "memory"); }} while(0)
#define NORMALIZE(v) do { u32 square; \
    __asm__ volatile(".set noreorder\n\tlqc2 $vf1,0(%1)\n\tvmaxw.xyzw $vf2,$vf0,$vf0w\n\t" \
        "vmul.xyzw $vf3,$vf1,$vf1\n\tvadday.x ACC,$vf3,$vf3y\n\t" \
        "vmaddz.x $vf2,$vf2,$vf3z\n\tqmfc2.ni %0,$vf2\n\t" \
        "nop\n\tnop\n\tvrsqrt Q,$vf0w,$vf2x\n\tbnez %0,1f\n\tnop\n\t" \
        "vsub.xyz $vf1,$vf0,$vf0\n\tsqc2 $vf1,0(%1)\n\tb 2f\n\tnop\n\t" \
        "1:\n\tvwaitq\n\tlqc2 $vf1,0(%1)\n\tvmulq.xyz $vf1,$vf1,Q\n\t" \
        "sqc2 $vf1,0(%1)\n\t2:\n\t.set reorder" \
        : "=&r"(square) : "r"(v) : "memory"); } while(0)
void func_L01_0067C5F8(u8 *record,s32 cardinal,f32 scale) {
    u8 *body;
#endif
    DirectionVector axes,local,transformed;
    f32 angle,length;s32 pressed,item,classification;
    axes.v[0]=D_FLOAT(record,0x120);axes.v[1]=-D_FLOAT(record,0x124);
    axes.v[2]=axes.v[3]=0.f;
    if(cardinal==1) {
        if(D_ABS(axes.v[1])>0.699999988079071044921875f &&
           D_ABS(axes.v[0])<0.119999997317790985107421875f) axes.v[0]=0.f;
        if(D_ABS(axes.v[0])>0.699999988079071044921875f &&
           D_ABS(axes.v[1])<0.119999997317790985107421875f) axes.v[1]=0.f;
    }
    LIMIT_XY(&axes);
    if(axes.v[0]==0.f && axes.v[1]==0.f) {
        D_STORE_WORD(record,0x1780,0);
        if(record[0x2659]) {
#ifdef DEADLOCKED_NATIVE
            return DREADZONE_DIRECTION_UNSUPPORTED_MODE;
#else
            D_STORE_FLOAT(record,0x17A0,func_L01_00689A10(record));
#endif
        } else D_STORE_WORD(record,0x17A0,D_WORD(record,0xB8));
        local.v[0]=1.f;local.v[1]=local.v[2]=local.v[3]=0.f;
#ifdef DEADLOCKED_NATIVE
        if(D_WORD(record,0x2EF0)&15 || D_WORD(record,0x2EF0)>0xFFFFFFFFU-0xEF)
            return DREADZONE_DIRECTION_NEEDS_MEMORY;
        body=dreadzone_actor_input_read(ctx,D_WORD(record,0x2EF0)+0xC0,48);
        if(!body) return DREADZONE_DIRECTION_NEEDS_MEMORY;
        MATRIX(body,&local,&transformed);
        memcpy(record+0x1790,&transformed,sizeof(transformed));
#else
        body=(u8 *)D_WORD(record,0x2EF0);
        MATRIX(body+0xC0,&local,(DirectionVector *)(record+0x1790));
#endif
#ifdef DEADLOCKED_NATIVE
        return DREADZONE_DIRECTION_APPROXIMATE;
#else
        return;
#endif
    }
    angle=func_L01_0055D708(-axes.v[1],-axes.v[0]);D_STORE_FLOAT(record,0x17A0,angle);
#ifdef DEADLOCKED_NATIVE
    if(record[0x5F4] || record[0x5F5]) goto classify_if_state;
#else
    if(*(s16 *)(record+0x5F4)) goto classify_if_state;
#endif
    if(D_WORD(record,0x1AE0)!=1) goto rotate_direction;
#ifdef DEADLOCKED_NATIVE
    status=dreadzone_direction_history(ctx,0x28,2,0,D_WORD(record,0x2F00),&pressed);
    if(status) return status;
#else
    pressed=func_L01_005BD868(0x28,2,0,(u8 *)D_WORD(record,0x2F00));
#endif
    if(!pressed) goto rotate_direction;
    item=func_L01_006739B0(record);if(item==1) goto rotate_direction;
    item=func_L01_006739B0(record);if(item==15) goto rotate_direction;
classify_if_state:
    if(D_WORD(record,0x25CC)==0x10) goto rotate_direction;
    angle=func_L01_0055D708(axes.v[0],axes.v[1]);classification=2;
    if(func_L01_0055D910(angle,1.57079637050628662109375f)<1.0471975803375244140625f)
        classification=3;
    else if(func_L01_0055D910(angle,0.f)<0.785398185253143310546875f) classification=1;
    else if(func_L01_0055D910(angle,3.1415927410125732421875f)<0.785398185253143310546875f)
        classification=0;
    record[0x5F7]=(u8)classification;
    if(classification==3) {
#ifdef DEADLOCKED_NATIVE
        if(!dreadzone_direction_valid_float(D_FLOAT(record,0x2E08),1.f))
            return DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;
#endif
        record[0x5F6]=(u8)(D_FLOAT(record,0x2E08)>0.0500000007450580596923828125f);
    }
rotate_direction:
    D_STORE_FLOAT(record,0x17A0,func_L01_0055D828(D_FLOAT(record,0x17A0),D_FLOAT(record,0x2CE8)));
    local.v[0]=-axes.v[1];local.v[1]=-axes.v[0];local.v[2]=local.v[3]=0.f;
    MATRIX(record+0x2CF0,&local,&transformed);
#ifdef DEADLOCKED_NATIVE
    memcpy(record+0x1790,&transformed,sizeof(transformed));
    status=dreadzone_direction_projection_bytes(record,record+0x1790,record+0x1790,0.f);
    if(status) return status;
    memcpy(&transformed,record+0x1790,sizeof(transformed));
#else
    *(DirectionVector *)(record+0x1790)=transformed;
    func_L01_0068ABA8(record,(DirectionVector *)(record+0x1790),(DirectionVector *)(record+0x1790),0.f);
    transformed=*(DirectionVector *)(record+0x1790);
#endif
    NORMALIZE(&transformed);
#ifdef DEADLOCKED_NATIVE
    memcpy(record+0x1790,&transformed,sizeof(transformed));
#else
    *(DirectionVector *)(record+0x1790)=transformed;
#endif
    LENGTH_XY(&local,length);D_STORE_FLOAT(record,0x1780,scale*length);
#ifdef DEADLOCKED_NATIVE
    return DREADZONE_DIRECTION_APPROXIMATE;
#endif
}
