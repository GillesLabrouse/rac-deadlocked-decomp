#include "dreadzone_callback_table.h"
#include <array>
#include <cstdio>
#include <cstring>

#define CHECK(condition) do { if (!(condition)) { \
    std::fprintf(stderr, "Callback table contract failed at line %d\n", __LINE__); \
    return 1; } } while (0)

int main() {
    std::array<uint8_t, 128> backing;
    backing.fill(0xA5);
    DreadzoneCallbackTableMemory valid{backing.data() + 1, 0x00212148u, 0x4Cu};
    CHECK(dreadzone_callback_table_006C1C00(&valid) == 0);
    CHECK(backing.front() == 0xA5 && backing[0x4D] == 0xA5);
    const auto once = backing;
    CHECK(dreadzone_callback_table_006C1C00(&valid) == 0 && backing == once);

    const DreadzoneCallbackTableMemory invalid[] = {
        {backing.data(), 0x00212148u, 0x4Bu},
        {backing.data(), 0x00212149u, 0x4Cu},
        {backing.data(), 0x00212148u, 0},
        {backing.data(), 0, 0x02000001u},
        {backing.data(), 0, backing.size()},
        {nullptr, 0x00212148u, 0x4Cu}
    };
    for (const auto &view : invalid) {
        CHECK(dreadzone_callback_table_006C1C00(&view) == -1 && backing == once);
    }
    CHECK(dreadzone_callback_table_006C1C00(nullptr) == -1);
    DreadzoneCallbackTableMemory alias{};
    alias.bytes = reinterpret_cast<uint8_t *>(&alias);
    alias.guest_base = 0x00212148u;
    alias.length = sizeof(alias);
    std::array<uint8_t, sizeof(alias)> original;
    std::memcpy(original.data(), &alias, sizeof(alias));
    CHECK(dreadzone_callback_table_006C1C00(&alias) == -1);
    CHECK(std::memcmp(original.data(), &alias, sizeof(alias)) == 0);
    return 0;
}
