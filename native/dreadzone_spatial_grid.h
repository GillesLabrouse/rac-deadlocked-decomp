#ifndef DEADLOCKED_DREADZONE_SPATIAL_GRID_H
#define DEADLOCKED_DREADZONE_SPATIAL_GRID_H
#include "common.h"

enum {
    DREADZONE_SPATIAL_GRID_BYTES=0x4000,
    DREADZONE_SPATIAL_BLOCK_BYTES=0xC000,
    DREADZONE_SPATIAL_BITMAP_BYTES=0xC0,
    DREADZONE_SPATIAL_ACTOR_BYTES=0x100,
    DREADZONE_SPATIAL_BLOCK_COUNT=1536,
    DREADZONE_SPATIAL_COMMITTED=1,
    DREADZONE_SPATIAL_INVALID=-1,
    DREADZONE_SPATIAL_EXHAUSTED=-2,
    DREADZONE_SPATIAL_TRAP=-3,
    DREADZONE_SPATIAL_HOST_MEMORY=-4
};

typedef struct DreadzoneSpatialRequest {
    u8 *actor;
    u32 actor_bytes;
    u8 *grid;
    u32 grid_bytes;
    u8 *blocks;
    u32 block_bytes;
    u8 *bitmap;
    u32 bitmap_bytes;
    u32 actor_guest;
    u32 actor_pool_guest;
    u32 actor_pool_count;
    u32 rectangle;
} DreadzoneSpatialRequest;

/* Complete CPU memory effects of0058DC78 plus0058DBA0/0058DC00 in a bounded
 * domain, including real bitmap allocation, list growth/shrink, removal,
 * overlap retention and packed actor.A4 publication. No matrix, collision,
 * VU, rendering or actor-construction callback is supplied here.
 *
 * Rectangles contain inclusive unsigned coordinates0..63 in byte order
 * [xmin,ymin,xmax,ymax]; any negative signed rectangle is unregistered.
 * Cell capacities are powers of two1..16, allocated in aligned bitmap runs;
 * used actor indices fit the supplied pool (1..65536 actors, stride256).
 * Host backing may be unaligned, but all four regions and the request
 * descriptor must be disjoint and spans must not wrap. Guest actor/pool
 * addresses are numeric32, with valid16-byte alignment and whole actor stride.
 * Actor/base signs must agree, avoiding the original signed SUB overflow.
 *
 * Validation and execution use private copies, then publish together. Every
 * error leaves all supplied backing untouched. TRAP identifies the fully
 * mapped retail missing-membership trap domain; corrupt ownership/bitmap
 * states and unmapped trap delay-slot reads are INVALID. EXHAUSTED identifies a
 * search leaving the real192-byte bitmap, where retail has no failure return.
 * These host refusals are not successful game returns. COMMITTED completes
 * the whole supported spatial operation, not the surrounding VU transform.
 */
int dreadzone_spatial_update(const DreadzoneSpatialRequest *request);
#endif
