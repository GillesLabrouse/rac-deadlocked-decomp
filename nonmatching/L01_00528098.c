/* cflags: -mno-split-addresses */
/* Dreadzone Station entry 0x00528098, retail extent 0x11E8 bytes.
 * NONMATCHING: SN GCC currently produces 4452 bytes, versus 4584 in retail.
 * Stack size (0x80 versus 0x70), register allocation, mixed GP/absolute global
 * accesses and block scheduling differ. Several callee signatures remain
 * inferred and must be reviewed against their bodies before acceptance.
 *
 * This draft uses actual L01 call/data bindings, not L00 multiplayer addresses.
 * The 21-entry state table's relative targets agree between the two overlays;
 * 32 observed data bindings change. Grouping by normalized code alone does not
 * verify those bindings or any callee's semantics.
 *
 * Guest pointers, record layouts and timer address 0x10000800 are PS2-specific.
 * Do not add this routine to a host build before replacing those boundaries.
 * The 32-byte request payload initializes only f0 and f6, as observed; its other
 * bytes remain unspecified. No campaign execution or native gameplay is claimed.
 */
#include "common.h"

typedef struct EntryRow304 {
    u8 pad0[0x300]; u8 f300; u8 pad301[3];
} EntryRow304;
typedef struct EntryState {
    u8 pad0[0x104]; u8 f104[10]; u8 f10E[10];
    u8 pad118[0x84]; s32 f19C; u8 pad1A0[6]; u8 f1A6;
} EntryState;
typedef struct EntryRecordAD0 {
    u8 pad0[8]; s16 f8; s16 fA; u8 padC[2]; s16 fE;
    u8 pad10[0xAC0];
} EntryRecordAD0;
typedef struct EntryChild { s32 f0; s32 f4; } EntryChild;
typedef struct EntryRecord31F0 {
    u8 pad0[0x2F10]; s32 f2F10; u8 pad2F14[0x14];
    EntryChild *f2F28; u8 pad2F2C[0x2C4];
} EntryRecord31F0;
typedef struct EntryRecord470 { s32 f0; u8 pad4[0x46C]; } EntryRecord470;
typedef struct EntryPayload20 {
    s32 f0; u8 pad4[2]; u8 f6; u8 pad7[0x19];
} EntryPayload20;
extern u8 D_001170D0[];
extern u8 D_001711B9_[];
#define D_001711B9 (D_001711B9_[0])
extern s16 D_00171B52_[];
#define D_00171B52 (D_00171B52_[0])
extern s16 D_00171B74_[];
#define D_00171B74 (D_00171B74_[0])
extern u8 D_00171BA4_[];
#define D_00171BA4 (D_00171BA4_[0])
extern s32 D_00171D38_[];
#define D_00171D38 (D_00171D38_[0])
extern s32 D_00172168_[];
#define D_00172168 (D_00172168_[0])
extern s32 D_00172180_[];
#define D_00172180 (D_00172180_[0])
extern s32 D_00172378_[];
#define D_00172378 (D_00172378_[0])
extern u8 D_001723B0_[];
#define D_001723B0 (D_001723B0_[0])
extern s32 D_001723B4_[];
#define D_001723B4 (D_001723B4_[0])
extern s32 D_001723B8_[];
#define D_001723B8 (D_001723B8_[0])
extern s32 D_001723BC_[];
#define D_001723BC (D_001723BC_[0])
extern u8 D_00172829[];
extern s32 D_0017292C[];
extern s32 D_00172954_[];
#define D_00172954 (D_00172954_[0])
extern u8 D_00172958_[];
#define D_00172958 (D_00172958_[0])
extern s16 D_001DFC48_[];
#define D_001DFC48 (D_001DFC48_[0])
extern s16 D_001DFC4A_[];
#define D_001DFC4A (D_001DFC4A_[0])
extern s32 D_001DFC60_[];
#define D_001DFC60 (D_001DFC60_[0])
extern s32 D_001DFC64_[];
#define D_001DFC64 (D_001DFC64_[0])
extern s32 D_001DFC80_[];
#define D_001DFC80 (D_001DFC80_[0])
extern s32 D_001DFC84_[];
#define D_001DFC84 (D_001DFC84_[0])
extern s32 D_001DFCA0_[];
#define D_001DFCA0 (D_001DFCA0_[0])
extern s32 D_001DFCB8_[];
#define D_001DFCB8 (D_001DFCB8_[0])
extern s32 D_001DFCD0_[];
#define D_001DFCD0 (D_001DFCD0_[0])
extern EntryRow304 D_001F87F0[];
extern s64 D_0021DA00_[];
#define D_0021DA00 (D_0021DA00_[0])
extern s32 D_0021DBD8;
extern s32 D_0021DD5C_[];
#define D_0021DD5C (D_0021DD5C_[0])
extern s32 D_0021DD60_[];
#define D_0021DD60 (D_0021DD60_[0])
extern s32 D_0021DD94_[];
#define D_0021DD94 (D_0021DD94_[0])
extern s32 D_0021DD9C;
extern s32 D_0021DDA0;
extern s32 *D_0021DDA4;
extern s32 *D_0021DDA8_[];
#define D_0021DDA8 (D_0021DDA8_[0])
extern s32 D_0021DDB4;
extern s32 D_0021DE10;
extern u8 D_0021DE18_[];
#define D_0021DE18 (D_0021DE18_[0])
extern s32 D_0021DE6C_[];
#define D_0021DE6C (D_0021DE6C_[0])
extern s32 D_0021DE80;
extern s32 D_0021DEC4_[];
#define D_0021DEC4 (D_0021DEC4_[0])
extern EntryState *D_0021DFE8;
extern s32 D_0021E1D8;
extern s32 D_0021E1DC;
extern u8 D_0021E1E4;
extern s32 D_0021E1E8;
extern u8 D_0021E1EC;
extern s32 D_0021E80C;
extern s32 D_0021E810_[];
#define D_0021E810 (D_0021E810_[0])
extern s32 D_0021E814_[];
#define D_0021E814 (D_0021E814_[0])
extern s32 D_0021E818;
extern s32 D_0021E81C;
extern s32 D_0021E820_[];
#define D_0021E820 (D_0021E820_[0])
extern s32 D_0021E824;
extern s32 D_0021E828;
extern s8 D_0021E86A;
extern s32 D_0021EDE0;
extern s32 D_00222900;
extern s32 D_00222904;
extern s32 D_00222A4C;
extern f32 D_00222A50_[];
#define D_00222A50 (D_00222A50_[0])
extern f32 D_00222A54_[];
#define D_00222A54 (D_00222A54_[0])
extern f32 D_00222A58_[];
#define D_00222A58 (D_00222A58_[0])
extern f32 D_00222A5C_[];
#define D_00222A5C (D_00222A5C_[0])
extern s32 D_00222B80_[];
#define D_00222B80 (D_00222B80_[0])
extern s32 D_00222B84;
extern f32 D_00222B88_[];
#define D_00222B88 (D_00222B88_[0])
extern s32 D_00222B90;
extern EntryState D_00223E40;
extern EntryRecord470 D_002306E8[];
extern s32 D_002437AC_[];
#define D_002437AC (D_002437AC_[0])
extern s32 D_002437B0_[];
#define D_002437B0 (D_002437B0_[0])
extern s32 D_002437B4_[];
#define D_002437B4 (D_002437B4_[0])
extern EntryRecord31F0 *D_003660F8[];
extern s32 D_00368BE8_[];
#define D_00368BE8 (D_00368BE8_[0])
extern EntryRecord31F0 D_00368F60[];
extern EntryRecordAD0 D_01F31380[];
void func_L01_005BE7B0(void);
void func_L01_0054D2C8(void);
void func_L01_0064B298(void);
void func_L01_0055E7C0(u64, s32, s32, s32, f32, f32, f32, f32, f32, f32, s32, u64);
void func_L01_00563FD0(s32, s32, s32, s32, s32, s32);
s32 func_01E9E270(s32, s32, s32, s32, s32, EntryPayload20 *);
#define ENTRY_OFFSET(type, field) ((u32)&(((type *)0)->field))
typedef char check_state10E[ENTRY_OFFSET(EntryState, f10E) == 0x10E ? 1 : -1];
typedef char check_state19C[ENTRY_OFFSET(EntryState, f19C) == 0x19C ? 1 : -1];
typedef char check_state1A6[ENTRY_OFFSET(EntryState, f1A6) == 0x1A6 ? 1 : -1];
typedef char check_record2F10[ENTRY_OFFSET(EntryRecord31F0, f2F10) == 0x2F10 ? 1 : -1];
typedef char check_record2F28[ENTRY_OFFSET(EntryRecord31F0, f2F28) == 0x2F28 ? 1 : -1];
typedef char check_payload6[ENTRY_OFFSET(EntryPayload20, f6) == 6 ? 1 : -1];
#undef ENTRY_OFFSET

typedef char check_row304[sizeof(EntryRow304) == 0x304 ? 1 : -1];
typedef char check_recordAD0[sizeof(EntryRecordAD0) == 0xAD0 ? 1 : -1];
typedef char check_record31F0[sizeof(EntryRecord31F0) == 0x31F0 ? 1 : -1];
typedef char check_record470[sizeof(EntryRecord470) == 0x470 ? 1 : -1];
typedef char check_payload20[sizeof(EntryPayload20) == 0x20 ? 1 : -1];
void func_00138D70(s32);                                 /* extern */
void func_00157F98();                                  /* extern */
void func_00159120();                                  /* extern */
void func_0015ADA8();                                  /* extern */
void func_0015EF00(EntryRecordAD0 *);                  /* extern */
void func_00161CF0();                                  /* extern */
void func_L01_00527B08();                              /* extern */
void func_L01_00527CE0();                              /* extern */
void func_L01_00529280();                              /* extern */
void func_L01_00529310();                              /* extern */
void func_L01_00529360();                              /* extern */
void func_L01_00529660();                              /* extern */
void func_L01_00529978();                              /* extern */
void func_L01_005299F8(s32);                             /* extern */
void func_L01_00529F38();                              /* extern */
void func_L01_0052A448();                              /* extern */
void func_L01_0052A628();                              /* extern */
void func_L01_0052A648();                              /* extern */
void func_L01_0052C5C8();                              /* extern */
void func_L01_0052C5D0();                              /* extern */
void func_L01_0052C5E8();                              /* extern */
void func_L01_00533810();                              /* extern */
void func_L01_00533EF0();                              /* extern */
void func_L01_00538610();                              /* extern */
void func_L01_00545AC0();                              /* extern */
void func_L01_005478F0();                              /* extern */
void func_L01_00549918();                              /* extern */
void func_L01_00549940();                              /* extern */
void func_L01_00549968();                              /* extern */
void func_L01_00549A20();                              /* extern */
void func_L01_00549AE8();                              /* extern */
void func_L01_00549B28();                              /* extern */
void func_L01_0054A7C8(s32, s32);                          /* extern */
void func_L01_0054AEC0();                              /* extern */
void func_L01_0054D080(s64);                           /* extern */
s32 func_L01_0055D148(s32 *);                       /* extern */
void func_L01_0055DFE8();                              /* extern */
void func_L01_0055E0A0(s32);                           /* extern */
void func_L01_00563D70();                              /* extern */
void func_L01_00564B30();                              /* extern */
void func_L01_00564DE0();                              /* extern */
void func_L01_00565170(s32);                             /* extern */
void func_L01_005663E0();                              /* extern */
void func_L01_00566540();                              /* extern */
s32 func_L01_00570620(s32);                           /* extern */
void func_L01_005742C8();                              /* extern */
void func_L01_00574730();                              /* extern */
void func_L01_005747A0();                              /* extern */
void func_L01_005756D0(s16, s32);                        /* extern */
void func_L01_00575DC0(u8);                            /* extern */
void func_L01_00575EA0(s32);                           /* extern */
void func_L01_00575F80(s32);                           /* extern */
void func_L01_00578940(s32, s32);                          /* extern */
void func_L01_0057EAC0();                              /* extern */
void func_L01_005800D0();                              /* extern */
void func_L01_00584038();                              /* extern */
void func_L01_0058AD10();                              /* extern */
void func_L01_0058B338();                              /* extern */
void func_L01_0058B6D0();                              /* extern */
void func_L01_005A9030();                              /* extern */
void func_L01_005A95C8();                              /* extern */
void func_L01_005A9608(s32, s32, s32, s32, s32);                 /* extern */
s32 func_L01_005A9910();                            /* extern */
void func_L01_005B5C38(s32, s32, s32);                       /* extern */
void func_L01_005B6128(s32);                             /* extern */
void func_L01_005B6148();                              /* extern */
void func_L01_005D6988();                              /* extern */
void func_L01_005D6CB0();                              /* extern */
void func_L01_005D6CE8();                              /* extern */
void func_L01_005D7048();                              /* extern */
void func_L01_005D7138();                              /* extern */
void func_L01_005D7170();                              /* extern */
void func_L01_005D71A8(s32);                             /* extern */
void func_L01_00614FD8();                              /* extern */
void func_L01_00615920();                              /* extern */
void func_L01_00624C88();                              /* extern */
void func_L01_00627D18();                              /* extern */
void func_L01_00628020();                              /* extern */
void func_L01_0062A778(s32, s32);                        /* extern */
void func_L01_0062A918(s32);                           /* extern */
void func_L01_0062FE40(s32, s32, s32, s32, s32, s32);              /* extern */
void func_L01_00630388();                              /* extern */
void func_L01_00630438();                              /* extern */
void func_L01_00630930();                              /* extern */
void func_L01_00631648();                              /* extern */
void func_L01_00633178(u8 *);                          /* extern */
void func_L01_00633360();                              /* extern */
void func_L01_006333B0();                              /* extern */
void func_L01_00633430();                              /* extern */
void func_L01_00633558(s32);                             /* extern */
void func_L01_0063A8B0();                              /* extern */
void func_L01_0063A940(s32);                             /* extern */
void func_L01_006532E0();                              /* extern */
void func_L01_00665BD0(EntryRow304 *, s32, s32, s32);    /* extern */
void func_L01_00665CE8();                              /* extern */
void func_L01_00665D58();                              /* extern */
void func_L01_00666058();                              /* extern */
s32 func_L01_00666600();                            /* extern */
void func_L01_006C1D68();                              /* extern */

void func_L01_00528098(void) {
    s32 var_s7;
    EntryPayload20 message;
    EntryChild *temp_v1_2;
    EntryRecord31F0 **var_a2;
    EntryRecord31F0 *var_a1_4;
    EntryRecord31F0 *var_s2_2;
    EntryRecord31F0 *var_v0_5;
    EntryRecord470 *var_a0_2;
    EntryRecordAD0 *var_s0;
    EntryRow304 *temp_v1;
    f32 var_f0;
    f32 var_f1;
    s16 var_v0_4;
    s32 temp_s0;
    s32 temp_s6;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v0_5;
    s32 var_a0;
    s32 var_a0_3;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a1_3;
    s32 var_a3;
    s32 var_a5;
    s32 var_s1;
    s32 var_s2;
    s32 var_s5;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1;
    s64 temp_a0;
    u8 *temp_a4;
    u8 *var_v0_3;

    /* Flowgraph is not reducible, falling back to gotos-only mode. */
    if (D_0021DE10 == 0) {
        goto block_2;
    }
    if ((u32) (D_0021DE10 - 0x29) >= 0x27U) {
        goto block_3;
    }
block_2:
    D_0021E824 = 1;
block_3:
    func_L01_006C1D68();
    func_L01_00529280();
    func_L01_00574730();
    func_L01_0055DFE8();
    if (D_0021DE10 == 0) {
        goto block_5;
    }
    var_v0 = 0;
    if (D_0021DE10 < 0x28) {
        goto block_6;
    }
block_5:
    var_v0 = 1;
block_6:
    if (var_v0 == 0) {
        goto block_8;
    }
    func_00161CF0();
    func_0015ADA8();
    D_0021E824 = 1;
    D_0021E818 = *D_0021DDA4 - 1;
    func_L01_0064B298();
    D_0021EDE0 = 0;
    goto block_12;
block_8:
    D_0021E824 = 0;
    if (func_L01_00666600() == 0) {
        goto block_10;
    }
    *D_0021DDA4 = 2;
    D_0021E828 = 1;
    D_0021E818 = 1;
    func_L01_0064B298();
    goto block_11;
block_10:
    *D_0021DDA4 = 1;
    D_0021E818 = 0;
    D_0021E828 = 0;
block_11:
    D_0021DFE8 = &D_00223E40;
    D_00172180 = *D_0021DDA4;
block_12:
    temp_v0 = D_0021DE10;
    if (*D_0021DDA4 < 2) {
        goto block_14;
    }
    func_L01_00527B08();
block_14:
    if (temp_v0 == 0) {
        goto block_19;
    }
    if (temp_v0 < 0x28) {
        goto block_18;
    }
    if (D_001723BC != 0) {
        goto block_18;
    }
    func_L01_00665CE8();
    goto block_19;
block_18:
    func_L01_00665D58();
block_19:
    func_L01_00628020();
    if (D_0021DE10 != 0) {
        goto block_26;
    }
    if (D_0021DE80 == 4) {
        goto block_24;
    }
    if (D_0021DE80 == 5) {
        goto block_24;
    }
    if (D_0021DE80 == 1) {
        goto block_24;
    }
    if (D_0021DE80 != 2) {
        goto block_25;
    }
block_24:
    D_0021E810 = -2;
    D_0021E814 = 0xA;
    D_0021DDB4 = 0;
    goto block_27;
block_25:
    D_0021DDB4 = 0;
    D_0021E814 = 0xA;
    D_0021E810 = 0;
    D_0021DE80 = 0;
    goto block_27;
block_26:
    D_0021E814 = 0xF;
    D_0021DDB4 = 0xF;
    D_0021E810 = 0xF;
block_27:
    if (D_0021E824 != 0) {
        goto block_29;
    }
    D_0021DDB4 = 0;
    D_0021E814 = -2;
    D_0021E810 = 0;
block_29:
    D_00222B84 = 0;
    func_L01_005742C8();
    temp_v0_2 = D_0021E824;
    if (temp_v0_2 != 1) {
        goto block_31;
    }
    func_00161CF0();
    func_0015ADA8();
block_31:
    if (temp_v0_2 != 0) {
        goto block_45;
    }
    D_001DFC64 = D_002437B0;
    if (D_001DFC60 == D_00171B52) {
        goto block_34;
    }
    func_L01_005756D0(D_00171B52, 0);
block_34:
    if (D_0021E824 != 0) {
        goto block_38;
    }
    if (D_0021E828 != 1) {
        goto block_38;
    }
    D_001DFC84 = D_002437B4;
    if (D_001DFC80 == 5) {
        goto block_39;
    }
    func_L01_005756D0(D_00171B74, 1);
    goto block_39;
block_38:
    D_001DFC80 = -1;
    D_002437AC = D_002437B0;
    D_001DFC84 = 0;
block_39:
    if (D_001DFCA0 == D_00171BA4) {
        goto block_41;
    }
    func_L01_00575DC0(D_00171BA4);
block_41:
    if (D_001DFCB8 == D_0021DD60) {
        goto block_43;
    }
    func_L01_00575EA0(D_0021DD60);
block_43:
    if (D_001DFCD0 == D_0021DD5C) {
        goto block_46;
    }
    func_L01_00575F80(D_0021DD5C);
    goto block_46;
block_45:
    D_001DFC60 = -1;
    D_001DFC80 = -1;
    D_001DFC84 = 0;
    D_001DFC64 = 0;
block_46:
    func_L01_005D6CB0();
    func_L01_005D6CE8();
    func_L01_00578940(1, 0);
    if (D_0021E824 != 1) {
        goto block_49;
    }
    func_00161CF0();
    func_0015ADA8();
    if (D_0021DE10 != 0) {
        goto block_49;
    }
    func_L01_0052C5E8();
block_49:
    func_L01_00584038();
    func_L01_00533810();
    if (D_0021E824 != 0) {
        goto block_51;
    }
    if (D_0021E828 == 1) {
        goto block_55;
    }
block_51:
    if (*D_0021DDA4 >= 2) {
        goto block_55;
    }
    if (D_0021E824 != 1) {
        goto block_56;
    }
    if (D_0021DE10 == 0x2D) {
        goto block_55;
    }
    if (D_0021DE10 != 0x2A) {
        goto block_56;
    }
block_55:
    func_L01_0062A918(D_0021E824);
block_56:
    func_L01_0052A448();
    if (D_0021E824 != 1) {
        goto block_58;
    }
    func_00161CF0();
    func_0015ADA8();
block_58:
    D_001DFC4A = 1;
    if (D_001DFC48 == 0) {
        goto block_60;
    }
    D_001DFC48 = 0;
block_60:
    D_0021DD94 = 0;
    var_a1 = 0;
    temp_v1 = &D_001F87F0[(s32) (D_0021DE10 - 1) % 20];
    temp_v1->f300 |= 4;
    if (D_0021DE10 == 0) {
        goto block_62;
    }
    var_v0_2 = 0;
    if (D_0021DE10 < 0x28) {
        goto block_63;
    }
block_62:
    var_a1 = 1;
    var_v0_2 = 1;
block_63:
    temp_v0_3 = D_0021DE10;
    if (var_v0_2 == 0) {
        goto block_66;
    }
    if (temp_v0_3 == 0) {
        goto block_67;
    }
    func_L01_00665BD0(D_001F87F0, var_a1, 0x304, D_0021DE10);
    D_0021DDB4 = 0xF;
block_66:
block_67:
    D_00222A4C = 0;
    if (temp_v0_3 != 0) {
        goto block_69;
    }
    func_L01_005B5C38(0, 0, 0x400);
    goto block_71;
block_69:
    if (D_0021E824 != 1) {
        goto block_71;
    }
    func_L01_005B5C38(4, 0xD, 0x400);
block_71:
    func_L01_005B6128(0);
    func_00159120();
    func_L01_005A9030();
    if (D_0021DDB4 == 0) {
        goto block_74;
    }
    if (D_0021DDB4 == 0xF) {
        goto block_74;
    }
    if (D_0021DDB4 != 0xA) {
        goto loop_216;
    }
block_74:
    func_L01_005B6148();
    goto block_215;
block_76:
    var_s5 = 0;
    var_s7 = 0;
    temp_a0 = D_0021DA00 + (u64)*(volatile u32 *)0x10000800;
    D_0021DA00 = temp_a0;
    *(volatile u32 *)0x10000800 = 0;
    D_00222B90 = 0;
    temp_s6 = D_0021DDB4;
    func_L01_0054D080(temp_a0);
    func_L01_00633178(D_001170D0);
    func_L01_00633430();
    func_L01_006333B0();
    func_L01_00564DE0();
    if (D_0021DDB4 == 0) {
        goto block_78;
    }
    if (D_0021DDB4 != 2) {
        goto block_79;
    }
block_78:
    func_L01_00565170(1);
    goto block_80;
block_79:
    func_L01_00565170(1);
block_80:
    func_L01_00564B30();
    func_00157F98();
    if (D_00222904 != 0) {
        goto block_82;
    }
    D_00222904 = 1;
    D_00222900 = D_0021DE6C;
block_82:
    if ((u32) (D_0021DDB4 + 2) >= 0x15U) {
        goto block_168;
    }
    switch (D_0021DDB4) {                            /* jump table: jtbl_003AC550 */
case -1:
case 0:
    if (D_0021E818 != 0) {
        goto block_86;
    }
    D_0021DDA0 = 0;
    goto block_90;
block_86:
    if (D_0021E818 <= 0) {
        goto block_168;
    }
    if (D_0021E818 >= 4) {
        goto block_170;
    }
    goto block_93;
block_90:
    D_0021DD9C = 0;
    func_L01_005BE7B0();
    func_00161CF0();
    func_L01_00630438();
    func_00161CF0();
    if (D_0021E1D8 != 1) {
        goto block_92;
    }
    D_0021DA00 += (u64)*(volatile u32 *)0x10000800;
    *(volatile u32 *)0x10000800 = 0;
    func_L01_005BE7B0();
    func_L01_00630438();
    func_00161CF0();
block_92:
    func_L01_00549918();
    func_L01_00527CE0();
    func_00161CF0();
    goto block_169;
block_93:
    func_L01_00529F38();
    temp_v0_4 = D_0021E818;
    if (D_0021E81C == -1) {
        goto block_168;
    }
    D_0021E818 = D_0021E81C;
    D_0021E820 = temp_v0_4;
    D_0021E81C = -1;
    goto block_168;
case 8:
    func_L01_005BE7B0();
    func_L01_0063A940(0);
    func_L01_0063A8B0();
    goto block_169;
case 12:
    func_L01_005BE7B0();
    func_L01_00630438();
    func_L01_00549940();
    goto block_169;
case 16:
    if (func_L01_005A9910() != 0xA) {
        goto block_99;
    }
    func_0015ADA8();
block_99:
    func_L01_00624C88();
    func_L01_00615920();
    goto block_169;
case 10:
    func_L01_005BE7B0();
    func_L01_0052C5D0();
    if (D_0021E80C != 0) {
        goto block_102;
    }
    func_L01_0052C5C8();
    goto block_103;
block_102:
    func_L01_005D7138();
    func_L01_005D71A8(0);
    func_L01_005D7170();
block_103:
    func_L01_00545AC0();
    goto block_169;
case 7:
    func_L01_005BE7B0();
    func_L01_00630930();
    func_L01_00549AE8();
    goto block_169;
case 15:
    D_00222A50 = 1.0f;
    D_00172954 = D_00172378;
    D_00222A5C = 1.0f;
    D_00222A58 = 1.0f;
    D_00222A54 = 1.0f;
    var_s2 = 0;
    func_00161CF0();
    func_0015ADA8();
    if (D_001723B8 == 0) {
        goto block_109;
    }
    if (D_0021E1EC == 0) {
        goto block_108;
    }
    D_0021E1E8 = 0x5DC;
    func_L01_005A9608(0, 2, 0, 0, 0);
    goto block_169;
block_108:
    func_L01_0054A7C8(1, 0);
    func_L01_0054D2C8();
    D_0021E1EC = 1;
    goto block_117;
block_109:
    if (D_001723B0 == 0) {
        goto block_117;
    }
    if (D_0021DFE8->f10E[0] == 8) {
        goto block_117;
    }
    var_a5 = 0;
    var_a1_2 = *D_0021DDA4 + *D_0021DDA8;
    if (var_a1_2 >= 0xA) {
        goto block_116;
    }
    temp_a4 = D_0021DFE8->f104;
    var_v0_3 = &temp_a4[var_a1_2];
loop_113:
    if (*var_v0_3 == 0xFF) {
        goto block_115;
    }
    var_a5 = ((D_0021DFE8->f10E[var_a1_2] ^ 8) == 0) ? (var_a5 + 1) : var_a5;
block_115:
    var_a1_2 += 1;
    var_v0_3 = &temp_a4[var_a1_2];
    if (var_a1_2 < 0xA) {
        goto loop_113;
    }
block_116:
    var_s2 = var_a5 == ((D_0021DFE8->f1A6 - *D_0021DDA4) - *D_0021DDA8);
block_117:
    if (D_001723B8 != 0) {
        goto block_157;
    }
    var_s0 = D_01F31380;
    var_v0_4 = D_01F31380[0].fA;
loop_119:
    if (var_v0_4 == 0) {
        goto block_123;
    }
    if (var_s0->fE != 0) {
        goto block_123;
    }
    if (var_s0->f8 != 0) {
        goto block_123;
    }
    func_0015EF00(var_s0);
block_123:
    var_s0 += 1;
    if ((s32) var_s0 >= (s32) (D_01F31380 + 10)) {
        goto block_125;
    }
    var_v0_4 = var_s0->fA;
    goto loop_119;
block_125:
    if (D_001723B0 != 0) {
        goto block_128;
    }
    if (D_0021E1E4 == 0) {
        goto block_129;
    }
block_128:
    if (var_s2 == 0) {
        goto block_147;
    }
block_129:
    var_a2 = D_003660F8;
    var_v0_5 = *D_003660F8;
    var_a3 = 0;
    var_a1_3 = 9;
loop_130:
    if (var_v0_5 != NULL) {
        goto block_132;
    }
    var_a2 += 1;
    goto block_136;
block_132:
    temp_v1_2 = var_v0_5->f2F28;
    var_a2 += 1;
    if (temp_v1_2 == NULL) {
        goto block_136;
    }
    if (temp_v1_2->f4 == 0) {
        goto block_136;
    }
    var_a3 += 1;
    D_0017292C[D_00172829[temp_v1_2->f0]] = D_00172378;
block_136:
    var_a1_3 -= 1;
    var_v0_5 = *var_a2;
    if (var_a1_3 >= 0) {
        goto loop_130;
    }
    if (var_a3 != (D_0021DFE8->f1A6 + *D_0021DDA8)) {
        goto block_147;
    }
    var_a1_4 = D_00368F60;
    D_0021E1E4 = 1;
    var_s1 = 0;
    if (*D_0021DDA4 <= 0) {
        goto block_147;
    }
    var_s2_2 = D_00368F60;
loop_140:
    if (D_001723B0 == 0) {
        goto block_142;
    }
    *((u8 *)D_0021DFE8 + 0x10E + var_a1_4->f2F10) = 8;
    D_0021DFE8->f19C = D_00172378 - 0x226;
    goto block_146;
block_142:
    message.f6 = 8;
    var_a0 = 0;
    message.f0 = var_a1_4->f2F10;
    if (D_001723B8 != 0) {
        goto block_144;
    }
    var_a0 = func_01E9E270(D_00172958 | 0x40, D_00172168, D_001723B4, D_0021DBD8, 0x20, &message);
block_144:
    if (var_a0 == 0) {
        goto block_146;
    }
    D_0021E1E4 = 0;
block_146:
    var_s2_2 += 1;
    var_s1 += 1;
    var_a1_4 = var_s2_2;
    if (var_s1 < *D_0021DDA4) {
        goto loop_140;
    }
block_147:
    if (func_L01_0055D148(&D_0021E1E8) == 0) {
        goto block_149;
    }
    D_0021DFE8->f10E[0] = 8;
    D_0021DFE8->f19C = D_00172378;
    D_0021E1E8 = 0x5DC;
block_149:
    if (D_0021DFE8->f10E[0] != 8) {
        goto block_156;
    }
    if (D_0021E1EC == 0) {
        goto block_155;
    }
    D_0021E1E4 = 0;
    D_0021E1E8 = 0x5DC;
    func_L01_005A9608(0, 2, 0, 0, 0);
    var_v1 = 0;
    if (*D_0021DDA4 <= 0) {
        goto block_157;
    }
    var_a0_2 = D_002306E8;
loop_153:
    var_a0_2->f0 = 2;
    var_v1 += 1;
    var_a0_2 += 1;
    if (var_v1 < *D_0021DDA4) {
        goto loop_153;
    }
    goto block_157;
block_155:
    D_0021E1EC = 1;
    goto block_157;
block_156:
    func_L01_0054D2C8();
block_157:
    func_L01_00630388();
    func_L01_00538610();
    func_L01_00549918();
    if (D_00222B84 < 0x1F5) {
        goto block_170;
    }
    temp_s0 = D_0021DEC4;
    func_L01_0055E0A0(0);
    func_L01_0055E7C0((u64)0x80808080, func_L01_00570620(0x1DB2), -1, 4, 256.0f, 150.0f, 1.5f, 1.5f, 0.0f, 0.0f, 0, (u64)0x80000000);
    func_L01_0055E0A0(temp_s0);
    goto block_169;
case 1:
    func_L01_0062FE40(0, 0, 0, 0, 0, 0);
    goto block_169;
case 2:
    func_L01_005BE7B0();
    func_L01_00631648();
    func_L01_00549968();
    func_L01_0054AEC0();
    goto block_169;
case 3:
    func_L01_005BE7B0();
    func_L01_00614FD8();
    func_L01_00549B28();
    goto block_169;
case 4:
    if (func_L01_005A9910() == 0xA) {
        goto block_164;
    }
    if (func_L01_005A9910() != 0x10) {
        goto block_165;
    }
block_164:
    func_0015ADA8();
block_165:
    func_L01_005BE7B0();
    func_L01_00566540();
    func_L01_00549A20();
    func_L01_00527CE0();
    func_L01_005663E0();
    goto block_169;
case 5:
case 6:
    func_L01_005BE7B0();
    goto block_169;
case 9:
    func_L01_005BE7B0();
    func_L01_0052A628();
    func_L01_0052A648();
default:
block_168:
    ;
    }
block_169:
block_170:
    D_00222B84 += 1;
    func_L01_00633178(D_001170D0);
    func_L01_005D7048();
    func_L01_005A95C8();
    if (D_0021DDB4 != 0) {
        goto block_172;
    }
    func_L01_00533EF0();
block_172:
    func_L01_005800D0();
    func_L01_0057EAC0();
    if (D_0021E824 != 0) {
        goto block_175;
    }
    if (D_0021E828 != 1) {
        goto block_175;
    }
    func_L01_00529310();
block_175:
    func_L01_00529360();
    func_L01_0058B338();
    func_L01_0058AD10();
    func_L01_0058B6D0();
    func_L01_00633558(1);
    temp_v0_5 = *(volatile u32 *)0x10000800;
    if (temp_v0_5 < 0) {
        goto block_177;
    }
    var_f1 = (f32) temp_v0_5;
    goto block_178;
block_177:
    var_f1 = 2.0f * (f32) ((temp_v0_5 & 1) | ((u32) temp_v0_5 >> 1));
block_178:
    if (D_00171D38 == 1) {
        goto block_183;
    }
    if (D_00171D38 == 0) {
        goto block_182;
    }
    var_f0 = 19200.0f;
    if (D_00171D38 == 2) {
        goto block_184;
    }
    var_f0 = 0.0f;
    goto block_184;
block_182:
    var_f0 = 9600.0f;
    goto block_184;
block_183:
    var_f0 = 11520.0f;
block_184:
    D_00222B88 = var_f1 / var_f0;
    if (D_00368BE8 != 0) {
        goto block_193;
    }
    if (D_00222B90 != 0) {
        goto block_193;
    }
    if (D_0021DDB4 != 0) {
        goto block_188;
    }
    func_L01_00529660();
    func_L01_005D6988();
    goto block_191;
block_188:
    if (D_0021DDB4 != 2) {
        goto block_190;
    }
    func_L01_00529978();
    func_L01_005D6988();
    goto block_191;
block_190:
    func_00138D70(0);
    D_0021E1D8 = 0;
    D_0021E1DC = 0;
    D_00222A4C += 1;
    func_L01_005D6988();
block_191:
    if (D_0021DDB4 == 3) {
        goto block_193;
    }
    D_0021E86A = 0;
block_193:
    func_L01_005478F0();
    if (D_0021DDB4 != 0) {
        goto block_196;
    }
    if (D_00368BE8 == 0) {
        goto block_196;
    }
    var_s5 = 1;
    var_s7 = 1;
block_196:
    if (D_0021E81C == -1) {
        goto block_201;
    }
    var_a0_3 = 0;
    if (D_0021E818 != 0) {
        goto block_199;
    }
    var_a0_3 = D_0021E81C != 0;
block_199:
    D_0021E820 = D_0021E818;
    D_0021E818 = D_0021E81C;
    D_0021E81C = -1;
    if (var_a0_3 == 0) {
        goto block_201;
    }
    func_L01_0062A778(var_a0_3, -1);
block_201:
    func_L01_006532E0();
    if (D_001711B9 == 0) {
        goto block_203;
    }
    D_001711B9 = 0;
    var_s5 = 1;
block_203:
    if (var_s5 == 0) {
        goto block_205;
    }
    func_L01_005299F8(var_s7);
    goto block_209;
block_205:
    func_L01_005A9030();
    if (D_0021DDB4 == temp_s6) {
        goto block_207;
    }
    D_00222B84 = 0;
block_207:
    if (D_00222B90 == 0) {
        goto block_210;
    }
    func_L01_00633360();
block_209:
block_210:
    if (D_00222900 == D_0021DE6C) {
        goto loop_216;
    }
    D_00222900 = D_0021DE6C;
    func_L01_00563D70();
    if (D_0021DE6C == 0) {
        goto block_213;
    }
    func_L01_00563FD0(0x200, 0x1A0, 0x280, 0x1C0, 0, 0);
    goto block_214;
block_213:
    func_L01_00563FD0(0x200, 0x1A0, 0x200, 0x1C0, 0, 0);
block_214:
    func_L01_0064B298();
block_215:
loop_216:
    if (D_0021E80C == 0) {
        goto block_76;
    }
    if (D_00222B80 == D_0021DE10) {
        goto block_219;
    }
    D_0021DE18 = (u8) D_0021DE10;
block_219:
    func_L01_00666058();
    if (D_0021E824 != 1) {
        goto block_221;
    }
    func_00161CF0();
    func_0015ADA8();
block_221:
    func_L01_00627D18();
    func_L01_005747A0();
    return;
}
