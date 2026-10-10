#include "pose_sequence.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace dlnative {
namespace {
constexpr std::size_t file_limit = 256 * 1024 * 1024;
std::uint32_t word(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    if (offset > bytes.size() || bytes.size()-offset < 4)
        throw std::runtime_error("Truncated pose sequence header");
    return std::uint32_t(bytes[offset]) | (std::uint32_t(bytes[offset+1])<<8) |
           (std::uint32_t(bytes[offset+2])<<16) | (std::uint32_t(bytes[offset+3])<<24);
}
float scalar(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    const auto bits=word(bytes,offset);
    float value; std::memcpy(&value,&bits,4);
    if (!std::isfinite(value)) throw std::runtime_error("Nonfinite pose sequence value");
    return value;
}
}

std::uint64_t pose_scene_fingerprint(const Scene& scene) {
    // FNV-1a over explicit little-endian binary32 fields, independent of padding.
    // This links vertex ordering and attributes, not a security checksum.
    std::uint64_t hash=14695981039346656037ULL;
    auto add=[&](float value) {
        std::uint32_t bits; std::memcpy(&bits,&value,4);
        for (unsigned shift=0;shift<32;shift+=8) { hash^=(bits>>shift)&255; hash*=1099511628211ULL; }
    };
    for (const auto& vertex:scene.vertices) {
        for (float value:vertex.position) add(value);
        for (float value:vertex.uv) add(value);
        for (float value:vertex.color) add(value);
    }
    return hash;
}

PoseSequence decode_pose_sequence(const std::vector<std::uint8_t>& bytes, const Scene& scene) {
    if (bytes.size()<64 || bytes.size()>file_limit || std::memcmp(bytes.data(),"DLPOSE01",8) || word(bytes,8)!=1)
        throw std::runtime_error("Unsupported pose sequence header");
    PoseSequence poses{};
    poses.frame_count=word(bytes,12); poses.vertex_count=word(bytes,16);
    poses.sample_rate=scalar(bytes,20);
    const auto fingerprint=std::uint64_t(word(bytes,24)) | (std::uint64_t(word(bytes,28))<<32);
    if (!poses.frame_count || poses.frame_count>4096 || !poses.vertex_count ||
        poses.vertex_count!=scene.vertices.size() || poses.sample_rate<1 || poses.sample_rate>240 ||
        fingerprint!=pose_scene_fingerprint(scene) || word(bytes,56) || word(bytes,60))
        throw std::runtime_error("Pose sequence does not match the base scene");
    const std::uint64_t count=std::uint64_t(poses.frame_count)*poses.vertex_count;
    if (count>(file_limit-64)/12 || bytes.size()!=64+count*12)
        throw std::runtime_error("Invalid pose sequence payload size");
    const std::array<float,16> identity{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    if (scene.instances.size()!=1 || scene.instances[0].transform!=identity)
        throw std::runtime_error("Pose sequence requires one identity instance");
    for (unsigned axis=0;axis<6;axis++) poses.bounds[axis]=scalar(bytes,32+axis*4);
    for (unsigned axis=0;axis<3;axis++)
        if (poses.bounds[axis]>poses.bounds[axis+3]) throw std::runtime_error("Inverted pose sequence bounds");
    poses.positions.resize(static_cast<std::size_t>(count));
    std::size_t offset=64;
    for (auto& position:poses.positions) for (unsigned axis=0;axis<3;axis++,offset+=4) {
        position[axis]=scalar(bytes,offset);
        if (position[axis]<poses.bounds[axis] || position[axis]>poses.bounds[axis+3])
            throw std::runtime_error("Pose sequence position exceeds its bounds");
    }
    return poses;
}

PoseSequence load_pose_sequence(const std::filesystem::path& path, const Scene& scene) {
    std::ifstream stream(path,std::ios::binary|std::ios::ate);
    if (!stream) throw std::runtime_error("Cannot open native pose sequence");
    const auto size=stream.tellg();
    if (size<64 || size>static_cast<std::streamoff>(file_limit)) throw std::runtime_error("Invalid pose sequence file size");
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    stream.seekg(0);
    if (!stream.read(reinterpret_cast<char*>(bytes.data()),static_cast<std::streamsize>(bytes.size())))
        throw std::runtime_error("Cannot read native pose sequence");
    return decode_pose_sequence(bytes,scene);
}

std::uint32_t sample_pose_frame(const PoseSequence& poses, double seconds) {
    if (!poses.frame_count || !std::isfinite(poses.sample_rate) || poses.sample_rate<1 || poses.sample_rate>240 ||
        !std::isfinite(seconds) || seconds<0)
        throw std::runtime_error("Invalid pose sequence presentation time");
    // Reduce time first, so even large finite times cannot overflow the product.
    const double period=double(poses.frame_count)/poses.sample_rate;
    const double sample=std::floor(std::fmod(seconds,period)*poses.sample_rate);
    return std::min(static_cast<std::uint32_t>(sample),poses.frame_count-1);
}

void apply_pose_frame(const PoseSequence& poses, std::uint32_t frame, Scene& scene) {
    if (frame>=poses.frame_count || poses.vertex_count!=scene.vertices.size() ||
        poses.positions.size()!=std::uint64_t(poses.frame_count)*poses.vertex_count)
        throw std::runtime_error("Invalid pose sequence frame");
    const auto first=std::size_t(frame)*poses.vertex_count;
    for (std::size_t i=0;i<scene.vertices.size();i++)
        std::copy(poses.positions[first+i].begin(),poses.positions[first+i].end(),scene.vertices[i].position);
    scene.bounds=poses.bounds;
}
}
