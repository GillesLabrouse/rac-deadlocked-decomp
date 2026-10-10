#include "scene.h"
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>

static void check(bool condition) {
    if (!condition) throw std::runtime_error("Scene test failed");
}
static void word(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    for (unsigned i = 0; i < 4; i++) bytes.push_back(static_cast<std::uint8_t>(value >> (8 * i)));
}
static void scalar(std::vector<std::uint8_t>& bytes, float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, 4);
    word(bytes, bits);
}
static std::vector<std::uint8_t> triangle() {
    std::vector<std::uint8_t> bytes{'D','L','H','O','S','T','0','1'};
    word(bytes, 1); word(bytes, 3); word(bytes, 1); word(bytes, 0);
    for (float bound : {-1.f,-1.f,0.f,1.f,1.f,0.f}) scalar(bytes, bound);
    for (const auto& p : std::vector<std::array<float,3>>{{-1,-1,0},{1,-1,0},{0,1,0}}) {
        for (float v : p) scalar(bytes, v);
        for (float v : {0.f,0.f,1.f,0.f,0.f,1.f}) scalar(bytes, v);
    }
    word(bytes, 0); word(bytes, 3); word(bytes, 0);
    return bytes;
}
static void rejects(const std::vector<std::uint8_t>& bytes) {
    bool failed = false;
    try { dlnative::decode_scene(bytes); } catch (const std::runtime_error&) { failed = true; }
    check(failed);
}
int main() {
    const auto bytes = triangle();
    auto scene = dlnative::decode_scene(bytes);
    check(scene.vertices.size() == 3 && scene.batches.size() == 1 && scene.pngs.empty());
    check(scene.vertices[1].position[0] == 1 && scene.vertices[0].color[0] == 1);
    for (std::size_t size = 0; size < bytes.size(); size++)
        rejects(std::vector<std::uint8_t>(bytes.begin(), bytes.begin() + size));
    auto damaged = bytes; damaged.push_back(0); rejects(damaged);
    damaged = bytes; damaged[0] = 'X'; rejects(damaged);
    damaged = bytes; damaged[12] = 0xFF; rejects(damaged);
    damaged = bytes; damaged[damaged.size()-4] = 1; rejects(damaged);
    damaged = bytes; damaged[damaged.size()-8] = 6; rejects(damaged);
    damaged = bytes; damaged[damaged.size()-12] = 1; rejects(damaged);
    damaged = bytes; damaged[48] = 0; damaged[49] = 0; damaged[50] = 0x80; damaged[51] = 0x7F; rejects(damaged);
    std::cout << "Scene decoding and malformed geometry rejection passed.\n";
}
