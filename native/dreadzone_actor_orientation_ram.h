#ifndef DREADZONE_ACTOR_ORIENTATION_RAM_H
#define DREADZONE_ACTOR_ORIENTATION_RAM_H
#include "common.h"
typedef struct DreadzoneActorOrientationRAMRequest {
    u8 *storage;u32 guest_base,storage_bytes,record_guest,gp_guest;
} DreadzoneActorOrientationRAMRequest;
typedef struct DreadzoneActorOrientationRAMInfo {
    u32 skips,turns,compositions,extractions;
} DreadzoneActorOrientationRAMInfo;
/* Complete selected L01 683310 RAM effects, including real 67CBC0 matrix
 * composition and 596470 Euler extraction, in bounded approximate binary32.
 * State/parity/mode>=3 exits and ordinary mode0 are supported; modes1/2 refuse.
 * Incoming GP-relative factors and absolute clocks are explicit, as in retail.
 * Selected angles<=pi, prior speed<=2pi, clocks/time<=16 in absolute value;
 * force/damping in0..1, maximum in0..16. Selected numeric inputs/intermediates
 * must be normal or signedzero; nonzero selected angle/clock/speed below2^-40 refuses.
 * Real atan coefficient/quadrant tables must be present and unchanged when
 * consumed. Atan ratios in(-1/64,1/64) other thanzero refuse. No shortcuts
 * replace original operation ordering or genuine conditional extraction.
 * Success1 publishes RAM/info atomically. Invalid mapping/unsupported path/
 * numeric domain/allocation failures -1/-2/-3/-4 publish neither.
 * Mapping<=16MiB, below2GiB, outside scratchpad70000000..70003FFF; guest record
 * must be16-byte aligned. Request may borrow RAM; info must be disjoint from request/RAM. Guest addresses
 * remain32-bit. This API emits no GPR/FPR/VF/VI/ACC/Q/SPR/VU/stack outputs.
 * In particular, unconsumed C60 VF28.W is excluded. Use the separate original
 * orientation API for its validated zero-tilt register-transport domain.
 * No physical FPU/VU precision, flags/timing, complete frame or gameplay loop
 * is claimed. Counters describe actual selected RAM work. */
int dreadzone_actor_orientation_ram_00683310(const DreadzoneActorOrientationRAMRequest *,DreadzoneActorOrientationRAMInfo *);
#endif
