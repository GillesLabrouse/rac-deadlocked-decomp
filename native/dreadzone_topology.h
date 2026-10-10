#ifndef DEADLOCKED_DREADZONE_TOPOLOGY_H
#define DEADLOCKED_DREADZONE_TOPOLOGY_H
#include "dreadzone_vertex_blend.h"

typedef struct DreadzoneTopologyPacket {
    const u8 *vif;
    u32 vif_bytes, first_vertex, vertex_count;
} DreadzoneTopologyPacket;
typedef struct DreadzoneTopologyTexture {
    s32 index;
    u32 defined;
} DreadzoneTopologyTexture;
typedef struct DreadzoneTopologyTriangle {
    u32 output_index[3], packet;
    u16 guest_id[3];
    s16 st[3][2];
    s32 texture;
} DreadzoneTopologyTriangle;
typedef struct DreadzoneTopologyRequest {
    const DreadzoneTopologyPacket *packets;
    u32 packet_count;
    const DreadzoneBlendedVertex *vertices;
    u32 vertex_count;
    DreadzoneTopologyTexture *texture;
    DreadzoneTopologyTriangle *triangles;
    u32 triangle_capacity;
    u32 *triangle_count;
} DreadzoneTopologyRequest;
enum {
    DREADZONE_TOPOLOGY_DIAGNOSTIC = 1,
    DREADZONE_TOPOLOGY_INVALID = -1,
    DREADZONE_TOPOLOGY_UNSUPPORTED = -2,
    DREADZONE_TOPOLOGY_CAPACITY = -3,
    DREADZONE_TOPOLOGY_UNDEFINED_TEXTURE = -4
};
/* Decode original, unrelocated VIF packets. Do not pass relocated TEX0 fields.
 * Exact packet ordinals, persistent guest IDs and signed ST are retained in
 * original GS FIFO order; no glTF winding normalization is performed. Only
 * FE area-cull-disabled headers are supported. The final three prefetched
 * indices do not emit vertices. ADC advances the FIFO without drawing.
 * Inserted texture packets reset the FIFO through their following PRE/PRIM
 * tag. Secret indices are packed at 16-byte stride in texture quad.w fields;
 * texture-index metadata itself remains at 64-byte stride.
 * This is a double-sided topology diagnostic, not a complete renderer: no
 * clipping, actor/world/view transform, illumination, upload or DMA occurs.
 * Inputs must remain stable through both passes. All publication is atomic
 * on refusal; packets/vertices and mutable outputs must not overlap.
 */
int dreadzone_topology_prepare(const DreadzoneTopologyRequest *);
#endif
