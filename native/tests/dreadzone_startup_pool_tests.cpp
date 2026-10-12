#include "dreadzone_startup_pool.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
constexpr std::uint32_t base=0x366000, end=0x3B2000, pool=0x37F468;
void require(bool value) { if(!value) throw std::runtime_error("Startup pool contract failure"); }
void put(std::vector<std::uint8_t>& bytes,std::uint32_t address,std::uint32_t value) {
    const auto offset=std::size_t(address-base)+1;
    for(unsigned i=0;i<4;++i) bytes.at(offset+i)=std::uint8_t(value>>(8*i));
}
std::uint32_t get(const std::vector<std::uint8_t>& bytes,std::uint32_t address) {
    std::uint32_t value=0;const auto offset=std::size_t(address-base)+1;
    for(unsigned i=0;i<4;++i) value|=std::uint32_t(bytes.at(offset+i))<<(8*i);
    return value;
}
}
int main() {
    try {
        std::vector<std::uint8_t> bytes(end-base+1,0xA5);
        DreadzoneStartupPoolMemory view{bytes.data()+1,base,bytes.size()-1};
        put(bytes,0x3B0060,512);put(bytes,0x3B00A8,84);
        const auto before=bytes;
        require(dreadzone_startup_pool_00667518_index6(&view)==0);
        require(get(bytes,0x366138)==0 && get(bytes,0x366154)==pool && get(bytes,0x366170)==pool);
        require(get(bytes,pool+0x10)==0 && get(bytes,pool+0x0C)==pool+84);
        const auto last=pool+511*84;
        require(get(bytes,last+0x0C)==0 && get(bytes,last+0x10)==last-84);
        require(get(bytes,pool+0x14)==0x3B15C8 && get(bytes,last+0x14)==0x3B15C8);
        for(std::uint32_t i=0;i<512;++i) {
            const auto record=pool+i*84;
            require(get(bytes,record)==get(before,record));
            require(get(bytes,record+0x18)==get(before,record+0x18));
        }
        const auto installed=bytes;
        require(dreadzone_startup_pool_00667518_index6(&view)==0 && bytes==installed);
        for(const auto stride : {1U,0xFFFFFFFCU}) {
            put(bytes,0x3B00A8,stride);const auto snapshot=bytes;
            require(dreadzone_startup_pool_00667518_index6(&view)==-1 && bytes==snapshot);
        }
        put(bytes,0x3B0060,4096);put(bytes,0x3B00A8,84);
        auto snapshot=bytes;
        require(dreadzone_startup_pool_00667518_index6(&view)==-1 && bytes==snapshot);
        auto short_view=view;short_view.length=0x100;
        require(dreadzone_startup_pool_00667518_index6(&short_view)==-1 && bytes==snapshot);
        require(dreadzone_startup_pool_00667518_index6(nullptr)==-1);
        put(bytes,0x3B0060,0xFFFFFFFF);put(bytes,0x3B00A8,1);
        const auto workspace=get(bytes,pool+0x14);
        put(bytes,0x366138,123);
        require(dreadzone_startup_pool_00667518_index6(&view)==0);
        require(get(bytes,0x366138)==0 && get(bytes,pool+0x14)==workspace);
        // Independent index2 controls: its serialized path is table-only.
        put(bytes,0x3B0050,0);put(bytes,0x3B0098,24);put(bytes,0x366128,123);
        snapshot=bytes;
        require(dreadzone_startup_pool_006670B8_index2(&view)==0);
        require(get(bytes,0x366128)==0 && get(bytes,0x366144)==0x37F420 && get(bytes,0x366160)==0x37F420);
        require(get(bytes,0x37F434)==get(snapshot,0x37F434));
        // Positive count here is a diagnostic contract case, not live state.
        put(bytes,0x3B0050,2);
        require(dreadzone_startup_pool_006670B8_index2(&view)==0);
        require(get(bytes,0x37F434)==0x3B0068 && get(bytes,0x37F430)==0);
        require(get(bytes,0x37F42C)==0x37F438 && get(bytes,0x37F444)==0);
        put(bytes,0x3B0098,1);snapshot=bytes;
        require(dreadzone_startup_pool_006670B8_index2(&view)==-1 && bytes==snapshot);
        // Actual index3 serialized count is zero; preserve its workspace.
        put(bytes,0x3B0054,0);put(bytes,0x3B009C,84);put(bytes,0x36612C,123);
        snapshot=bytes;
        require(dreadzone_startup_pool_006671D0_index3(&view)==0);
        require(get(bytes,0x36612C)==0 && get(bytes,0x366148)==0x37F450 && get(bytes,0x366164)==0x37F450);
        require(get(bytes,0x37F464)==get(snapshot,0x37F464));
        // Diagnostic positive case checks the separately verified node binding.
        put(bytes,0x3B0054,2);
        require(dreadzone_startup_pool_006671D0_index3(&view)==0);
        require(get(bytes,0x37F464)==0x3B1510 && get(bytes,0x37F460)==0);
        require(get(bytes,0x37F45C)==0x37F4A4 && get(bytes,0x37F4B4)==0x37F450);
        require(get(bytes,0x37F4B0)==0);
        put(bytes,0x3B009C,1);snapshot=bytes;
        require(dreadzone_startup_pool_006671D0_index3(&view)==-1 && bytes==snapshot);
        put(bytes,0x3B0058,0);put(bytes,0x3B00A0,80);put(bytes,0x366130,123);
        snapshot=bytes;
        require(dreadzone_startup_pool_006672E8_index4(&view)==0);
        require(get(bytes,0x366130)==0 && get(bytes,0x36614C)==0x37F458 && get(bytes,0x366168)==0x37F458);
        require(get(bytes,0x37F46C)==get(snapshot,0x37F46C));
        put(bytes,0x3B0058,2);
        require(dreadzone_startup_pool_006672E8_index4(&view)==0);
        require(get(bytes,0x37F46C)==0x3B1538 && get(bytes,0x37F468)==0);
        require(get(bytes,0x37F464)==0x37F4A8 && get(bytes,0x37F4B8)==0x37F458);
        require(get(bytes,0x37F4B4)==0);
        put(bytes,0x3B00A0,1);snapshot=bytes;
        require(dreadzone_startup_pool_006672E8_index4(&view)==-1 && bytes==snapshot);
        // Index5 uses independently verified node1588 and stride24 bindings.
        put(bytes,0x3B005C,0);put(bytes,0x3B00A4,24);put(bytes,0x366134,123);
        snapshot=bytes;
        require(dreadzone_startup_pool_00667400_index5(&view)==0);
        require(get(bytes,0x366134)==0 && get(bytes,0x366150)==0x37F460 && get(bytes,0x36616C)==0x37F460);
        require(get(bytes,0x37F474)==get(snapshot,0x37F474));
        put(bytes,0x3B005C,2);
        require(dreadzone_startup_pool_00667400_index5(&view)==0);
        require(get(bytes,0x37F474)==0x3B1588 && get(bytes,0x37F470)==0);
        require(get(bytes,0x37F46C)==0x37F478 && get(bytes,0x37F488)==0x37F460);
        require(get(bytes,0x37F484)==0);
        put(bytes,0x3B00A4,1);snapshot=bytes;
        require(dreadzone_startup_pool_00667400_index5(&view)==-1 && bytes==snapshot);
        std::cout << "Startup pool contract passed\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr << error.what() << '\n';return 1;
    }
}
