#include "pose_sequence.h"
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace dlnative;
namespace {
unsigned checks=0;
void require(bool value) { ++checks; if (!value) throw std::runtime_error("Pose sequence assertion failed"); }
void put(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    for (unsigned i=0;i<4;i++) bytes[offset+i]=std::uint8_t(value>>(i*8));
}
void put_float(std::vector<std::uint8_t>& bytes, std::size_t offset, float value) {
    std::uint32_t bits; std::memcpy(&bits,&value,4); put(bytes,offset,bits);
}
template<class F> void rejects(F action) {
    try { action(); } catch (const std::runtime_error&) { ++checks; return; }
    throw std::runtime_error("Invalid pose sequence was accepted");
}
Scene base() {
    Scene scene{};
    scene.bounds={0,0,0,1,0,1};
    scene.vertices={{{0,0,0},{0,0},{1,1,1,1}},{{1,0,0},{1,0},{1,1,1,1}},{{0,0,1},{0,1},{1,1,1,1}}};
    scene.instances.resize(1); scene.batches={{0,3,0,0,1}};
    return scene;
}
std::vector<std::uint8_t> package(const Scene& scene) {
    std::vector<std::uint8_t> bytes(64+2*3*12);
    std::memcpy(bytes.data(),"DLPOSE01",8);
    put(bytes,8,1); put(bytes,12,2); put(bytes,16,3); put_float(bytes,20,2);
    const auto hash=pose_scene_fingerprint(scene);
    put(bytes,24,std::uint32_t(hash)); put(bytes,28,std::uint32_t(hash>>32));
    const float bounds[]={0,0,0,3,0,1};
    for (unsigned i=0;i<6;i++) put_float(bytes,32+i*4,bounds[i]);
    std::size_t offset=64;
    for (unsigned frame=0;frame<2;frame++) for (const auto& vertex:scene.vertices)
        for (unsigned axis=0;axis<3;axis++,offset+=4)
            put_float(bytes,offset,vertex.position[axis]+(axis==0 ? float(frame*2) : 0));
    return bytes;
}
}
int main() {
    try {
        auto scene=base(); const auto bytes=package(scene);
        const auto poses=decode_pose_sequence(bytes,scene);
        require(poses.frame_count==2 && poses.vertex_count==3 && poses.positions.size()==6);
        require(sample_pose_frame(poses,0)==0 && sample_pose_frame(poses,0.499)==0);
        require(sample_pose_frame(poses,0.5)==1 && sample_pose_frame(poses,0.999)==1);
        require(sample_pose_frame(poses,1)==0 && sample_pose_frame(poses,1.5)==1);
        require(sample_pose_frame(poses,std::numeric_limits<double>::max())<2);
        const auto before=scene;
        apply_pose_frame(poses,1,scene);
        require(scene.vertices[0].position[0]==2 && scene.vertices[1].position[0]==3);
        require(scene.vertices[2].position[2]==1 && scene.bounds==poses.bounds);
        require(scene.vertices[1].uv[0]==before.vertices[1].uv[0] && scene.vertices[1].color[0]==1);
        apply_pose_frame(poses,0,scene);
        require(std::memcmp(scene.vertices.data(),before.vertices.data(),scene.vertices.size()*sizeof(Vertex))==0);
        rejects([&]{apply_pose_frame(poses,2,scene);});
        require(std::memcmp(scene.vertices.data(),before.vertices.data(),scene.vertices.size()*sizeof(Vertex))==0);
        rejects([&]{sample_pose_frame(poses,-1);});
        rejects([&]{sample_pose_frame(poses,std::numeric_limits<double>::infinity());});
        rejects([&]{sample_pose_frame(poses,std::numeric_limits<double>::quiet_NaN());});
        for (std::size_t size=0;size<bytes.size();size++) {
            auto truncated=bytes; truncated.resize(size);
            rejects([&]{decode_pose_sequence(truncated,before);});
        }
        for (auto offset:{8U,12U,16U,24U,28U,56U,60U}) {
            auto bad=bytes; put(bad,offset,0xFFFFFFFFU);
            rejects([&]{decode_pose_sequence(bad,before);});
        }
        for (float rate:{0.f,0.5f,241.f,std::numeric_limits<float>::infinity()}) {
            auto bad=bytes; put_float(bad,20,rate); rejects([&]{decode_pose_sequence(bad,before);});
        }
        auto bad=bytes; bad.push_back(0); rejects([&]{decode_pose_sequence(bad,before);});
        bad=bytes; put_float(bad,32,4); rejects([&]{decode_pose_sequence(bad,before);});
        bad=bytes; put_float(bad,64,4); rejects([&]{decode_pose_sequence(bad,before);});
        bad=bytes; put_float(bad,64,std::numeric_limits<float>::quiet_NaN());
        rejects([&]{decode_pose_sequence(bad,before);});
        auto wrong=before; wrong.vertices[0].uv[0]=0.5f;
        rejects([&]{decode_pose_sequence(bytes,wrong);});
        wrong=before; wrong.instances[0].transform[12]=1;
        rejects([&]{decode_pose_sequence(bytes,wrong);});
        wrong=before; wrong.vertices.pop_back();
        rejects([&]{apply_pose_frame(poses,0,wrong);});
        std::cout<<checks<<" pose sequence checks passed.\n";
        return 0;
    } catch (const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
}
