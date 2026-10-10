#ifndef DEADLOCKED_NATIVE_POSE_SEQUENCE_H
#define DEADLOCKED_NATIVE_POSE_SEQUENCE_H
#include "scene.h"

namespace dlnative {
// Diagnostic sampled positions, attached to one identity instance. Samples
// are evaluated by recovered native stages before packaging, never interpolated
// by this viewer. Their presentation rate is not a recovered actor clock.
struct PoseSequence {
    std::uint32_t frame_count;
    std::uint32_t vertex_count;
    float sample_rate;
    std::array<float, 6> bounds;
    std::vector<std::array<float, 3>> positions;
};
std::uint64_t pose_scene_fingerprint(const Scene& scene);
PoseSequence decode_pose_sequence(const std::vector<std::uint8_t>& bytes, const Scene& scene);
PoseSequence load_pose_sequence(const std::filesystem::path& path, const Scene& scene);
std::uint32_t sample_pose_frame(const PoseSequence& poses, double seconds);
// The scene must be the scene used when decoding, with only positions changed.
// Invalid dimensions or frame leave it intact; UV/color/batches are retained.
void apply_pose_frame(const PoseSequence& poses, std::uint32_t frame, Scene& scene);
}
#endif
