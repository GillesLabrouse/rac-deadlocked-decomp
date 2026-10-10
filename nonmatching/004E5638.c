/* cflags: -mno-split-addresses */
#include "common.h"

/* Experimental transition setup recovered from the resident PS2 routine.
 * The current compile is 1484 bytes versus 1496 in retail, with additional
 * saved-register and instruction-selection differences. Return types of
 * callees whose results are discarded remain provisional.
 * This draft is excluded from matching coverage
 * and the native runtime: its graphics, allocation, asynchronous loading,
 * actor callbacks and guest pointers require recovered host implementations.
 * Several symbols use both GP and absolute accesses in retail, which this
 * draft does not yet reproduce. All record strides below are byte-verified.
 */
typedef struct TransitionActor TransitionActor;
typedef struct ActorCallbacks {
    u8 gap0[0x3C];
    s32 (*f3C)(TransitionActor *, s32, s32, s32, s32);
} ActorCallbacks;

struct TransitionActor {
    u8 gap0[0x14];
    ActorCallbacks *f14;
    u8 gap18[0x264A];
    u8 f2662;
    u8 gap2663[0x88D];
    s32 f2EF0;
};

typedef struct TransitionSession {
    u8 gap0[0x70];
    s32 f70;
    s32 f74;
    s32 f78;
    s32 f7C;
    u8 gap80[8];
    volatile s32 f88;
    u8 gap8C[2];
    s16 f8E;
    u8 gap90[16];
    u8 *fA0;
    u8 *fA4;
    u8 *fA8;
    u8 *fAC;
    u8 *fB0;
    u8 *fB4;
    s32 fB8;
    s32 fBC;
    s32 fC0;
    s32 fC4;
    s32 fC8;
    s16 fCC;
    s16 fCE;
    f32 fD0;
    f32 fD4;
    f32 fD8;
    f32 fDC;
    u8 gapE0[0x150];
} TransitionSession;

typedef struct TransitionArena {
    u8 gap0[12];
    u8 *fC;
    u8 *f10;
} TransitionArena;

typedef struct ShortField4 {
    u8 gap0[4];
    s16 f4;
} ShortField4;

typedef struct ByteField6 {
    u8 gap0[6];
    u8 f6;
} ByteField6;

typedef struct FlagChild {
    u8 gap0[0x34];
    u16 f34;
} FlagChild;

typedef struct FlagRoot {
    u8 gap0[8];
    FlagChild *f8;
} FlagRoot;
extern TransitionSession D_00237000;
extern TransitionArena D_0023AB00;
extern ShortField4 D_00206948;
extern ByteField6 D_00171D38;
extern FlagRoot D_0023EE00;
extern TransitionActor *D_003367B8[11];
extern s32 *D_0021DDA4 __attribute__((section(".data")));
extern u32 D_00171C08[];
extern s32 D_00171FC8[6] __attribute__((section(".data")));
extern u8 D_0023A7C0[64] __attribute__((section(".data"))); extern u8 D_00326880[7][64] __attribute__((section(".data"))); extern u8 D_0023A800[112] __attribute__((section(".data")));
extern f32 D_002211D0[4] __attribute__((section(".data"))); extern s32 D_002211E0[4] __attribute__((section(".data"))); extern u8 D_002214D0[] __attribute__((section(".data")));
extern s32 D_0021DDB4 __attribute__((section(".data")));
extern s32 D_0021DE10;
extern s32 D_0021EDD4;
extern s32 D_0021EDD8 __attribute__((section(".data")));
extern s32 D_0021EE08 __attribute__((section(".data")));
extern s32 D_0021EE0C;
extern s32 D_002206E4;
extern s32 D_002211F0 __attribute__((section(".data")));
extern s32 D_002211F4;
extern s32 D_00221D84 __attribute__((section(".data")));
extern s32 D_00221E28;
extern u8 D_0022122C __attribute__((section(".data")));
extern u8 D_0022122D __attribute__((section(".data")));
extern u8 D_0022122E __attribute__((section(".data")));
extern f32 D_0021DE04;
extern f32 D_0021EE04 __attribute__((section(".data")));
extern f32 D_00221230 __attribute__((section(".data")));
extern f32 D_00221234 __attribute__((section(".data")));
extern f32 D_00221238 __attribute__((section(".data")));
extern f32 D_0022123C __attribute__((section(".data")));
s32 func_00138D70(s32); s32 func_001634A8(s32);
s32 func_004494E8(s32,s32); void *func_00453608(void *,s32,s32); void *func_00453808(void *,void *,s32);
void func_0045C8E8(void *); s32 func_0045DE88(void);
s32 func_00463AC0(s32,s32); s32 func_00463B78(void); s32 func_00463CB8(s32,s32); s32 func_00463D70(s32); s32 func_00463E08(void);
s32 func_00489458(s32); s32 func_00489C28(s32);
s32 func_00489FA0(void *,s32,s32,s32,s32); s32 func_0048A170(void *,s32,s32,s32,s32);
s32 func_0048A230(void *,s32,s32 *); s32 func_0048A290(void *,s32); s32 func_0048A368(void *,s32,f32); s32 func_0048A498(void *,f32);
s32 func_004E0858(void); s32 func_004E2AF8(s32); s32 func_004E49D0(void); s32 func_004E55C0(s32); s32 func_004E7638(void); s32 func_00523780(TransitionActor *);
void func_004E5638(s32);


typedef char transition_session_size_check[sizeof(TransitionSession) == 0x230 ? 1 : -1];
typedef char transition_actor_flag_check[((u32)&(((TransitionActor *)0)->f2662)) == 0x2662 ? 1 : -1];
typedef char transition_actor_active_check[((u32)&(((TransitionActor *)0)->f2EF0)) == 0x2EF0 ? 1 : -1];
typedef char transition_callback_check[((u32)&(((ActorCallbacks *)0)->f3C)) == 0x3C ? 1 : -1];
typedef char transition_session_chunks_check[((u32)&(((TransitionSession *)0)->fCC)) == 0xCC ? 1 : -1];
typedef char transition_session_float_check[((u32)&(((TransitionSession *)0)->fDC)) == 0xDC ? 1 : -1];

void func_004E5638(s32 arg0) {
    TransitionActor **var_s2;
    TransitionActor *var_s0_2;
    f32 *var_v1_2;
    s32 *var_a0_2;
    s32 *var_v0;
    s32 *var_v1_3;
    s32 temp_a2;
    s32 temp_s0;
    s32 temp_v0;
    s32 temp_v0_4;
    s32 var_a0_3;
    s32 var_a0_4;
    s32 var_s0;
    s32 var_s0_3;
    s32 var_s1_2;
    s32 var_s1_3;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v1;
    s32 status[4];
    s32 previous_iteration;
    u32 *temp_a0;
    u8 (*var_a0)[0x40];
    u8 (*var_s1)[0x40];
    u8 *temp_a1;
    u8 *temp_a1_2;
    u8 *temp_a3;
    u8 *temp_v0_2;
    u8 *temp_v0_3;
    u8 *temp_v1;

    if (*D_0021DDA4 >= 2) {
        func_004E49D0();
        D_00221D84 = 0x10000;
    }
    if (arg0 >= 0) {
        temp_a0 = &D_00171C08[(u32) arg0 >> 5];
        *temp_a0 |= 1U << (arg0 & 0x1F);
    }
    func_0045C8E8(D_0023A800);
    temp_v0 = func_004E2AF8(arg0);
    if (temp_v0 < 0) {
        var_v1 = 5;
        var_v0 = &D_00171FC8[5];
        do {
            *var_v0 = 0x400;
            var_v1 -= 1;
            var_v0--;
        } while (var_v1 >= 0);
        return;
    }
    if (D_00237000.f8E == 0) {
        func_00489458(0);
        func_00489C28(0);
    }
    func_00453608(&D_00237000, 0, 0x230);
    func_00453608(D_0023A7C0, 0, 0x40);
    var_s0 = 6;
    var_s1 = D_00326880;
    var_a0 = D_00326880;
loop_11:
    var_s1++;
    func_00453608(var_a0, 0, 0x40);
    var_s0 -= 1;
    if (var_s0 >= 0) {
        var_a0 = var_s1;
        goto loop_11;
    }
    D_0021EE08 = 0;
    D_0021EE04 = 1.0f;
    D_00237000.f70 = temp_v0;
    D_00237000.f74 = 0;
    D_00237000.f7C = 0;
    func_004494E8(2, 0);
    temp_a3 = D_0023AB00.f10;
    D_0023AB00.f10 = D_0023AB00.fC + 0x10000;
    D_00237000.fA0 = D_0023AB00.fC + 0x20000;
    D_00237000.fA4 = &temp_a3[D_00221E28];
    D_00237000.fB0 = D_0023AB00.fC;
    D_00237000.fB4 = temp_a3;
    D_00237000.fB8 = 0;
    D_00237000.fA8 = NULL;
    D_00237000.fAC = NULL;
    D_00237000.fBC = 0;
    D_00221E28 = 0x10000;
    func_004E7638();
    func_0045DE88();
    var_a0_2 = D_002211E0;
    var_v1_2 = D_002211D0;
    var_v0_2 = 3;
    do {
        *var_v1_2 = 1.0f;
        var_v0_2 -= 1;
        *var_a0_2 = 0;
        var_v1_2++;
        var_a0_2++;
    } while (var_v0_2 >= 0);
    D_00237000.fDC = D_0022123C;
    D_0021DDB4 = 2;
    D_00237000.fC0 = (s32) D_0022122C;
    D_00237000.fC4 = (s32) D_0022122D;
    D_00237000.fC8 = (s32) D_0022122E;
    D_00237000.fD0 = D_00221230;
    D_00237000.fD4 = D_00221234;
    D_00237000.fD8 = D_00221238;
    D_0021EE0C = 0;
    if (D_0021EDD4 == 0) {
        var_s0_2 = *D_003367B8;
        var_s2 = D_003367B8;
        var_s1_2 = 9;
        do {
            if ((var_s0_2 != NULL) && (var_s0_2->f2EF0 != 0)) {
                var_s0_2->f14->f3C(var_s0_2, 0x63, 2, 0, 1);
                var_s0_2->f2662 = 1;
                func_00523780(var_s0_2);
            }
            var_s2++;
            var_s1_2 -= 1;
            var_s0_2 = *var_s2;
        } while (var_s1_2 >= 0);
    }
    if (D_0023EE00.f8 != NULL) {
        D_0023EE00.f8->f34 |= 1;
    }
    D_002211F0 = 1;
    D_002211F4 = 0x34;
    if (D_00206948.f4 != 0) {
        func_00489C28(1);
    }
    if (D_00171D38.f6 != 0) {
        var_a0_3 = D_0021DE10;
        if (D_0021EDD4 == 0) {
            var_a0_3 = ((D_0021EDD8 ^ 1) == 0) ? (var_a0_3 - 0x14) : var_a0_3;
        }
        func_00463CB8(var_a0_3, temp_v0);
    }
    var_a0_4 = D_0021DE10;
    if (D_0021EDD4 == 0) {
        var_a0_4 = ((D_0021EDD8 ^ 1) == 0) ? (var_a0_4 - 0x14) : var_a0_4;
    }
    temp_s0 = func_00463AC0(var_a0_4, temp_v0);
    temp_v1 = D_00237000.fA0;
    temp_a1 = temp_v1 + 0x28000;
    D_00237000.fA0 = temp_a1;
    D_00237000.fA8 = temp_v1;
    D_00237000.fAC = temp_a1;
    func_001634A8(1);
    func_00463D70(0);
    func_001634A8(1);
    if (temp_s0 != 0) {
        func_00463B78();
    }
    func_004E55C0(1);
    if (D_00206948.f4 != 0) {
        func_00489C28(1);
    }
    D_00237000.fAC = D_00237000.fA0;
    func_00463E08();
    temp_v0_2 = D_00237000.fA4 - (s32)((u32)(s32)D_00237000.fCC << 11);
    temp_a1_2 = D_00237000.fA8;
    temp_a2 = (s32)((u32)(s32)D_00237000.fCE << 11);
    D_00237000.fAC = temp_v0_2;
    temp_v0_3 = temp_v0_2 - temp_a2;
    D_00237000.fA8 = temp_v0_3;
    D_00237000.fA4 = temp_v0_3;
    func_00453808(temp_v0_3, temp_a1_2, temp_a2);
    func_00463E08();
    temp_v0_4 = ((s32) (D_00237000.fA4 - D_0023AB00.fC) >> 1) & 0xFFFFFFF0;
    D_00221E28 = temp_v0_4;
    D_0023AB00.f10 = &D_0023AB00.fC[temp_v0_4];
    func_004E7638();
    func_00489FA0(D_002214D0, 1, 0x12C, 0, 0);
    D_002206E4 = func_0048A170(D_002214D0, D_00237000.f70, 3, 0, 0);
    var_v1_3 = &D_00171FC8[5];
    var_v0_3 = 5;
    do {
        *var_v1_3 = 0x400;
        var_v0_3 -= 1;
        var_v1_3--;
    } while (var_v0_3 >= 0);
    var_s0_3 = 0;
loop_39:
    func_004E0858();
    func_00138D70(0);
    previous_iteration = var_s0_3;
    var_s0_3 += 1;
    if (previous_iteration < 0x12D) {
        func_0048A230(D_002214D0, D_002206E4, status);
        if (status[0] != 4) {
            goto loop_39;
        }
    }
    var_s1_3 = 0;
    func_0048A368(D_002214D0, D_002206E4, 0.75f);
    D_00237000.f88 = 0;
    func_0048A290(D_002214D0, D_002206E4);
    func_0048A498(D_002214D0, D_0021DE04);
    func_00463D70(1);
    if (D_00237000.f88 < 0) {
        do {
            var_s1_3 -= 1;
            func_00138D70(0);
        } while (D_00237000.f88 < var_s1_3);
    }
}
