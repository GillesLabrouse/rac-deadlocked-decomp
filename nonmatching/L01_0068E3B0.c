/* cflags: -mno-split-addresses */
/* asflags: --float-immediates */
/* Actual L01 mode setter0068E3B0..0068E5D8,552 bytes. NONMATCHING.
 * Scalar mutations are C. The PS2 normalization uses VU instructions written
 * for the recovered algorithm. Native nonzero modes are explicitly approximate.
 */
#include "common.h"
typedef char ActorFieldWordWidth[sizeof(u32) == 4 ? 1 : -1];
typedef char ActorFieldFloatWidth[sizeof(f32) == 4 ? 1 : -1];
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_fields.h"
#include <math.h>
static u32 actor_field_read(const u8 *record, u32 offset) {
    return (u32)record[offset] | ((u32)record[offset + 1] << 8) |
        ((u32)record[offset + 2] << 16) | ((u32)record[offset + 3] << 24);
}
static void actor_field_write(u8 *record, u32 offset, u32 bits) {
    record[offset] = (u8)bits; record[offset + 1] = (u8)(bits >> 8);
    record[offset + 2] = (u8)(bits >> 16); record[offset + 3] = (u8)(bits >> 24);
}
static u16 actor_field_half(const u8 *record, u32 offset) {
    return (u16)((u16)record[offset] | ((u16)record[offset + 1] << 8));
}
static void actor_field_half_write(u8 *record, u32 offset, u16 bits) {
    record[offset] = (u8)bits; record[offset + 1] = (u8)(bits >> 8);
}
static void actor_field_float_write(u8 *record, u32 offset, f32 value) {
    union { f32 f; u32 bits; } copy;
    copy.f = value;
    actor_field_write(record, offset, copy.bits);
}
#define WORD(o) actor_field_read(record, o)
#define STORE(o,v) actor_field_write(record,o,(u32)(v))
#define HALF(o) actor_field_half(record,o)
#define STORE_HALF(o,v) actor_field_half_write(record,o,(u16)(v))
#define STORE_FLOAT(o,v) actor_field_float_write(record,o,v)
#define MODE_BYTES host->mode_bytes_00171DE0
#define FLAG host->flag_0021E878
int dreadzone_actor_0068E3B0(DreadzoneActorFieldView *host, s32 mode) {
    u8 *record;
    union { f32 f; u32 bits; } component[3], inverse, normalized[3];
    u32 i, magnitude, original_w = 0;
    volatile f32 x2, y2, z2, xy, length_squared;
    if (!host || !host->record || host->record_size < 0x31F0 || (host->record_guest & 15) ||
        host->record_guest > 0xFFFFFFFFU - (host->record_size - 1))
        return DREADZONE_ACTOR_FIELDS_INVALID_STORAGE;
    record = host->record;
    if (mode >= 0 && mode <= 2) {
        if (!MODE_BYTES || !host->mode_count || (s32)WORD(0x2F10) < 0 ||
            WORD(0x2F10) >= host->mode_count || host->mode_count - 1 > 0xFFFFFFFFU - 0x171DE0)
            return DREADZONE_ACTOR_FIELDS_INVALID_STORAGE;
    }
    if (mode != 0) {
        if (!host->client || host->client_size < 0x3A0 || host->client_guest != WORD(0x2EFC) ||
            (host->client_guest & 15) || host->client_guest > 0xFFFFFFFFU - (host->client_size - 1))
            return DREADZONE_ACTOR_FIELDS_NEEDS_CLIENT;
        for (i = 0; i < 3; i++) {
            component[i].bits = actor_field_read(host->client, 0x390 + 4 * i);
            magnitude = component[i].bits & 0x7FFFFFFFU;
            if (magnitude != 0 && (magnitude < 0x30800000U || magnitude > 0x4E800000U))
                return DREADZONE_ACTOR_FIELDS_UNSUPPORTED_FLOAT_DOMAIN;
        }
        original_w = actor_field_read(host->client, 0x39C);
        x2 = component[0].f * component[0].f;
        y2 = component[1].f * component[1].f;
        z2 = component[2].f * component[2].f;
        xy = x2 + y2;
        length_squared = xy + z2;
        /* Architecture facts only, no PCSX2 implementation copied:
         * github.com/PCSX2/pcsx2/blob/master/pcsx2/Interpreter.cpp (BEQ64),
         * VU0.cpp (QMFC2), VUops.cpp (finite VRSQRT saturation at zero). */
        if (length_squared == 0.0f) inverse.bits = 0x7F7FFFFFU;
        else inverse.f = 1.0f / sqrtf(length_squared);
        for (i = 0; i < 3; i++) {
            normalized[i].f = component[i].f * inverse.f;
            if ((normalized[i].bits & 0x7F800000U) == 0x7F800000U)
                return DREADZONE_ACTOR_FIELDS_UNSUPPORTED_FLOAT_DOMAIN;
        }
    }
#else
typedef struct ActorFieldRecord { u8 bytes[0x31F0]; } ActorFieldRecord;
typedef struct ActorFieldVector { f32 x, y, z, w; } __attribute__((aligned(16))) ActorFieldVector;
typedef char ActorFieldRecordExtent[sizeof(ActorFieldRecord) == 0x31F0 ? 1 : -1];
typedef char ActorFieldVectorExtent[sizeof(ActorFieldVector) == 16 ? 1 : -1];
extern u8 D_00171DE0[];
extern s32 D_0021E878[];
#define WORD(o) (*(u32 *)(record + (o)))
#define STORE(o,v) (WORD(o) = (u32)(v))
#define HALF(o) (*(u16 *)(record + (o)))
#define STORE_HALF(o,v) (HALF(o) = (u16)(v))
#define STORE_FLOAT(o,v) (*(f32 *)(record + (o)) = (v))
#define MODE_BYTES D_00171DE0
#define FLAG D_0021E878[0]
void func_L01_0068E3B0(ActorFieldRecord *actor, s32 mode) {
    u8 *record = actor->bytes;
    ActorFieldVector normalized;
    u32 i;
    if (mode != 0) {
        /* vf2.y remains1.f. The retail 64-bit beqz after qmfc2 therefore
         * cannot select its apparent zero fallback. Keep the actual VRSQRT
         * path, including finite Q saturation on zero and untouched W. */
        __asm__ volatile(
            "vmaxw.xyzw $vf2,$vf0,$vf0w\n\t"
            "lqc2 $vf1,0(%1)\n\t"
            "vmul.xyzw $vf3,$vf1,$vf1\n\t"
            "vadday.x ACC,$vf3,$vf3y\n\t"
            "vmaddz.x $vf2,$vf2,$vf3z\n\t"
            "vrsqrt Q,$vf0w,$vf2x\n\t"
            "vwaitq\n\t"
            "vmulq.xyz $vf1,$vf1,Q\n\t"
            "sqc2 $vf1,0(%0)"
            : : "r"(&normalized), "r"((u8 *)WORD(0x2EFC) + 0x390) : "memory");
    }
#endif
    if (mode == 0) MODE_BYTES[WORD(0x2F10)] = 0;
    /* Retail sq instructions clear exactly these two full vectors. */
    for (i = 0; i < 8; i++) STORE(0x1A30 + 4 * i, 0);
    if (mode == 0) {
        STORE_HALF(0x1A50, HALF(0x1A50) | 1);
        STORE_FLOAT(0x1AFC, 1.14999997615814208984375f);
        STORE_FLOAT(0x1A24, 1.5533430576324462890625f);
        STORE_FLOAT(0x1A28, -1.5533430576324462890625f);
        STORE_HALF(0x5F4, 0);
        STORE(0x1AE0, 0);
#ifdef DEADLOCKED_NATIVE
        return DREADZONE_ACTOR_FIELDS_EXACT_MODE_ZERO;
#else
        return;
#endif
    }
    STORE_HALF(0x1A50, HALF(0x1A50) & 0xFFFE);
#ifdef DEADLOCKED_NATIVE
    for (i = 0; i < 3; i++) STORE(0x1960 + 4 * i, normalized[i].bits);
    STORE(0x196C, original_w);
#else
    for (i = 0; i < 4; i++) STORE(0x1960 + 4 * i, ((u32 *)&normalized)[i]);
#endif
    if (mode == 1 || mode == 2) {
        MODE_BYTES[WORD(0x2F10)] = (u8)mode;
        STORE(0x1AE0, mode);
        if (mode == 2) {
            if (FLAG) {
                STORE_FLOAT(0x1A24, 1.2217304706573486328125f);
                STORE_FLOAT(0x1A28, -1.2217304706573486328125f);
            } else {
                STORE_FLOAT(0x1A24, 1.5533430576324462890625f);
                STORE_FLOAT(0x1A28, -1.5533430576324462890625f);
            }
            STORE_FLOAT(0x1AFC, 1.14999997615814208984375f);
        } else {
            /* Retail's two level==1 branches perform these same writes. */
#ifdef DEADLOCKED_NATIVE
            STORE_FLOAT(0x1A30, -6.0f);
            STORE_FLOAT(0x1A38, 0.0f);
#else
            __asm__ volatile(
                "lqc2 $vf1,0(%0)\n\t"
                "qmtc2 %1,$vf2\n\t"
                "vaddx.x $vf1,$vf0,$vf2x\n\t"
                "sqc2 $vf1,0(%0)\n\t"
                "qmtc2 %2,$vf2\n\t"
                "vaddx.z $vf1,$vf0,$vf2x\n\t"
                "sqc2 $vf1,0(%0)"
                : : "r"(record + 0x1A30), "r"(0xC0C00000U), "r"(0) : "memory");
#endif
            STORE_FLOAT(0x1A94, 6.0f);
            STORE_FLOAT(0x1AFC, 2.0f);
            STORE_FLOAT(0x1A24, 1.2217304706573486328125f);
            STORE_FLOAT(0x1A28, -1.2217304706573486328125f);
        }
    }
#ifdef DEADLOCKED_NATIVE
    return DREADZONE_ACTOR_FIELDS_APPROXIMATE_VU;
#endif
}
