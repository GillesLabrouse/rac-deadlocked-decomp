#include "pose_clock.h"
#include "dreadzone_animation_clock.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace dlnative {
namespace {
constexpr std::uint32_t actor_guest=0x400000, definition_guest=0x500000;
constexpr std::size_t file_limit=64+0x420+4096*4;
std::uint32_t word(const std::uint8_t* p) {
    return std::uint32_t(p[0]) | (std::uint32_t(p[1])<<8) |
           (std::uint32_t(p[2])<<16) | (std::uint32_t(p[3])<<24);
}
void put(std::uint8_t* p, std::uint32_t value) {
    for (unsigned i=0;i<4;++i) p[i]=std::uint8_t(value>>(i*8));
}
float scalar(std::uint32_t bits) { float value;std::memcpy(&value,&bits,4);return value; }
std::uint32_t float_bits(float value) { std::uint32_t bits;std::memcpy(&bits,&value,4);return bits; }
std::uint32_t lookup(const PoseClock& clock) {
    const auto bits=word(clock.actor.data()+0x44);
    const auto found=std::find(clock.frame_labels.begin(),clock.frame_labels.end(),bits);
    if (found==clock.frame_labels.end()) throw std::runtime_error("Original clock result has no exact diagnostic pose");
    return static_cast<std::uint32_t>(found-clock.frame_labels.begin());
}
}

std::uint64_t pose_clock_fingerprint(const PoseSequence& poses) {
    std::uint64_t hash=14695981039346656037ULL;
    auto add=[&](std::uint32_t value) {
        for (unsigned i=0;i<4;++i) { hash^=(value>>(i*8))&255;hash*=1099511628211ULL; }
    };
    add(poses.frame_count);add(poses.vertex_count);add(float_bits(poses.sample_rate));
    for (float value:poses.bounds) add(float_bits(value));
    for (const auto& position:poses.positions) for (float value:position) add(float_bits(value));
    return hash;
}

PoseClock decode_pose_clock(const std::vector<std::uint8_t>& bytes,
                            const Scene& scene, const PoseSequence& poses) {
    if (bytes.size()<64 || bytes.size()>file_limit || std::memcmp(bytes.data(),"DLCLK001",8))
        throw std::runtime_error("Unsupported pose clock header");
    auto get=[&](std::size_t offset) { return word(bytes.data()+offset); };
    const auto labels=get(12), metadata=get(16), flags=get(20);
    const auto scene_hash=std::uint64_t(get(32)) | (std::uint64_t(get(36))<<32);
    const auto poses_hash=std::uint64_t(get(40)) | (std::uint64_t(get(44))<<32);
    const float tick_rate=scalar(get(24));
    if (get(8)!=1 || !labels || labels>4096 || labels!=poses.frame_count || metadata<0x24 || metadata>0x41C ||
        bytes.size()!=64+std::uint64_t(metadata)+std::uint64_t(labels)*4 || flags>255 || (flags&4) ||
        !std::isfinite(tick_rate) || tick_rate<1 || tick_rate>240 || get(28)!=0x3F800000 ||
        get(48)!=0 || get(56)!=0 || get(60)!=0 || scene_hash!=pose_scene_fingerprint(scene) ||
        poses_hash!=pose_clock_fingerprint(poses) || poses.vertex_count!=scene.vertices.size() ||
        poses.positions.size()!=std::uint64_t(poses.frame_count)*poses.vertex_count)
        throw std::runtime_error("Pose clock does not match the bounded primary diagnostic");
    const auto count=bytes[64+0x10];
    if (!count || metadata!=0x20+std::uint32_t(count)*4)
        throw std::runtime_error("Invalid original animation metadata extent");
    PoseClock clock{};
    clock.definition.assign(bytes.begin()+64,bytes.begin()+64+metadata);
    for (std::uint32_t i=0;i<count;++i) {
        const auto duration=std::uint32_t(clock.definition[0x20+i*4]) |
                            (std::uint32_t(clock.definition[0x21+i*4])<<8);
        if (!duration || duration>32767 || (!i && get(52)!=duration))
            throw std::runtime_error("Invalid original animation duration");
    }
    for (std::uint32_t i=0;i<labels;++i) {
        const auto bits=get(64+metadata+i*4);
        const float frame=scalar(bits);
        if (!std::isfinite(frame) || frame<0 || frame>=float(count) ||
            std::find(clock.frame_labels.begin(),clock.frame_labels.end(),bits)!=clock.frame_labels.end())
            throw std::runtime_error("Invalid or duplicate exact pose label");
        clock.frame_labels.push_back(bits);
    }
    put(clock.actor.data()+0x40,definition_guest);
    put(clock.actor.data()+0x44,get(48));put(clock.actor.data()+0x48,get(28));
    clock.actor[0x4C]=std::uint8_t(get(52));clock.actor[0x4D]=std::uint8_t(get(52)>>8);
    clock.actor[0x54]=0;clock.actor[0x55]=std::uint8_t(flags);
    clock.tick_rate=tick_rate;clock.sample=lookup(clock);
    return clock;
}

PoseClock load_pose_clock(const std::filesystem::path& path,
                          const Scene& scene, const PoseSequence& poses) {
    std::ifstream stream(path,std::ios::binary|std::ios::ate);
    if (!stream) throw std::runtime_error("Cannot open native pose clock");
    const auto size=stream.tellg();
    if (size<64 || size>static_cast<std::streamoff>(file_limit)) throw std::runtime_error("Invalid pose clock file size");
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    stream.seekg(0);
    if (!stream.read(reinterpret_cast<char*>(bytes.data()),static_cast<std::streamsize>(bytes.size())))
        throw std::runtime_error("Cannot read native pose clock");
    return decode_pose_clock(bytes,scene,poses);
}

std::uint32_t advance_pose_clock(PoseClock& clock, std::uint32_t ticks) {
    if (ticks>4096 || clock.definition.size()<0x24 || clock.definition.size()>0x41C ||
        !clock.frame_labels.size() || clock.frame_labels.size()>4096 ||
        ticks>std::numeric_limits<std::uint64_t>::max()-clock.completed_ticks)
        throw std::runtime_error("Invalid pose clock advance");
    auto next=clock;
    next.sample=lookup(next);
    DreadzoneAnimationClockRequest request{next.actor.data(),static_cast<std::uint32_t>(next.actor.size()),actor_guest,
        next.definition.data(),static_cast<std::uint32_t>(next.definition.size()),definition_guest};
    for (std::uint32_t i=0;i<ticks;++i) {
        if (dreadzone_animation_clock_advance(&request)!=DREADZONE_ANIMATION_CLOCK_APPROXIMATE)
            throw std::runtime_error("Recovered primary animation clock refused");
        next.sample=lookup(next);
    }
    next.completed_ticks+=ticks;clock=std::move(next);
    return clock.sample;
}
}
