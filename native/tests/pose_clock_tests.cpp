#include "pose_clock.h"
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace dlnative;
namespace {
unsigned checks=0;
void require(bool value) { ++checks;if (!value) throw std::runtime_error("Pose clock assertion failed"); }
template<class F> void rejects(F action) {
    try { action(); } catch (const std::runtime_error&) { ++checks;return; }
    throw std::runtime_error("Unsupported pose clock accepted");
}
void put(std::vector<std::uint8_t>& data,std::size_t offset,std::uint32_t value) {
    for (unsigned i=0;i<4;++i) data[offset+i]=std::uint8_t(value>>(i*8));
}
std::uint32_t bits(float value) { std::uint32_t result;std::memcpy(&result,&value,4);return result; }
void wide(std::vector<std::uint8_t>& data,std::size_t offset,std::uint64_t value) {
    put(data,offset,std::uint32_t(value));put(data,offset+4,std::uint32_t(value>>32));
}
std::vector<std::uint8_t> package(const Scene& scene,const PoseSequence& poses) {
    std::vector<std::uint8_t> data(64+0x28+4*4);
    std::memcpy(data.data(),"DLCLK001",8);put(data,8,1);put(data,12,4);put(data,16,0x28);
    put(data,24,bits(60));put(data,28,bits(1));put(data,52,2);
    wide(data,32,pose_scene_fingerprint(scene));wide(data,40,pose_clock_fingerprint(poses));
    data[64+0x10]=2;data[64+0x20]=2;data[64+0x24]=2;
    for (unsigned i=0;i<4;++i) put(data,64+0x28+i*4,bits(float(i)*.5f));
    return data;
}
}
int main(int argc,char** argv) {
    try {
        Scene scene{};scene.vertices={{{0,0,0},{0,0},{1,1,1,1}}};
        PoseSequence poses{4,1,60,{0,0,0,3,0,0},{{0,0,0},{1,0,0},{2,0,0},{3,0,0}}};
        const auto data=package(scene,poses);
        auto clock=decode_pose_clock(data,scene,poses);
        require(clock.sample==0&&clock.completed_ticks==0&&clock.tick_rate==60);
        for (unsigned i=1;i<=120;++i) {
            require(advance_pose_clock(clock,1)==i%4);
            require(clock.completed_ticks==i&&clock.actor[0x4C]==2);
        }
        const auto before=clock;
        require(advance_pose_clock(clock,0)==before.sample);
        rejects([&]{advance_pose_clock(clock,4097);});
        require(clock.actor==before.actor&&clock.completed_ticks==before.completed_ticks);
        for (std::size_t length=0;length<data.size();++length) {
            auto bad=data;bad.resize(length);rejects([&]{decode_pose_clock(bad,scene,poses);});
        }
        for (auto offset:{8U,12U,16U,20U,28U,32U,36U,40U,44U,48U,52U,56U,60U}) {
            auto bad=data;put(bad,offset,0xFFFFFFFFU);rejects([&]{decode_pose_clock(bad,scene,poses);});
        }
        for (float value:{0.f,.5f,241.f,std::numeric_limits<float>::infinity()}) {
            auto bad=data;put(bad,24,bits(value));rejects([&]{decode_pose_clock(bad,scene,poses);});
        }
        auto bad=data;bad.push_back(0);rejects([&]{decode_pose_clock(bad,scene,poses);});
        bad=data;put(bad,64+0x28+4,0);rejects([&]{decode_pose_clock(bad,scene,poses);});
        bad=data;put(bad,64+0x28+4,0x7FC00000);rejects([&]{decode_pose_clock(bad,scene,poses);});
        auto wrong=poses;wrong.positions[2][0]=2.5f;rejects([&]{decode_pose_clock(data,scene,wrong);});
        wrong=poses;wrong.sample_rate=30;rejects([&]{decode_pose_clock(data,scene,wrong);});
        auto wrong_scene=scene;wrong_scene.vertices[0].uv[0]=.5f;
        rejects([&]{decode_pose_clock(data,wrong_scene,poses);});
        // An original duration of four produces frame .25, which is absent
        // from this half-frame table. Refuse without rounding to a nearby pose.
        bad=data;bad[64+0x20]=4;put(bad,52,4);
        clock=decode_pose_clock(bad,scene,poses);const auto unequal=clock;
        rejects([&]{advance_pose_clock(clock,1);});
        require(clock.actor==unequal.actor&&clock.completed_ticks==0);
        // A later unsupported sample rolls back earlier successful ticks too.
        bad=data;bad[64+0x24]=4;
        clock=decode_pose_clock(bad,scene,poses);const auto late=clock;
        rejects([&]{advance_pose_clock(clock,3);});
        require(clock.actor==late.actor&&clock.completed_ticks==0);
        // Explicit clamp flags use the original branch, not file modulo.
        bad=data;put(bad,20,0x10);clock=decode_pose_clock(bad,scene,poses);
        require(advance_pose_clock(clock,120)==2&&clock.actor[0x55]==0x32);
        bad=data;put(bad,24,bits(120));clock=decode_pose_clock(bad,scene,poses);
        require(advance_pose_clock(clock,1)==1); // Tick scheduling does not alter its delta.
        clock.completed_ticks=std::numeric_limits<std::uint64_t>::max();
        rejects([&]{advance_pose_clock(clock,1);});
        // Optional real package validation for the private/original fixture.
        if (argc==4) {
            auto actual_scene=load_scene(argv[1]);auto actual_poses=load_pose_sequence(argv[2],actual_scene);
            auto actual_clock=load_pose_clock(argv[3],actual_scene,actual_poses);
            require(actual_poses.frame_count==60&&actual_clock.definition.size()==152);
            for (unsigned i=1;i<=120;++i) require(advance_pose_clock(actual_clock,1)==i%60);
            require(actual_clock.completed_ticks==120);
        }
        std::cout<<checks<<" pose clock checks passed.\n";return 0;
    } catch (const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }
}
