#ifndef DEADLOCKED_NATIVE_POSE_CLOCK_H
#define DEADLOCKED_NATIVE_POSE_CLOCK_H
#include "pose_sequence.h"

namespace dlnative {
// Primary-only diagnostic clock. Each tick invokes the recovered 589CE0
// arithmetic, then selects an exactly labelled precomputed native pose.
// tick_rate schedules presentation calls; it is not a recovered PS2 frequency.
struct PoseClock {
    std::array<std::uint8_t, 0x56> actor;
    std::vector<std::uint8_t> definition;
    std::vector<std::uint32_t> frame_labels;
    float tick_rate;
    std::uint32_t sample;
    std::uint64_t completed_ticks;
};
std::uint64_t pose_clock_fingerprint(const PoseSequence& poses);
PoseClock decode_pose_clock(const std::vector<std::uint8_t>& bytes,
                            const Scene& scene, const PoseSequence& poses);
PoseClock load_pose_clock(const std::filesystem::path& path,
                          const Scene& scene, const PoseSequence& poses);
// Bounded transactional advance: a refusal or absent exact label leaves the
// clock unchanged. At most 4096 explicit ticks per call; no host interpolation.
std::uint32_t advance_pose_clock(PoseClock& clock, std::uint32_t ticks);
}
#endif
