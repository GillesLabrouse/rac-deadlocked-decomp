#ifndef DEADLOCKED_FRESH_CLASS_COMPOSITION_H
#define DEADLOCKED_FRESH_CLASS_COMPOSITION_H
#include <stdint.h>
typedef struct DreadzoneFreshClass {
    uint8_t *storage;
    uint32_t guest_base, bytes, gp, actor, object_class, skip_cleanup;
    uint8_t *scratch;
    uint32_t scratch_bytes;
    const uint8_t *vu_data;
    uint32_t vu_data_bytes;
} DreadzoneFreshClass;
/* Complete supported fresh L01 00586B10 RAM/SPR effects: real filler,
 * environmental packing, class/definition defaults, genuine model allocation
 * with optional compaction, initial fixed/variable model update, and final flags.
 * Requires skip_cleanup!=0 or original actor+58==0; cleanup is unsupported.
 * Caller supplies published class tables and relocated model assets in one
 * <=16MiB RAM view below2GiB excluding SPR, GP0022597F, a16-aligned100B actor,
 * full class tables26B590/26B2B0/26DD90/26AE00/26AA40 and environment222C50.
 * Class IDs are0..FFFF, with the original signed-half/full-class comparison.
 * Cached table/definition/auxiliary/environment inputs must be disjoint from
 * actor and every model-pool write footprint, including all live100B actors.
 * Mismatch/zero-joint paths and1..127 joints are supported; reached model paths
 * require valid guest pool state and the initial-model API domain, including
 * aligned animation pointers and bounded current/next variable streams.
 * Explicit16KiB SPR and4KiB inherited VU backing are disjoint from RAM and
 * each other. Request may alias RAM/SPR; it is captured before writes.
 * Success=1 (host binary32 approximation when model code is reached);
 * invalidhost=-1, mapping=-2, unsupportedstate/path=-3, allocationfailure=-4.
 * RAM and SPR publish atomically; VU input stays unchanged. Every refusal
 * leaves all caller bytes unchanged. Original stack/register ABI, physical
 * EE/VU rounding and DMA timing are excluded. This does not publish class
 * tables, allocate an actor, construct a player or earn matching credit. */
int dreadzone_fresh_class_initialize(const DreadzoneFreshClass *request);
#endif
