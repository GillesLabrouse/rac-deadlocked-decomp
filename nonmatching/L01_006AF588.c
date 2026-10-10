/* Dreadzone's pre-loop reset of twenty records at 0037B320.
 * Independently recovered from L01: 232/232 bytes with default SN flags, but
 * unresolved register/scheduling differences remain after exact linking.
 * Native compilation preserves the guest record layout and runs the actual
 * reset algorithm, including the subsequent seven-counter reset. Guest
 * addresses use the observed signed comparison in the PS2 build; the native
 * build compares pointers within its own twenty-record storage.
 */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_record_reset.h"
#else
typedef unsigned int DreadzoneQuad __attribute__((mode(TI)));
typedef struct DreadzoneResetRecord {
    u32 f0;
    u8 gap4[0x24C];
    DreadzoneQuad f250;
    DreadzoneQuad f260;
    u8 gap270[0x40];
    u32 f2B0, f2B4, f2B8, f2BC;
    u8 gap2C0[0x18];
    u32 f2D8;
    s16 f2DC;
    u8 gap2DE[6];
    s16 f2E4;
    u16 f2E6;
    f32 f2E8, f2EC;
    u32 f2F0;
    u8 gap2F4[4];
    u32 f2F8;
    u8 gap2FC[4];
    u32 f300, f304;
    u8 gap308[8];
    u32 f310, f314;
    u8 gap318[0x10];
    u32 f328, f32C;
    u8 gap330[0x10];
} DreadzoneResetRecord;
extern DreadzoneResetRecord D_0037B320[20];
extern u32 D_00220844;
extern u32 D_0037F428[7];
extern void func_L01_0055D2A8(u32 *, u32, s32);
#define RESET_RECORDS D_0037B320
#define RESET_WORDS ((u32 *)D_0037B320)
#define ZERO_QUAD ((DreadzoneQuad)0)
#define RECORD_BEFORE(a, b) ((s32)(a) < (s32)(b))
#endif

#ifdef DEADLOCKED_NATIVE
#define RESET_OFFSET(type, field) offsetof(type, field)
#define RECORD_BEFORE(a, b) ((a) < (b))
#else
#define RESET_OFFSET(type, field) ((u32)&((type *)0)->field)
#endif
typedef char check_reset_size[sizeof(DreadzoneResetRecord) == 0x340 ? 1 : -1];
typedef char check_reset250[RESET_OFFSET(DreadzoneResetRecord, f250) == 0x250 ? 1 : -1];
typedef char check_reset2D8[RESET_OFFSET(DreadzoneResetRecord, f2D8) == 0x2D8 ? 1 : -1];
typedef char check_reset2E4[RESET_OFFSET(DreadzoneResetRecord, f2E4) == 0x2E4 ? 1 : -1];
typedef char check_reset2E8[RESET_OFFSET(DreadzoneResetRecord, f2E8) == 0x2E8 ? 1 : -1];
typedef char check_reset310[RESET_OFFSET(DreadzoneResetRecord, f310) == 0x310 ? 1 : -1];
typedef char check_reset328[RESET_OFFSET(DreadzoneResetRecord, f328) == 0x328 ? 1 : -1];
#undef RESET_OFFSET

void func_L01_006AF588(void) {
    DreadzoneResetRecord *record = RESET_RECORDS;
    DreadzoneResetRecord *end;
    s32 index;
    D_00220844 = 0;
    func_L01_0055D2A8(RESET_WORDS, 0, 0x4100);
    end = record + 20;
    do {
        record->f0 = 0xFFFFFFFFU;
        record->f250 = ZERO_QUAD;
        record->f260 = ZERO_QUAD;
        record->f2B0 = 0;
        record->f2B4 = 0;
        record->f2B8 = 0;
        record->f2BC = 0;
        record->f328 = 0;
        record->f32C = 0;
        record->f2D8 = 0;
        record->f2E8 = 40.0f;
        record->f2EC = 40.0f;
        record->f2F0 = 0;
        record->f2DC = -1;
        record->f2E4 = -1;
        record->f2F8 = 0;
        record->f300 = 0;
        record->f304 = 0;
        record->f2E6 = 0;
        record->f310 = 0;
        record->f314 = 0;
        record++;
    } while (RECORD_BEFORE(record, end));
    for (index = 6; index >= 0; index--) {
        D_0037F428[index] = 0;
    }
}
