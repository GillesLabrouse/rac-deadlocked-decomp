#ifndef DEADLOCKED_DREADZONE_STATIC_BODY_CONTACT_H
#define DEADLOCKED_DREADZONE_STATIC_BODY_CONTACT_H
#include "common.h"

typedef struct DreadzoneStaticBodyRequest {
    u8 *storage;
    u32 guest_base,storage_bytes;
    u32 table_guest,center_guest,packet_guest,collect_end_guest;
    float radius;
    u32 incoming_vf25_29_w[5];
} DreadzoneStaticBodyRequest;

typedef struct DreadzoneStaticBodyInfo {
    u32 selected_groups,tested_faces,contacts,write_words;
    u32 q_bits,q_exception_bits;
} DreadzoneStaticBodyInfo;

enum {
    DREADZONE_STATIC_BODY_INVALID=-1,
    DREADZONE_STATIC_BODY_UNSUPPORTED=-2
};

/* Complete CPU algorithm00542280..00542808. table_guest is the relocated
 * live00222BE0 table (count,16-byte records, absolute payload pointers).
 * center_guest names the original16-byte a0 vector; radius is real f12.
 * collect_end_guest=0 executes the packet-publication mode used by67EF00.
 * Otherwise it is the original a1 end pointer:64-byte contacts are written
 * backwards into its preceding4096 bytes, and the actual count is returned.
 * Mode0 returns0 for miss or1 for contact. No-hit packet bytes are preserved.
 * All guest accesses share one mapped backing, so collection/packet/source
 * aliases observe the original planned write order. Refusal is atomic.
 * info must be disjoint from request and storage. Request itself is captured
 * before execution and may overlap storage. Mapped guest addresses are below
 * 2GiB, naturally aligned and disjoint from original scratch70000000..3FFF.
 * Inputs/intermediates support finite IEEE normal/zero values, positive radius,
 * center XYZ in [0,1023], radius*64 <=32767, lower bounds >=-32768,
 * upper bounds <=65535; packed coordinates are expanded as unsigned16.
 * Counts <=1023 and vertex counts1..256 are supported; every used address and
 * original unconditional preload must be mapped. Signed SUB traps are refused.
 * Explicit incoming W lanes are real unsaved VF25..29 state; these are never
 * invented as initialized fields. Arithmetic is ordered host-f32 with modeled
 * Q zero saturation/I-D diagnostics, not physical VU rounding or timing.
 * This performs genuine supplementary static body correction; the primary
 * terrain sphere/capsule probes and actor motion entry remain separate work.
 */
int dreadzone_static_body_contact(const DreadzoneStaticBodyRequest *request,
                                 DreadzoneStaticBodyInfo *info);
#endif
