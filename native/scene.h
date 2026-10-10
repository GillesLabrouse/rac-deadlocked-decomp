#ifndef DEADLOCKED_NATIVE_SCENE_H
#define DEADLOCKED_NATIVE_SCENE_H

#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace dlnative {

struct Vertex {
    float position[3];
    float uv[2];
    float color[4];
};
static_assert(sizeof(Vertex) == 36, "Scene vertex layout must match the asset bridge");

struct DrawBatch {
    std::uint32_t first;
    std::uint32_t count;
    std::uint32_t texture;
};

struct Scene {
    std::array<float, 6> bounds;
    // Texture zero is the renderer's white fallback; blobs occupy indices 1..N.
    std::vector<std::vector<std::uint8_t>> pngs;
    std::vector<Vertex> vertices;
    std::vector<DrawBatch> batches;
};

Scene decode_scene(const std::vector<std::uint8_t>& bytes);
Scene load_scene(const std::filesystem::path& path);

} // namespace dlnative
#endif
