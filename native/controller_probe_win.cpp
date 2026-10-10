#include "controller_win.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* Standalone diagnostic bindings for the real recovered decoder/callback.
 * This owns isolated host storage and supplies no player/actor game binding. */
extern "C" {
u8 D_0021DE31[1];
s32 D_0021DDB4_[1],D_0021E8A0[1],D_002208A8,D_002208AC;
}
namespace {
u32 word(const InputRecord5C0 &record,u32 offset) {
    u32 result;
    std::memcpy(&result,reinterpret_cast<const u8 *>(&record)+offset,4);
    return result;
}
void put(InputRecord5C0 &record,u32 offset,u32 value) {
    std::memcpy(reinterpret_cast<u8 *>(&record)+offset,&value,4);
}
void print_packet(const u8 *packet) {
    for(u32 j=0;j<20;j++) std::printf("%02X%s",unsigned(packet[j]),j==19 ? "\n" : " ");
}
struct ProbeSession {
    InputRecord5C0 record{};
    DreadzoneInputContext1E0 context{};
    DreadzonePollContextView view{};
    DreadzonePollContext host{};
    bool bound=false;
};
void initialize_session(ProbeSession &session,dlnative::ControllerWin &backend,u32 index) {
    session.record={}; session.context={}; session.host={};
    auto &record=session.record;
    for(u16 &buttons:session.context.f0) buttons=0xFFFF;
    for(u16 &axes:session.context.f8) axes=0x8080;
    for(u16 &axes:session.context.f10) axes=0x8080;
    record.f54C=static_cast<u8>(index%2);
    reinterpret_cast<u8 *>(&record)[0x558]=static_cast<u8>(index/2);
    const u32 context_address=0x00300000+index*0x1E0;
    put(record,0x56C,0x006B5CB8); put(record,0x570,context_address);
    session.view={context_address,&session.context};
    auto &host=session.host;
    host.pool=&record; host.pool_count=1; host.pool_guest_base=0x001EE600+index*0x5C0;
    host.contexts=&session.view; host.context_count=1; host.primary_0021DDCC=host.pool_guest_base;
    host.platform=backend.platform(); host.platform_context=backend.platform_context();
    session.bound=true;
}
bool poll_one(ProbeSession &session,dlnative::ControllerWin &backend,u32 index) {
    if(!session.bound) initialize_session(session,backend,index);
    auto &record=session.record;
    auto &context=session.context;
    auto &host=session.host;
    for(unsigned attempt=0;attempt<12 && word(record,0x198)!=99;attempt++) {
        if(dreadzone_poll_record(&host,0)!=DREADZONE_POLL_COMPLETE) { session.bound=false; return false; }
    }
    if(word(record,0x198)!=99) {
        std::printf("  Adapter configuration did not complete (state %lu).\n",static_cast<unsigned long>(word(record,0x198)));
        session.bound=false;
        return false;
    }
    const int result=dreadzone_poll_record(&host,0);
    std::printf("  recovered poll result=%d state=%lu held=%08lX pressed=%08lX released=%08lX callback_frames=%ld\n",
        result,static_cast<unsigned long>(word(record,0x198)),static_cast<unsigned long>(record.f1C0),
        static_cast<unsigned long>(record.f1C4),static_cast<unsigned long>(record.f1C8),static_cast<long>(context.f1D8));
    std::printf("  adapter packet: "); print_packet(reinterpret_cast<const u8 *>(&record)+0x574);
    const bool decoded=result==DREADZONE_POLL_COMPLETE && context.f1D8>0 && reinterpret_cast<const u8 *>(&record)[0x551]==0;
    if(!decoded) session.bound=false;
    return decoded;
}
}
int main(int argc,char **argv) {
    unsigned frames=1;
    if(argc==3 && std::strcmp(argv[1],"--frames")==0) {
        char *end=nullptr;
        const unsigned long parsed=std::strtoul(argv[2],&end,10);
        if(!end || *end!=0 || parsed==0 || parsed>60) { std::fprintf(stderr,"Frames must be 1..60.\n"); return 2; }
        frames=static_cast<unsigned>(parsed);
    } else if(argc!=1) {
        std::fprintf(stderr,"Usage: controller_probe_win [--frames 1..60]\n"); return 2;
    }
    dlnative::ControllerWin backend; /* Feedback denied: no SetState calls. */
    std::array<ProbeSession,4> sessions{}; /* Stable per-device history. */
    unsigned found=0,decoded=0;
    std::puts("Windows XInput 1.4 controller diagnostic; no vibration, actor update or gameplay.");
    std::puts("Four bindings: XInput index = 2*slot+port; ports/slots each 0..1.");
    for(unsigned frame=0;frame<frames;frame++) {
        if(frames>1) std::printf("Sample %u/%u\n",frame+1,frames);
        for(u32 index=0;index<4;index++) {
            dlnative::ControllerWinSnapshot snapshot;
            const bool ready=backend.snapshot(index,snapshot);
            std::printf("index=%u port=%u slot=%u state_error=%lu capabilities_error=%lu",
                index,index%2,index/2,static_cast<unsigned long>(snapshot.state_error),static_cast<unsigned long>(snapshot.capabilities_error));
            if(!snapshot.connected) { sessions[index].bound=false; std::puts(" disconnected"); continue; }
            found++;
            const auto &g=snapshot.state.Gamepad;
            std::printf(" packet=%lu buttons=%04X LT=%u RT=%u LX=%d LY=%d RX=%d RY=%d\n",
                static_cast<unsigned long>(snapshot.state.dwPacketNumber),unsigned(g.wButtons),unsigned(g.bLeftTrigger),unsigned(g.bRightTrigger),
                int(g.sThumbLX),int(g.sThumbLY),int(g.sThumbRX),int(g.sThumbRY));
            std::printf("  type=%u subtype=%u flags=%04X adapter=%s\n",unsigned(snapshot.capabilities.Type),unsigned(snapshot.capabilities.SubType),
                unsigned(snapshot.capabilities.Flags),ready ? "ready" : "unavailable");
            if(!ready) sessions[index].bound=false;
            if(ready && poll_one(sessions[index],backend,index)) decoded++;
        }
        if(frame+1<frames) Sleep(1000); /* Empty indices are not polled every frame. */
    }
    std::printf("Connected samples=%u; complete recovered single-record reads=%u; after_pool remains unbound.\n",found,decoded);
    if(found==0) std::puts("No XInput controller detected; attached hardware input has not been validated.");
    return found!=0 && decoded==0 ? 1 : 0;
}
