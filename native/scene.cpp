#include "scene.h"

#include <algorithm>
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
    const auto version = input.word();
    if (std::memcmp(magic.data(), "DLHOST01", 8) || (version != 1 && version != 2))
        throw std::runtime_error("Unsupported native scene format");
    const auto vertex_count = input.word();
    const auto batch_count = input.word();
    const auto texture_count = input.word();
    const auto instance_count = version == 2 ? input.word() : 1;
    if (!vertex_count || vertex_count % 3 || vertex_count > 8000000 ||
        !batch_count || batch_count > vertex_count / 3 || texture_count > 4096 ||
        !instance_count || instance_count > 1000000)
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
    const std::uint64_t expected = std::uint64_t(vertex_count) * sizeof(Vertex) +
        std::uint64_t(batch_count) * (version == 2 ? 20 : 12) +
        (version == 2 ? std::uint64_t(instance_count) * sizeof(Instance) : 0);
    if (expected != input.remaining()) throw std::runtime_error("Scene geometry size mismatch");
    scene.vertices.resize(vertex_count);
    for (auto& vertex : scene.vertices) {
        for (auto& p : vertex.position) p = input.scalar();
        for (auto& uv : vertex.uv) uv = input.scalar();
        for (auto& color : vertex.color) color = input.scalar();
        for (unsigned axis = 0; version == 1 && axis < 3; axis++)
            if (vertex.position[axis] < scene.bounds[axis] || vertex.position[axis] > scene.bounds[axis + 3])
                throw std::runtime_error("Vertex exceeds scene bounds");
    }
    scene.instances.resize(instance_count);
    if (version == 2) {
        for (auto& instance : scene.instances) {
            for (auto& component : instance.transform) component = input.scalar();
            for (auto& component : instance.color) component = input.scalar();
            const auto& m = instance.transform;
            if (std::abs(m[3]) > 1e-6f || std::abs(m[7]) > 1e-6f || std::abs(m[11]) > 1e-6f || std::abs(m[15]-1) > 1e-6f)
                throw std::runtime_error("Non-affine scene instance transform");
        }
    }
    std::uint32_t next = 0;
    std::uint64_t submitted_triangles = 0, submitted_instances = 0;
    for (std::uint32_t i = 0; i < batch_count; i++) {
        DrawBatch batch{input.word(), input.word(), input.word()};
        if (version == 2) {
            batch.first_instance = input.word();
            batch.instance_count = input.word();
        }
        if (batch.first != next || !batch.count || batch.count % 3 ||
            batch.count > vertex_count - next || batch.texture > texture_count ||
            batch.first_instance >= instance_count || !batch.instance_count ||
            batch.instance_count > instance_count - batch.first_instance)
            throw std::runtime_error("Invalid scene draw range");
        submitted_triangles += std::uint64_t(batch.count/3)*batch.instance_count;
        submitted_instances += batch.instance_count;
        if (submitted_triangles > 100000000 || submitted_instances > 1000000)
            throw std::runtime_error("Scene draw work exceeds supported limits");
        next += batch.count;
        if (version == 2) {
            std::array<float,6> local;
            for (unsigned axis=0;axis<3;axis++) {
                local[axis]=local[axis+3]=scene.vertices[batch.first].position[axis];
                for (std::uint32_t v=batch.first+1;v<next;v++) {
                    local[axis]=std::min(local[axis],scene.vertices[v].position[axis]);
                    local[axis+3]=std::max(local[axis+3],scene.vertices[v].position[axis]);
                }
            }
            for (std::uint32_t instance_index=batch.first_instance;instance_index<batch.first_instance+batch.instance_count;instance_index++) {
                const auto& m=scene.instances[instance_index].transform;
                for (unsigned corner=0;corner<8;corner++) {
                    const float x=local[(corner&1)?3:0], y=local[(corner&2)?4:1], z=local[(corner&4)?5:2];
                    for (unsigned axis=0;axis<3;axis++) {
                        const float position=x*m[axis]+y*m[4+axis]+z*m[8+axis]+m[12+axis];
                        const float tolerance=1e-4f*std::max({1.f,std::abs(scene.bounds[axis]),std::abs(scene.bounds[axis+3])});
                        if (!std::isfinite(position) || position < scene.bounds[axis]-tolerance || position > scene.bounds[axis+3]+tolerance)
                            throw std::runtime_error("Transformed instance exceeds scene bounds");
                    }
                }
            }
        }
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
