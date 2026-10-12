#include "dreadzone_startup_pool.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
constexpr std::uint32_t base = 0x220000, end = 0x3B2000;
void require(bool condition) {
    if (!condition) throw std::runtime_error("Startup composition contract failure");
}
void put(std::vector<std::uint8_t>& bytes, std::uint32_t address, std::uint32_t value) {
    const auto offset = std::size_t(address - base) + 1;
    for (unsigned lane = 0; lane < 4; ++lane)
        bytes.at(offset + lane) = std::uint8_t(value >> (8 * lane));
}
std::uint32_t get(const std::vector<std::uint8_t>& bytes, std::uint32_t address) {
    const auto offset = std::size_t(address - base) + 1;
    std::uint32_t result = 0;
    for (unsigned lane = 0; lane < 4; ++lane)
        result |= std::uint32_t(bytes.at(offset + lane)) << (8 * lane);
    return result;
}
std::vector<std::uint8_t> fixture(std::uint32_t count) {
    std::vector<std::uint8_t> bytes(end - base + 2, 0xA5);
    for (std::uint32_t index = 1; index <= 6; ++index) {
        put(bytes, 0x3B0048 + 4 * index, count);
        put(bytes, 0x3B0110 + 4 * index, 16);
    }
    for (std::uint32_t index = 2; index <= 6; ++index)
        put(bytes, 0x3B0090 + 4 * index, 24);
    return bytes;
}
}

int main() {
    try {
        auto bytes = fixture(1);
        DreadzoneStartupPoolMemory view{bytes.data() + 1, base, end - base};
        require(dreadzone_startup_compose_006669D8_ram_effects(&view) == 0);
        require(get(bytes, 0x223E00) == UINT32_MAX && get(bytes, 0x223E04) == UINT32_MAX);
        require(get(bytes, 0x368B78 + 4) == 0 && get(bytes, 0x368B78 + 4 + 0x1C) == 0x389C70);
        require(get(bytes, 0x366138) == 0 && get(bytes, 0x366154) == 0x37F468);
        require(bytes.front() == 0xA5 && bytes.back() == 0xA5);

        // A failure at the final leaf must undo both sentinels and earlier leaves.
        bytes = fixture(1);
        put(bytes, 0x3B00A8, 3);
        view.bytes = bytes.data() + 1;
        const auto before = bytes;
        require(dreadzone_startup_compose_006669D8_ram_effects(&view) == -1 && bytes == before);

        bytes = fixture(0);
        view.bytes = bytes.data() + 1;
        for (std::uint32_t index = 1; index <= 6; ++index)
            put(bytes, 0x3B0110 + 4 * index, 3);
        require(dreadzone_startup_compose_006669D8_ram_effects(&view) == 0);
        require(get(bytes, 0x368B78 + 4) == 0 && get(bytes, 0x368BB0 + 4) == 0x389C70);

        const auto installed = bytes;
        view.length = 0x223E04 - base + 3;
        require(dreadzone_startup_compose_006669D8_ram_effects(&view) == -1 && bytes == installed);
        view.length = end - base;
        auto* alias = bytes.data() + 16;
        std::memcpy(alias, &view, sizeof(view));
        const auto aliased = bytes;
        require(dreadzone_startup_compose_006669D8_ram_effects(
            reinterpret_cast<const DreadzoneStartupPoolMemory*>(alias)) == -1 && bytes == aliased);
        std::cout << "Startup composition success and atomic rollback checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
