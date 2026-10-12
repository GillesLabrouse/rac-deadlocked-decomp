#include "dreadzone_startup_initializer.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("Startup initializer contract failure");
}
}

int main() {
    try {
        // A minimum-size mapping ending at the record boundary, with deliberately
        // byte-unaligned host backing. Untouched bytes retain distinct patterns.
        std::array<std::uint8_t, 0x56> bytes{};
        for (std::size_t i = 0; i < bytes.size(); ++i) bytes[i] = std::uint8_t(i * 17 + 3);
        auto expected = bytes;
        const std::array<unsigned, 9> minus_one_words{{
            0x24, 0x28, 0x34, 0x38, 0x3C, 0x40, 0x44, 0x48, 0x4C}};
        for (unsigned offset : minus_one_words)
            for (unsigned lane = 0; lane < 4; ++lane) expected[1 + offset + lane] = 0xFF;
        for (unsigned offset : {0x18U, 0x20U, 0x30U})
            for (unsigned lane = 0; lane < 4; ++lane) expected[1 + offset + lane] = 0;
        expected[1 + 0x1C] = expected[1 + 0x1D] = 0;
        expected[1 + 0x2C] = expected[1 + 0x50] = 0;
        expected[1 + 0x51] = 1;
        DreadzoneStartupInitializerMemory view{bytes.data() + 1, 0x1000, 0x54};
        require(dreadzone_startup_initializer_006B4078(&view, 0x1000) == 0 && bytes == expected);
        const auto before = bytes;
        require(dreadzone_startup_initializer_006B4078(&view, 0x1001) == -1 && bytes == before);
        view.length = 0x53;
        require(dreadzone_startup_initializer_006B4078(&view, 0x1000) == -1 && bytes == before);
        view.length = 0x54; view.guest_base = 0x7FFFFFAC;
        require(dreadzone_startup_initializer_006B4078(&view, 0x7FFFFFAC) == 0 && bytes == before);
        view.guest_base += 4;
        require(dreadzone_startup_initializer_006B4078(&view, view.guest_base) == -1 && bytes == before);
        // The descriptor cannot be stored inside the writable backing.
        alignas(DreadzoneStartupInitializerMemory) std::array<std::uint8_t, 0x54> alias{};
        view = {alias.data(), 0x1000, alias.size()};
        std::memcpy(alias.data(), &view, sizeof(view));
        const auto alias_before = alias;
        require(dreadzone_startup_initializer_006B4078(
            reinterpret_cast<const DreadzoneStartupInitializerMemory*>(alias.data()), 0x1000) == -1);
        require(alias == alias_before);
        require(dreadzone_startup_initializer_006B4078(nullptr, 0x1000) == -1);
        std::cout << "Startup initializer field preservation and atomic refusal passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
