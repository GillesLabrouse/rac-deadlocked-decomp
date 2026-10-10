/* cflags: -mno-split-addresses */
/* asflags: --float-immediates */
#include "common.h"

/* Experimental Dreadzone state-zero orchestration, independently read from
 * the L01 overlay. The retail extent is 1272 bytes; the current compile is
 * 1240 bytes with a 0x20 frame versus retail's 0x30. This is excluded from
 * matching coverage and the native runtime. It relies on existing 0x31F0
 * records and does not construct their actors. See DREADZONE_OBJECT_SETUP.md.
 * Several globals use both GP and absolute accesses in retail; this draft
 * retains their data identity but does not yet reproduce each access mode.
 * Values returned by calls whose results are discarded remain provisional.
 */
typedef struct ObjectFlags1A8 {
    u8 gap0[0x1A4];
    u32 f1A4;
} ObjectFlags1A8;

typedef struct ObjectChild8 {
    s32 f0;
    u8 *f4;
} ObjectChild8;

typedef struct ObjectRecord31F0 {
    u8 gap0[0x2EDC];
    u8 f2EDC;
    u8 f2EDD;
    u8 gap2EDE[0x22];
    ObjectFlags1A8 *f2F00;
    u8 gap2F04[0x24];
    ObjectChild8 *f2F28;
    u8 gap2F2C[0x2C4];
} ObjectRecord31F0;
typedef unsigned int ObjectQuad __attribute__((mode(TI)));
extern u8 D_001711B9 __attribute__((section(".data")));
extern u8 D_001711BD __attribute__((section(".data")));
extern u8 D_00171D3D __attribute__((section(".data")));
extern u8 D_001723B0 __attribute__((section(".data")));
extern u8 D_001737E1 __attribute__((section(".data")));
extern s32 D_00171B40 __attribute__((section(".data")));
extern s32 D_00172170 __attribute__((section(".data")));
extern s32 D_001723B4 __attribute__((section(".data")));
extern s32 D_001723B8 __attribute__((section(".data")));
extern s32 D_00172424 __attribute__((section(".data")));
extern s32 D_00172430 __attribute__((section(".data")));
extern u8 D_002121A0[];
extern u8 D_002124F0[0x1C4];
extern s32 *D_0021DDA4;
extern u8 *D_0021DFE8;
extern s32 D_0021DDB4;
extern s32 D_0021DE10 __attribute__((section(".data")));
extern s32 D_0021E818 __attribute__((section(".data")));
extern s32 D_0021E824;
extern s32 D_0021E828 __attribute__((section(".data")));
extern s32 D_0021E8A0;
extern s32 D_0021E8A4 __attribute__((section(".data")));
extern s32 D_0021E8FC __attribute__((section(".data")));
extern s32 D_00222B84 __attribute__((section(".data")));
extern s32 D_00222BEC;
extern ObjectQuad *D_0021EE00 __attribute__((section(".data")));
s32 func_L01_00630430(void);
ObjectRecord31F0 *func_L01_006658C0(s32);
void func_L01_00665CB0(s32, s32);
s32 func_L01_0055D1E0(u8 *);
void func_L01_0055D3B8(void *, const void *, u32);
s32 func_L01_005A9608(s32, s32, s32, s32, u8 *);
s32 func_L01_00527B00(void);
void func_L01_005A95E8(s32);
s32 func_L01_0053A470(void);
s32 func_L01_005D8228(s32);
s32 func_L01_005D8248(s32);
void func_L01_005D7058(s32);
void func_0015C750(s32);
void func_0015C760(void);
s32 func_0015ADA8(void);
void func_0015B370(void);
void func_00161CF0(void);
s32 func_01E9E798(void);
void func_L01_00630438(void);
void func_L01_00538610(void);
void func_L01_0056F440(void);
void func_L01_00573C58(void);
void func_L01_00574B28(void);
void func_L01_00586950(void);
void func_L01_00587E20(void);
void func_L01_005A0AD8(void);
void func_L01_005B5940(void);
void func_L01_005BA3B8(void);
void func_L01_005BEC10(void);
void func_L01_005D7010(void);
void func_L01_00624C88(void);
void func_L01_00625DA8(void);
void func_L01_006272C8(void);
void func_L01_00630360(void);
void func_L01_00630388(void);
void func_L01_00637588(void);
void func_L01_00653028(void);
void func_L01_006666A8(void);
void func_L01_006A8340(void);


typedef char object_record_size_check[sizeof(ObjectRecord31F0) == 0x31F0 ? 1 : -1];
typedef char object_flags_offset_check[(u32)&((ObjectFlags1A8 *)0)->f1A4 == 0x1A4 ? 1 : -1];
typedef char object_record_flag_offset_check[(u32)&((ObjectRecord31F0 *)0)->f2EDC == 0x2EDC ? 1 : -1];
typedef char object_record_definition_offset_check[(u32)&((ObjectRecord31F0 *)0)->f2F00 == 0x2F00 ? 1 : -1];
typedef char object_record_child_offset_check[(u32)&((ObjectRecord31F0 *)0)->f2F28 == 0x2F28 ? 1 : -1];
typedef char object_child_pointer_offset_check[(u32)&((ObjectChild8 *)0)->f4 == 4 ? 1 : -1];
typedef char object_quad_size_check[sizeof(ObjectQuad) == 16 ? 1 : -1];

void func_L01_00630438(void) {
    ObjectChild8 *temp_v0_2;
    ObjectChild8 *temp_v0_4;
    ObjectRecord31F0 *temp_s0;
    ObjectRecord31F0 *temp_v0;
    ObjectRecord31F0 *temp_v0_3;
    s32 *temp_v1;
    s32 *temp_v1_2;
    s32 temp_v0_5;
    s32 temp_v0_6;
    s32 var_s0;
    s32 var_s0_3;
    s32 var_s0_4;
    s32 var_s1;
    s32 var_s2_2;
    s32 var_v0;
    u8 *temp_a1;
    u8 *var_s0_2;
    u8 *var_s2;

    if (func_L01_00630430() == 0) {
        if ((D_0021E824 != 0) || (D_0021E828 != 0)) {
            var_s1 = 0;
            if (*D_0021DDA4 > 0) {
                do {
                    temp_s0 = func_L01_006658C0(var_s1);
                    if ((D_00222B84 >= 8) && (temp_s0->f2F00->f1A4 & 0x800) && (D_0021DDB4 == 0)) {
                        func_L01_00665CB0(var_s1, 1);
                        temp_s0->f2EDD = 0xF;
                    }
                    if (temp_s0->f2EDC != 0) {
                        func_L01_0055D1E0(&temp_s0->f2EDD);
                    }
                    temp_v1 = D_0021DDA4;
                    var_s1 += 1;
                } while (var_s1 < *temp_v1);
            }
        }
        func_L01_005A0AD8();
        if ((D_00222BEC != 0) && (func_L01_005A9608(6, 1, 0, 0, NULL) == 0)) {
            func_L01_005A95E8(func_L01_00527B00() != 0);
            D_00222BEC = 0;
        } else {
            func_L01_00630360();
            if ((D_0021DE10 == 0) || (var_v0 = 0, ((D_0021DE10 < 0x28) == 0))) {
                var_v0 = 1;
            }
            if (var_v0 != 0) {
                func_00161CF0();
                func_0015ADA8();
                if ((((f32) D_00172430 > 1500.0f) || (D_001737E1 != 0)) && (D_001723B8 == 0)) {
                    if (D_001737E1 != 0) {
                        D_00172424 = 2;
                    }
                    switch (D_00172424) {           /* irregular */
                    case 0:
                        break;
                    case 2:
                        /* fallthrough */
                    default:
                        D_0021E8A4 = 1;
                        var_s0 = 0;
                        D_00171D3D = 0;
                        if (*D_0021DDA4 > 0) {
                            var_s2 = D_002121A0;
                            do {
                                temp_v0 = func_L01_006658C0(var_s0);
                                if (temp_v0 != NULL) {
                                    temp_v0_2 = temp_v0->f2F28;
                                    if (temp_v0_2 != NULL) {
                                        temp_a1 = temp_v0_2->f4;
                                        if (temp_a1 != NULL) {
                                            func_L01_0055D3B8(var_s2, temp_a1, 0xC0U);
                                        }
                                    }
                                }
                                temp_v1_2 = D_0021DDA4;
                                var_s0 += 1;
                                var_s2 += 0xC0;
                            } while (var_s0 < *temp_v1_2);
                        }
                        func_L01_0055D3B8(D_002124F0, D_0021DFE8, 0x1C4U);
                        var_s2_2 = 0;
                        D_0021DFE8 = D_002124F0;
                        func_0015C750(D_00172170);
                        func_0015B370();
                        func_01E9E798();
                        func_0015C760();
                        D_001723B0 = 1;
                        D_001723B8 = 1;
                        D_001723B4 = D_00172170;
                        if (*D_0021DDA4 > 0) {
                            var_s0_2 = D_002121A0;
                            do {
                                temp_v0_3 = func_L01_006658C0(var_s2_2);
                                if (temp_v0_3 != NULL) {
                                    temp_v0_4 = temp_v0_3->f2F28;
                                    if (temp_v0_4 != NULL) {
                                        temp_v0_4->f4 = var_s0_2;
                                    }
                                }
                                var_s2_2 += 1;
                                var_s0_2 += 0xC0;
                            } while (var_s2_2 < *D_0021DDA4);
                        }
                        break;
                    }
                }
            } else {
                func_0015ADA8();
                func_L01_00653028();
            }
            func_L01_00637588();
            func_L01_005BA3B8();
            func_L01_00586950();
            func_L01_005B5940();
            func_L01_006272C8();
            func_L01_00573C58();
            func_L01_006666A8();
            if ((D_0021E824 == 0) && (D_0021DDB4 != 0xB) && ((D_001711B9 == 0) || (D_001711BD != 0))) {
                func_L01_006A8340();
            }
            func_L01_005BEC10();
            *D_0021EE00 = (ObjectQuad)0;

            if ((D_0021E818 != 0) || (func_L01_00538610(), (D_0021E818 != 0))) {
                temp_v0_5 = D_0021E824;
                if ((temp_v0_5 == 0) || (D_0021E8A0 != 0)) {
                    func_L01_00625DA8();
                }
            } else {
                func_L01_00624C88();
            }
            var_s0_3 = 1;
            func_L01_00574B28();
            func_L01_00587E20();
            D_00171B40 = (D_00171B40 <= 0x05F5E0FF) ? D_00171B40 : 0x05F5E0FF;
            if ((func_L01_0053A470() != 0) && (func_L01_005D8228(0) != 3)) {
                var_s0_3 = ((func_L01_005D8248(0) ^ 3) != 0) ? 0 : 1;
            }
            if (var_s0_3 != 0) {
                var_s0_4 = 0;
                func_L01_005D7010();
                if (*D_0021DDA4 > 0) {
                    do {
                        func_L01_005D7058(var_s0_4);
                        var_s0_4 += 1;
                    } while (var_s0_4 < *D_0021DDA4);
                }
                func_L01_005D7058(4);
            }
            func_L01_0056F440();
            func_L01_00630388();
            temp_v0_6 = (s32)((u32)D_0021E8FC - 1U);
            D_0021E8FC = (temp_v0_6 <= -1) ? 0 : temp_v0_6;
        }
    }
}
