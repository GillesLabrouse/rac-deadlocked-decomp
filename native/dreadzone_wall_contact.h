#ifndef DEADLOCKED_DREADZONE_WALL_CONTACT_H
#define DEADLOCKED_DREADZONE_WALL_CONTACT_H
#include "common.h"

typedef struct DreadzoneWallRequest {
    u8 *storage;
    u32 guest_base,storage_bytes;
    u32 start_guest,end_guest,packet_guest,grid_guest,actor_pool_pointer_guest;
    u32 actor_pool_count,grid_block_count,ignored_actor_guest,caller_flags,cache_guest;
    u32 incoming_vf25_27_w[3];
} DreadzoneWallRequest;
typedef struct DreadzoneWallInfo {
    u32 grid_records,terrain_cells,terrain_faces,terrain_quad_faces;
    u32 dynamic_cells,actor_candidates,actors_ignored,actors_filtered,actors_marked;
    u32 write_words,q_bits,q_exception_bits;
} DreadzoneWallInfo;
enum { DREADZONE_WALL_INVALID=-1,DREADZONE_WALL_UNSUPPORTED=-2 };

/* Complete bounded mask2/t0=0 CPU probe0053DC70. a0/a1 name16-byte endpoint
 * quads, a2 is caller_flags, a3 ignored_actor_guest and t0 cache_guest. The
 * original explicitly clears endpoint W; unsaved VF25/26/27 W are caller
 * state. packet.word0 is the actual terrain tree. grid_guest and the actor
 * base global are26FF20/222D14 in L01. Actor indices are u16, stride256.
 *
 * Executes actual ordered grid/tree/packed geometry/triangle/quad terrain,
 * XY deduplication, actor broadphase/class/visitation filters and final packet
 * conversion/publication. Mask2 genuinely skips primitives; eligible model
 * geometry refuses atomically, never becomes a fabricated miss. cache_guest
 * must be0. A reset-empty actor grid is an explicit fixture, not whole-level
 * initialization. No player frame or wall response is executed.
 * The initial tree/leaf read also occurs when integer endpoints coincide;
 * that early return does not decode the fetched block. Finite signed-zero
 * bounds retain the original minimum/maximum word-order choice.
 *
 * One writable nonwrapping guest range below2GiB, disjoint from scratchpad;
 * guest natural alignment, explicit pool/block bounds and genuine preloads
 * are checked. Normal/zero finite ordered host-f32 with saturated Q/I-D is
 * an explicit numerical approximation, not physical VU/DMA validation.
 * Requests are snapshotted and may alias RAM; info is disjoint from both.
 * Refusals preserve RAM/info; real misses keep genuine visitation/generation
 * effects. Exclusive access is required during the snapshot transaction.
 */
int dreadzone_wall_contact(const DreadzoneWallRequest *request,DreadzoneWallInfo *info);
#endif
