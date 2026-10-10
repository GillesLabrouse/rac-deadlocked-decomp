#include "scene.h"
#include <algorithm>
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
static void replace_scalar(std::vector<std::uint8_t>& bytes, std::size_t offset, float value) {
    std::vector<std::uint8_t> encoded;
    scalar(encoded, value);
    std::copy(encoded.begin(), encoded.end(), bytes.begin()+offset);
}
static std::vector<std::uint8_t> placed_triangles() {
    const auto original = triangle();
    std::vector<std::uint8_t> bytes(original.begin(),original.begin()+24);
    bytes[8]=2;
    word(bytes,2);
    for (float bound : {-3.f,-1.f,0.f,3.f,1.f,0.f}) scalar(bytes,bound);
    bytes.insert(bytes.end(),original.begin()+48,original.end()-12);
    for (float x : {-2.f,2.f}) {
        dlnative::Instance instance;
        instance.transform[12]=x;
        for (float component : instance.transform) scalar(bytes,component);
        for (float component : instance.color) scalar(bytes,component);
    }
    word(bytes,0); word(bytes,3); word(bytes,0); word(bytes,0); word(bytes,2);
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
    check(scene.instances.size()==1 && scene.instances[0].transform[15]==1);
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
    const auto placed=placed_triangles();
    scene=dlnative::decode_scene(placed);
    check(scene.vertices.size()==3 && scene.instances.size()==2 && scene.batches[0].instance_count==2);
    check(scene.instances[0].transform[12]==-2 && scene.instances[1].transform[12]==2);
    for (std::size_t size=0;size<placed.size();size++)
        rejects(std::vector<std::uint8_t>(placed.begin(),placed.begin()+size));
    damaged=placed; replace_scalar(damaged,160+12,1); rejects(damaged);
    damaged=placed; replace_scalar(damaged,160+48,100); rejects(damaged);
    damaged=placed; damaged[damaged.size()-8]=2; rejects(damaged);
    damaged=placed; damaged[damaged.size()-4]=3; rejects(damaged);
    damaged=placed; damaged.push_back(0); rejects(damaged);
    std::cout << "Legacy/instanced scene decoding and malformed geometry rejection passed.\n";
}
