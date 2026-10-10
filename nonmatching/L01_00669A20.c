/* cflags: -mno-split-addresses */
/* asflags: --float-immediates */
/* Complete parameter initializer00669A20..00669D30,784 bytes. NONMATCHING.
 * Eighteen16-byte parameter prefixes have aB0-byte stride. No later prologue
 * is part of this function. Six reads use the actual global0021E824. */
#include "common.h"
typedef struct ActorParameterPrefix {
    s16 index, flags;
    f32 first, second, scale;
} ActorParameterPrefix;
typedef char ActorParameterPrefixWidth[sizeof(ActorParameterPrefix)==16 ? 1 : -1];
typedef char ActorParameterFloatWidth[sizeof(f32)==4 ? 1 : -1];
typedef char ActorParameterHalfWidth[sizeof(s16)==2 ? 1 : -1];
typedef char ActorParameterWordWidth[sizeof(u32)==4 ? 1 : -1];
#ifdef DEADLOCKED_NATIVE
#define PARAMETER_OFFSET(member) offsetof(ActorParameterPrefix,member)
#else
#define PARAMETER_OFFSET(member) ((u32)&((ActorParameterPrefix *)0)->member)
#endif
typedef char ActorParameterFlagsOffset[PARAMETER_OFFSET(flags)==2 ? 1 : -1];
typedef char ActorParameterFirstOffset[PARAMETER_OFFSET(first)==4 ? 1 : -1];
typedef char ActorParameterSecondOffset[PARAMETER_OFFSET(second)==8 ? 1 : -1];
typedef char ActorParameterScaleOffset[PARAMETER_OFFSET(scale)==12 ? 1 : -1];
#undef PARAMETER_OFFSET
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_parameters.h"
static void parameter_half(u8 *p,s16 value) {
    u16 bits=(u16)value;
    p[0]=(u8)bits; p[1]=(u8)(bits>>8);
}
static void parameter_float(u8 *p,f32 value) {
    union { f32 f; u32 bits; } copy;
    copy.f=value;
    p[0]=(u8)copy.bits; p[1]=(u8)(copy.bits>>8); p[2]=(u8)(copy.bits>>16); p[3]=(u8)(copy.bits>>24);
}
#define MODE mode
#define PARAMETER(offset,id,flag,a,b) do { \
    parameter_half(record+(offset),(s16)(id)); \
    parameter_half(record+(offset)+2,(s16)(flag)); \
    parameter_float(record+(offset)+4,a); \
    parameter_float(record+(offset)+8,b); \
    parameter_float(record+(offset)+12,1.f); \
} while(0)
int dreadzone_actor_00669A20(const DreadzoneActorParameterView *host) {
    u8 *record;
    s32 mode;
    if(!host || (uintptr_t)host>UINTPTR_MAX-(sizeof(*host)-1) ||
       !host->record || host->record_bytes<0x31F0 || (host->record_guest&3) ||
       host->record_guest>0xFFFFFFFFU-(host->record_bytes-1) ||
       (uintptr_t)host->record>UINTPTR_MAX-(host->record_bytes-1))
        return DREADZONE_ACTOR_PARAMETERS_INVALID_STORAGE;
    record=host->record;
    mode=host->mode_0021E824;
#else
extern volatile s32 D_0021E824;
#define MODE D_0021E824
#define PARAMETER(offset,id,flag,a,b) do { \
    ActorParameterPrefix *p=(ActorParameterPrefix *)(record+(offset)); \
    p->index=(s16)(id); p->flags=(s16)(flag); p->first=a; p->second=b; p->scale=1.f; \
} while(0)
void func_L01_00669A20(u8 *record) {
#endif
    PARAMETER(0x8C0,9,0,0.f,0.f);
    PARAMETER(0x970,10,0,0.f,0.f);
    PARAMETER(0xA20,11,0,0.f,0.f);
    PARAMETER(0xAD0,4,0,0.f,0.f);
    PARAMETER(0xB80,15,0,0.1f,0.3f);
    PARAMETER(0xC30,16,0,0.1f,0.3f);
    PARAMETER(0xCE0,7,0,0.1f,0.3f);
    PARAMETER(0xD90,8,0,0.1f,0.3f);
    PARAMETER(0xE40,13,0,0.f,0.f);
    PARAMETER(0xEF0,12,0,0.f,0.f);
    PARAMETER(0xFA0,6,0,0.f,0.f);
    PARAMETER(0x1050,0,0,0.f,0.f);
    PARAMETER(0x1100,26,MODE==1?-1:0,0.12f,0.3f);
    PARAMETER(0x11B0,1,MODE==1?-1:0,0.1f,0.3f);
    PARAMETER(0x1260,19,MODE==1?-1:0,0.1f,0.3f);
    PARAMETER(0x1310,MODE==1?20:27,0,0.013f,0.25f);
    PARAMETER(0x13C0,30,MODE==1?-1:0,0.017f,0.21f);
    PARAMETER(0x1470,17,MODE==1?-1:0,0.03f,0.2f);
#ifdef DEADLOCKED_NATIVE
    return DREADZONE_ACTOR_PARAMETERS_COMPLETED;
#endif
}
