#include "scene.h"

#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace dlnative {
namespace {
class Reader {
    const std::vector<std::uint8_t>& bytes;
    std::size_t cursor = 0;
public:
    explicit Reader(const std::vector<std::uint8_t>& input) : bytes(input) {}
    std::size_t remaining() const { return bytes.size() - cursor; }
    std::uint32_t word() {
        if (remaining() < 4) throw std::runtime_error("Truncated scene word");
        const auto* p = bytes.data() + cursor;
        cursor += 4;
        return std::uint32_t(p[0]) | (std::uint32_t(p[1]) << 8) |
               (std::uint32_t(p[2]) << 16) | (std::uint32_t(p[3]) << 24);
    }
    float scalar() {
        auto value = word();
        float result;
        static_assert(sizeof(result) == sizeof(value));
        std::memcpy(&result, &value, sizeof(result));
        if (!std::isfinite(result)) throw std::runtime_error("Non-finite scene scalar");
        return result;
    }
    std::vector<std::uint8_t> blob(std::size_t count) {
        if (count > remaining()) throw std::runtime_error("Truncated scene blob");
        std::vector<std::uint8_t> result(bytes.begin() + cursor, bytes.begin() + cursor + count);
        cursor += count;
        return result;
    }
};
}

Scene decode_scene(const std::vector<std::uint8_t>& bytes) {
    Reader input(bytes);
    auto magic = input.blob(8);
    if (std::memcmp(magic.data(), "DLHOST01", 8) || input.word() != 1)
        throw std::runtime_error("Unsupported native scene format");
    const auto vertex_count = input.word();
    const auto batch_count = input.word();
    const auto texture_count = input.word();
    if (!vertex_count || vertex_count % 3 || vertex_count > 8000000 ||
        !batch_count || batch_count > vertex_count / 3 || texture_count > 4096)
        throw std::runtime_error("Invalid scene counts");
    Scene scene;
    for (auto& bound : scene.bounds) bound = input.scalar();
    for (unsigned i = 0; i < 3; i++)
        if (scene.bounds[i] > scene.bounds[i + 3]) throw std::runtime_error("Inverted scene bounds");
    for (std::uint32_t i = 0; i < texture_count; i++) {
        const auto length = input.word();
        if (length < 8 || length > 64000000) throw std::runtime_error("Invalid scene PNG size");
        auto png = input.blob(length);
        const std::uint8_t signature[] = {137, 80, 78, 71, 13, 10, 26, 10};
        if (std::memcmp(png.data(), signature, 8)) throw std::runtime_error("Invalid scene PNG signature");
        scene.pngs.push_back(std::move(png));
    }
    const std::uint64_t expected = std::uint64_t(vertex_count) * sizeof(Vertex) + std::uint64_t(batch_count) * 12;
    if (expected != input.remaining()) throw std::runtime_error("Scene geometry size mismatch");
    scene.vertices.resize(vertex_count);
    for (auto& vertex : scene.vertices) {
        for (auto& p : vertex.position) p = input.scalar();
        for (auto& uv : vertex.uv) uv = input.scalar();
        for (auto& color : vertex.color) color = input.scalar();
        for (unsigned axis = 0; axis < 3; axis++)
            if (vertex.position[axis] < scene.bounds[axis] || vertex.position[axis] > scene.bounds[axis + 3])
                throw std::runtime_error("Vertex exceeds scene bounds");
    }
    std::uint32_t next = 0;
    for (std::uint32_t i = 0; i < batch_count; i++) {
        DrawBatch batch{input.word(), input.word(), input.word()};
        if (batch.first != next || !batch.count || batch.count % 3 ||
            batch.count > vertex_count - next || batch.texture > texture_count)
            throw std::runtime_error("Invalid scene draw range");
        next += batch.count;
        scene.batches.push_back(batch);
    }
    if (next != vertex_count) throw std::runtime_error("Scene draw ranges do not cover its vertices");
    return scene;
}

Scene load_scene(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream) throw std::runtime_error("Cannot open native scene");
    const auto size = stream.tellg();
    if (size < 0 || size > 512 * 1024 * 1024) throw std::runtime_error("Invalid native scene file size");
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    stream.seekg(0);
    if (!stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())))
        throw std::runtime_error("Cannot read native scene");
    return decode_scene(bytes);
}
} // namespace dlnative
