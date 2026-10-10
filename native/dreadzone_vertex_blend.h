#ifndef DEADLOCKED_DREADZONE_VERTEX_BLEND_H
#define DEADLOCKED_DREADZONE_VERTEX_BLEND_H
#include "common.h"

enum { DREADZONE_VERTEX_APPROXIMATE = 1, DREADZONE_VERTEX_INVALID = -1,
       DREADZONE_VERTEX_UNSUPPORTED = -2, DREADZONE_VERTEX_CAPACITY = -3 };

typedef struct DreadzoneVertexPacket {
    const u8 *storage;
    u32 bytes;
} DreadzoneVertexPacket;

/* Host diagnostic records, not enlarged guest records. Matrix columns and
 * positions are the original unscaled local skinning intermediates. */
typedef struct DreadzoneBlendedVertex {
    float matrix[16];
    float position[4];
    u32 packet;
    u16 index;
    u16 row;
    u8 duplicate;
    u8 reserved[3];
    u8 original[16];
} DreadzoneBlendedVertex;

typedef struct DreadzoneVertexState {
    u8 matrix_cache[64 * 64];
    u64 matrix_defined;
    DreadzoneBlendedVertex vertex_cache[512];
    u8 vertex_defined[512];
} DreadzoneVertexState;

typedef struct DreadzoneVertexBlend {
    const DreadzoneVertexPacket *packets;
    u32 packet_count;
    const u8 *joint_matrices;
    u32 joint_count;
    DreadzoneVertexState *state;
    DreadzoneBlendedVertex *output;
    u32 output_capacity;
    u32 *output_count;
    void *workspace;
    u32 workspace_bytes;
} DreadzoneVertexBlend;

/* Transactionally prepares the genuine matrix transfer/blend and local
 * position stages consumed by0053C818, for up to256 compact vertex packets.
 * Transfers/cache persist across packets; duplicate IDs use the512-entry
 * vertex cache; source IDs have the actual seven-record delay and epilogue.
 * Blend weights use byte/256, as the original ITOF12 + MULi16 chain specifies.
 * Require at least3 ordinary/blended rows, total<=127, aligned bounded records
 * and epilogue1..6.
 * Matrix slots0..61 are available; reserved VU constant slots62/63 refuse.
 * All output/state/workspace ranges must be disjoint from borrowed inputs
 * and each other. Workspace needs sizeof(state)+capacity*sizeof(output)+15
 * bytes and is temporary scratch, allowed to change on any return.
 * Other outputs remain unchanged on refusal. Numeric inputs/intermediates
 * must be finite normal IEEE binary32 or zero. APPROXIMATE describes only
 * these CPU-visible mathematical stages, not the complete0053C818 routine:
 * no lighting, normal decode, packed VU1 output, world/view scale, animation,
 * physical VU rounding, synchronization or DMA executes. */
int dreadzone_vertex_blend_prepare(const DreadzoneVertexBlend *request);
#endif
