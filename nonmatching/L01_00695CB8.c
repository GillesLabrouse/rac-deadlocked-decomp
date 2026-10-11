/* cflags: -mno-split-addresses */
/* asflags: --float-immediates */
/* Complete 2404-byte state-two update; NONMATCHING recovery draft. */
#include "common.h"
typedef unsigned int StateTwoQuad __attribute__((mode(TI)));
extern volatile f32 D_0021DDEC,D_0021DDF4,D_0021DDF8;
extern f32 func_L01_0055D910(f32,f32);
extern s32 func_L01_005BD868(s32,s32,u32,u32);
extern f32 func_L01_005BD900(s32,u32);
extern void func_L01_0066FAB8(u8 *,s32,s32);
extern void func_L01_0066FB88(u8 *,s32,s32);
extern void func_L01_0066FD98(u8 *,s32,s32);
extern void func_L01_0066FFA8(u8 *,s32,s32,s32);
extern s32 func_L01_006739B0(const u8 *);
extern void func_L01_0067BEC0(u8 *);
extern void func_L01_0067CAF8(u8 *);
extern void func_L01_0067CFD0(u8 *,s32,f32,f32,f32);
extern void func_L01_0067D100(u8 *);
extern void func_L01_0067D350(u8 *,f32,f32);
extern void func_L01_0067D3A0(u8 *,f32);
extern void func_L01_0067D480(u8 *,u8 *);
extern void func_L01_0067D4E8(u8 *);
extern void func_L01_0067D6E8(u8 *,s32);
extern f32 func_L01_0067EEA0(u8 *,StateTwoQuad);
extern void func_L01_00681B30(u8 *);
extern void func_L01_006840B8(u8 *,u32,u32,u32);
extern void func_L01_0068A178(u8 *,void *,const void *,f32);
#define FIELD(type,offset) (*(volatile type *)(arg0+(offset)))
static __inline__ f32 state_two_min(f32 a,f32 b) {
    f32 result;
    __asm__("min.s %0,%1,%2" : "=f"(result) : "f"(a),"f"(b));
    return result;
}
static __inline__ f32 state_two_max(f32 a,f32 b) {
    f32 result;
    __asm__("max.s %0,%1,%2" : "=f"(result) : "f"(a),"f"(b));
    return result;
}

void func_L01_00695CB8(u8 *arg0) {
    f32 temp_f0;
    f32 temp_f1;
    f32 temp_f1_2;
    f32 temp_f20;
    f32 temp_f2;
    f32 temp_f5;
    f32 var_f0;
    f32 var_f12;
    f32 var_f13;
    f32 var_f20;
    s16 temp_v0;
    s32 temp_s1;
    s32 temp_s1_2;
    s32 temp_s1_3;
    s32 temp_s1_4;
    s32 temp_s1_5;
    s32 var_a0;
    s32 var_v1;
    void *temp_a1;

    var_v1 = 0;
    if ((FIELD(u8,0x434) != 0) && (FIELD(f32,0x2E14) < 0.25f)) {
        var_v1 = 1;
    }
    if ((FIELD(u8,0x2661) != 0) || (var_v1 != 0)) {
        if ((D_0021DDF4 * FIELD(f32,0x25C4)) < FIELD(f32,0x1B4)) {
            func_L01_0066FB88(arg0, 0, 2);
        }
        if ((D_0021DDF4 * 0.5f * FIELD(f32,0x25C4)) < FIELD(f32,0x1B4)) {
            func_L01_0066FAB8(arg0, 0x28, 5);
            func_L01_0066FD98(arg0, 2, 4);
        }
        func_L01_0066FFA8(arg0, 5, 2, 0);
    }
    if ((FIELD(s16,0x5F4) != 0) || ((temp_s1 = FIELD(s32,0x1AE0), var_f0 = D_0021DDF4, (temp_s1 == 1)) && (var_f0 = D_0021DDF4, (func_L01_005BD868(0x28, 2, 0, FIELD(s32,0x2F00)) != 0)) && (var_f0 = D_0021DDF4, (func_L01_006739B0(arg0) != temp_s1)) && (var_f0 = D_0021DDF4, (func_L01_006739B0(arg0) != 0xF)))) {
        if (FIELD(s32,0x25CC) == 0x10) {
            var_f0 = D_0021DDF4;
            goto block_17;
        }
    } else {
block_17:
        if (FIELD(f32,0x1B4) < (var_f0 * 1.7f * FIELD(f32,0x25C4))) {
            FIELD(u8,0x1C8C) = 1U;
            if ((__builtin_fabsf(FIELD(f32,0x17A8)) > 2.0943952f) && (FIELD(f32,0x2E08) == 1.0f)) {
                FIELD(u8,0x1C8F) = 1U;
            }
        }
        temp_f20 = func_L01_005BD900(1, FIELD(s32,0x2F00));
        if (func_L01_0055D910(temp_f20, func_L01_005BD900(7, FIELD(s32,0x2F00))) > 1.9198622f) {
            if ((__builtin_fabsf(FIELD(f32,0x17A8)) > 1.2217305f) && (FIELD(f32,0x2E08) == 1.0f)) {
                FIELD(u8,0x1C8C) = 1U;
                FIELD(u8,0x1C8F) = 1U;
            }
        }
        if (__builtin_fabsf(FIELD(f32,0x17A8)) < 0.08726646f) {
            FIELD(u8,0x1C8C) = 0U;
        }
    }
    if ((FIELD(s32,0x3B4) < 3) && (__builtin_fabsf(FIELD(f32,0x17A8)) > 1.5707964f) && (FIELD(u8,0x5F6) != 0)) {
        FIELD(u8,0x1C8C) = 1U;
        FIELD(u8,0x1C8F) = 1U;
    }
    if ((FIELD(u8,0x1C8F) != 0) && (__builtin_fabsf(FIELD(f32,0x17A8)) < 0.06981317f)) {
        FIELD(u8,0x1C8F) = 0U;
    }
    if (FIELD(f32,0x2E08) < 0.2f) {
        if (FIELD(s32,0x1C90) != 0) {
            FIELD(u8,0x1C8F) = 1U;
        }
        FIELD(StateTwoQuad,0x1790) = (StateTwoQuad) FIELD(StateTwoQuad,0x1C70);
        FIELD(f32,0x17A0) = (f32) FIELD(f32,0x1C88);
        FIELD(f32,0x1780) = 0.0f;
    } else {
        func_L01_0067CAF8(arg0);
        var_a0 = -1;
        if (FIELD(s32,0x2314) == 2) {
            var_a0 = FIELD(s32,0x2318);
        }
        if (var_a0 != 1) {
            goto block_52;
        }
        if (FIELD(f32,0x2E38) != 1.0f) {
            if (FIELD(s32,0x1C90) == 0) {
                goto block_52;
            }
            goto block_48;
        }
block_48:
        if (FIELD(u8,0x2658) != 0) {
block_52:
            FIELD(StateTwoQuad,0x1C70) = FIELD(StateTwoQuad,0x20);
            FIELD(f32,0x1C88) = (f32) FIELD(f32,0xB8);
        } else {
            FIELD(StateTwoQuad,0x1C70) = (StateTwoQuad) FIELD(StateTwoQuad,0x1790);
            FIELD(f32,0x1C88) = (f32) FIELD(f32,0x17A0);
            FIELD(s32,0x1C90) = var_a0;
        }
    }
    if (FIELD(f32,0x1E70) > 0.0f) {
        f32 contribution,value,scale,time;
        contribution = func_L01_0067EEA0(arg0,FIELD(StateTwoQuad,0x1E60));
        value = FIELD(f32,0x1780) + contribution;
        scale = FIELD(f32,0x25C4);
        FIELD(f32,0x1780) = value;
        time = D_0021DDF4;
        FIELD(StateTwoQuad,0x1E60) = 0;
        FIELD(f32,0x1780) = state_two_max(value,time * 0.7f * scale);
    }
    var_f20 = 1.0f;
    if (FIELD(s16,0x5F4) == 0) {
        temp_s1_2 = FIELD(s32,0x1AE0);
        if (temp_s1_2 != 1) {
            goto block_72;
        }
        if (func_L01_005BD868(0x28, 2, 0, FIELD(s32,0x2F00)) == 0) {
            goto block_72;
        }
        if (func_L01_006739B0(arg0) == temp_s1_2) {
            goto block_72;
        }
        if (func_L01_006739B0(arg0) == 0xF) {
            goto block_72;
        }
        goto block_68;
    }
block_68:
    if (FIELD(s32,0x25CC) == 0x10) {
block_72:
        FIELD(f32,0x5F8) = FIELD(f32,0xB8);
        FIELD(StateTwoQuad,0x5E0) = (StateTwoQuad) FIELD(StateTwoQuad,0x20);
        if ((FIELD(s32,0x25D0) == 1) && (FIELD(u8,0x1C8F) == 0) && (FIELD(u8,0x2658) == 0)) {
            temp_f0 = FIELD(f32,0x25C4);
            func_L01_0067CFD0(arg0, 0, D_0021DDEC * 0.037f * temp_f0, D_0021DDEC * 0.13f * temp_f0, D_0021DDF4 * 12.217304f * temp_f0);
        } else {
            temp_f1 = __builtin_fabsf(FIELD(f32,0x17A8));
            if (((FIELD(f32,0x2E08) < 0.9f) || ((FIELD(u8,0x2658) != 0) && (FIELD(u8,0x1C8F) == 0))) && (temp_f1 > 0.7853982f)) {
                var_f20 = 0.0f;
            }
            func_L01_0067D100(arg0);
        }
    } else {
        func_L01_0067BEC0(arg0);
    }
    func_L01_00681B30(arg0);
    if (FIELD(u8,0x1C8F) != 0) {
        f32 time = D_0021DDF4;
        f32 scale = FIELD(f32,0x25C4);
        f32 angle = __builtin_fabsf(FIELD(f32,0x17A8));
        f32 reduction = time * 5.5f * scale * angle / 3.1415927f;
        f32 maximum = state_two_max(time * 7.25f * scale - reduction,0.0f);
        FIELD(f32,0x1780) = state_two_min(FIELD(f32,0x1780),maximum);
    }
    temp_f1_2 = D_0021DDF8;
    temp_f5 = FIELD(f32,0x25C4);
    var_f13 = temp_f1_2 * 30.0f * temp_f5;
    if (FIELD(u8,0x1C8F) != 0) {
        f32 ratio = state_two_min(2.6179938f - FIELD(f32,0x17A8),1.0f);
        FIELD(f32,0x1780) = FIELD(f32,0x1780) * state_two_max(ratio,0.2f);
        var_f13 = D_0021DDF8 * 17.0f * temp_f5;
    }
    temp_f2 = __builtin_fabsf(FIELD(f32,0x17A8));
    if ((FIELD(f32,0x2E08) < 0.75f) && (temp_f2 > 0.61086524f)) {
        FIELD(f32,0x1780) = FIELD(f32,0x1780) * state_two_max(1.0f - temp_f2 * 0.4f,0.0f);
    }
    func_L01_0067D350(arg0, temp_f1_2 * 37.0f * temp_f5 * var_f20, var_f13);
    if ((FIELD(u8,0x1C8C) != 0) || (((FIELD(s16,0x5F4) != 0) || ((temp_s1_3 = FIELD(s32,0x1AE0), (temp_s1_3 == 1)) && (func_L01_005BD868(0x28, 2, 0, FIELD(s32,0x2F00)) != 0) && (func_L01_006739B0(arg0) != temp_s1_3) && (func_L01_006739B0(arg0) != 0xF))) && (FIELD(s32,0x25CC) != 0x10))) {
        temp_v0 = FIELD(s16,0x5F4);
        if (FIELD(u8,0x2659) == 0) {
            if (temp_v0 == 0) {
                temp_s1_4 = FIELD(s32,0x1AE0);
                if (temp_s1_4 != 1) {
                    var_f12 = FIELD(f32,0x17A0);
                } else if (func_L01_005BD868(0x28, 2, 0, FIELD(s32,0x2F00)) == 0) {
                    var_f12 = FIELD(f32,0x17A0);
                } else if (func_L01_006739B0(arg0) == temp_s1_4) {
                    var_f12 = FIELD(f32,0x17A0);
                } else if (func_L01_006739B0(arg0) == 0xF) {
                    var_f12 = FIELD(f32,0x17A0);
                } else {
                    goto block_110;
                }
            } else {
block_110:
                if (FIELD(s32,0x25CC) == 0x10) {
                    var_f12 = FIELD(f32,0x17A0);
                } else {
                    var_f12 = FIELD(f32,0x5F8);
                }
            }
            func_L01_0067D3A0(arg0, var_f12);
        } else if (((temp_v0 != 0) || ((temp_s1_5 = FIELD(s32,0x1AE0), (temp_s1_5 == 1)) && (func_L01_005BD868(0x28, 2, 0, FIELD(s32,0x2F00)) != 0) && (func_L01_006739B0(arg0) != temp_s1_5) && (func_L01_006739B0(arg0) != 0xF))) && (FIELD(s32,0x25CC) != 0x10)) {
            func_L01_0067D480(arg0, arg0 + 0x5E0);
        } else {
            func_L01_0067D480(arg0, arg0 + 0x1790);
        }
    } else {
        func_L01_0067D3A0(arg0, 99999.0f);
    }
    func_L01_0067D6E8(arg0, 0);
    if (FIELD(s16,0x3D0) != 0) {
        func_L01_006840B8(arg0, 0x40A00000, 0x3E4CCCCD, 0x3EB33333);
    }
    if (FIELD(s16,0x2FE) != 0) {
        func_L01_0068A178(arg0, arg0 + 0x130, arg0 + 0x170, D_0021DDF8 * 25.0f * FIELD(f32,0x25C4));
        func_L01_0067D4E8(arg0);
        return;
    }
    temp_a1 = arg0 + 0x130;
    func_L01_0068A178(arg0, temp_a1, temp_a1, D_0021DDF8 * 54.0f * FIELD(f32,0x25C4));
}
