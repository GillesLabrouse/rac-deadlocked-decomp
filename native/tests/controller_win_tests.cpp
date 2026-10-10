#include "controller_win.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>

extern "C" {
u8 D_0021DE31[1];
s32 D_0021DDB4_[1],D_0021E8A0[1],D_002208A8,D_002208AC;
}
namespace {
unsigned checks;
void check(bool condition,const char *message) {
    checks++;
    if(!condition) { std::fprintf(stderr,"Windows controller check failed: %s\n",message); std::exit(1); }
}
struct Fixture {
    DWORD state_error[4],caps_error[4],feedback_error=ERROR_SUCCESS;
    XINPUT_STATE states[4]{};
    XINPUT_CAPABILITIES caps[4]{};
    XINPUT_VIBRATION vibration{};
    DWORD feedback_index=0;
    unsigned reads=0,queries=0,feedbacks=0;
} fixture;
DWORD WINAPI get_state(DWORD index,XINPUT_STATE *state) {
    check(index<4 && state,"state API index and storage bounded"); fixture.reads++;
    *state=fixture.states[index]; return fixture.state_error[index];
}
DWORD WINAPI get_caps(DWORD index,DWORD flags,XINPUT_CAPABILITIES *caps) {
    check(index<4 && flags==0 && caps,"capabilities API arguments bounded"); fixture.queries++;
    *caps=fixture.caps[index]; return fixture.caps_error[index];
}
DWORD WINAPI set_state(DWORD index,XINPUT_VIBRATION *vibration) {
    check(index<4 && vibration,"feedback API storage bounded"); fixture.feedbacks++;
    fixture.feedback_index=index; fixture.vibration=*vibration; return fixture.feedback_error;
}
const dlnative::ControllerWinApi api={get_state,get_caps,set_state};
void setup() {
    fixture={};
    for(unsigned i=0;i<4;i++) {
        fixture.state_error[i]=fixture.caps_error[i]=ERROR_DEVICE_NOT_CONNECTED;
        fixture.caps[i].Type=XINPUT_DEVTYPE_GAMEPAD;
        fixture.caps[i].Gamepad.wButtons=0xF3FF;
        fixture.caps[i].Gamepad.bLeftTrigger=fixture.caps[i].Gamepad.bRightTrigger=255;
        fixture.caps[i].Gamepad.sThumbLX=fixture.caps[i].Gamepad.sThumbLY=static_cast<SHORT>(-1);
        fixture.caps[i].Gamepad.sThumbRX=fixture.caps[i].Gamepad.sThumbRY=static_cast<SHORT>(-1);
        fixture.caps[i].Vibration.wLeftMotorSpeed=fixture.caps[i].Vibration.wRightMotorSpeed=65535;
    }
    fixture.state_error[0]=fixture.caps_error[0]=ERROR_SUCCESS;
    fixture.states[0].dwPacketNumber=43;
}
u16 mask(const u8 *packet) { return static_cast<u16>((u16(packet[2])<<8|packet[3])^0xFFFF); }
u32 word(const InputRecord5C0 &record,u32 offset) { u32 result; std::memcpy(&result,reinterpret_cast<const u8 *>(&record)+offset,4); return result; }
void put(InputRecord5C0 &record,u32 offset,u32 value) { std::memcpy(reinterpret_cast<u8 *>(&record)+offset,&value,4); }
void conversion() {
    const WORD buttons[]={XINPUT_GAMEPAD_BACK,XINPUT_GAMEPAD_LEFT_THUMB,XINPUT_GAMEPAD_RIGHT_THUMB,XINPUT_GAMEPAD_START,
        XINPUT_GAMEPAD_DPAD_UP,XINPUT_GAMEPAD_DPAD_RIGHT,XINPUT_GAMEPAD_DPAD_DOWN,XINPUT_GAMEPAD_DPAD_LEFT,
        XINPUT_GAMEPAD_LEFT_SHOULDER,XINPUT_GAMEPAD_RIGHT_SHOULDER,XINPUT_GAMEPAD_Y,XINPUT_GAMEPAD_B,XINPUT_GAMEPAD_A,XINPUT_GAMEPAD_X};
    const u16 expected[]={0x100,0x200,0x400,0x800,0x1000,0x2000,0x4000,0x8000,4,8,0x10,0x20,0x40,0x80};
    u8 guarded[34]; XINPUT_GAMEPAD g{};
    std::memset(guarded,0xA5,sizeof(guarded));
    for(unsigned i=0;i<14;i++) {
        g={}; g.wButtons=buttons[i];
        dlnative::ControllerWin::encode_packet(g,true,true,guarded+1);
        check(mask(guarded+1)==expected[i],"each mapped button has its independent recovered mask bit");
        check(guarded[0]==0xA5 && guarded[33]==0xA5,"packet write is exactly bounded to32");
    }
    g={}; g.wButtons=0x0C00; g.bLeftTrigger=30; g.bRightTrigger=31;
    dlnative::ControllerWin::encode_packet(g,true,true,guarded+1);
    check(mask(guarded+1)==2 && guarded[1+18]==30 && guarded[1+19]==31,"reserved buttons ignored and threshold strictly greater than30 while analog pressures remain raw");
    g={}; g.sThumbLX=SHRT_MIN; g.sThumbLY=SHRT_MAX; g.sThumbRX=SHRT_MAX; g.sThumbRY=SHRT_MIN;
    dlnative::ControllerWin::encode_packet(g,true,false,guarded+1);
    check(guarded[1+4]==255 && guarded[1+5]==255 && guarded[1+6]==0 && guarded[1+7]==0,"stick extremes and Y inversion have correct endpoints");
    g={}; dlnative::ControllerWin::encode_packet(g,true,false,guarded+1);
    check(guarded[1+4]==128 && guarded[1+5]==128 && guarded[1+6]==128 && guarded[1+7]==128,"neutral packet has exact128 center on all four channels");
    g.wButtons=XINPUT_GAMEPAD_A|XINPUT_GAMEPAD_Y|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_RIGHT_SHOULDER;
    g.bLeftTrigger=77; g.bRightTrigger=210;
    dlnative::ControllerWin::encode_packet(g,true,true,guarded+1);
    check(guarded[1+9]==255 && guarded[1+12]==255 && guarded[1+14]==255 && guarded[1+17]==255 && guarded[1+18]==77 && guarded[1+19]==210,
        "declared pressure slots distinguish synthesized digital pressure from actual analog triggers");
    dlnative::ControllerWin::encode_packet(g,false,true,guarded+1);
    check(guarded[1+1]==0x41 && guarded[1+4]==128 && guarded[1+18]==0,"digital adapter never advertises pressure data");
}
void transport_and_configuration() {
    setup(); dlnative::ControllerWin backend(api);
    const auto *p=backend.platform(); void *user=backend.platform_context();
    check(!p->after_pool,"actor/repeat game binding remains absent");
    for(s32 port=-1;port<3;port++) for(s32 slot=-1;slot<3;slot++) {
        const int index=dlnative::ControllerWin::user_index(port,slot);
        check(index==(port>=0 && port<2 && slot>=0 && slot<2 ? slot*2+port : -1),"explicit2x2 binding domain");
    }
    check(p->query_state(user,0,0)==6 && p->query_state(user,1,0)==0,"actual OS return gates connection result");
    u8 bytes[32]; std::memset(bytes,0x5A,32);
    const unsigned reads=fixture.reads;
    check(p->read_packet(user,0,2,bytes)==0 && fixture.reads==reads && bytes[0]==0x5A,"unsupported slots do not query outside0..3 or overwrite packets");
    check(p->request_state(user,0,0)==-1 && p->request_pressure(user,0,0)==0,"configuration completion is not unconditionally synthesized");
    check(p->query_modes(user,0,0,1,0)==7 && p->query_modes(user,0,0,1,1)==0,"only declared mode selectors and index accepted");
    check(p->request_mode(user,0,0,0,3)==0 && p->request_mode(user,0,0,1,3)==1,"valid local analog request depends on device axes");
    check(p->select_config(user,0,0,0,0)==0 && p->select_config(user,0,0,-1,0)==1,"local selector validates recovered arguments and capabilities");
    const u8 alignment[6]={0,1,255,255,255,255}; const u8 invalid[6]={0,0,255,255,255,255};
    check(p->set_alignment(user,0,0,invalid)==0 && p->set_alignment(user,0,0,alignment)==1,"local motor mapping refuses duplicate roles");
    check(p->request_pressure(user,0,0)==1 && p->query_pressure(user,0,0)==1 && p->request_state(user,0,0)==0,"configured local adapter publishes synchronized request and pressure state");
    check(p->read_packet(user,0,0,bytes)==1 && bytes[1]==0x79,"ready pressure adapter produces real state packet");
    const u8 rumble[6]={1,128,250,250,250,250};
    check(p->feedback(user,0,0,rumble)==0 && fixture.feedbacks==0,"default backend never actuates even nonzero feedback");
    fixture.state_error[0]=ERROR_DEVICE_NOT_CONNECTED;
    check(p->query_state(user,0,0)==0 && p->query_pressure(user,0,0)==0,"disconnect clears local configuration");
    fixture.state_error[0]=ERROR_SUCCESS;
    check(p->request_state(user,0,0)==-1 && p->read_packet(user,0,0,bytes)==1 && bytes[1]==0x73,"reconnect requires fresh configuration");
    fixture.caps_error[0]=ERROR_NOT_SUPPORTED;
    check(p->query_state(user,0,0)==0 && p->read_packet(user,0,0,bytes)==0,"missing capabilities never claim configured device");
    dlnative::ControllerWinSnapshot snapshot;
    check(!backend.snapshot(4,snapshot) && snapshot.state_error==ERROR_BAD_ARGUMENTS,"snapshot index boundary is explicit");
}
void feedback_fixture() {
    setup(); dlnative::ControllerWin backend(api,true);
    const auto *p=backend.platform(); void *user=backend.platform_context(); const u8 alignment[6]={0,1,255,255,255,255};
    check(p->select_config(user,0,0,-1,0)==1 && p->set_alignment(user,0,0,alignment)==1,"fixture feedback needs real local configuration");
    const u8 bytes[6]={1,128,250,250,250,250};
    check(p->feedback(user,0,0,bytes)==1 && fixture.feedback_index==0 && fixture.vibration.wLeftMotorSpeed==32896 && fixture.vibration.wRightMotorSpeed==65535,
        "injected OS setter receives scaled big and binary small motors only");
    fixture.feedback_error=ERROR_DEVICE_NOT_CONNECTED;
    check(p->feedback(user,0,0,bytes)==0,"setter failure is propagated");
    fixture.caps[0].Vibration.wLeftMotorSpeed=0;
    check(p->select_config(user,0,0,-1,0)==0 && p->feedback(user,0,0,bytes)==0,"changed/absent motor capabilities invalidate alignment");
}
void recovered_record() {
    setup(); dlnative::ControllerWin backend(api);
    InputRecord5C0 record{}; DreadzoneInputContext1E0 callback{};
    for(u16 &value:callback.f0) value=0xFFFF;
    put(record,0x56C,0x006B5CB8); put(record,0x570,0x00300000);
    DreadzonePollContextView view{0x00300000,&callback};
    DreadzonePollContext host{}; host.pool=&record; host.pool_count=1; host.pool_guest_base=0x001EE600;
    host.contexts=&view; host.context_count=1; host.platform=backend.platform(); host.platform_context=backend.platform_context();
    const u32 states[]={70,71,72,76,77,78,99};
    for(u32 expected:states) {
        check(dreadzone_poll_record(&host,0)==DREADZONE_POLL_COMPLETE && word(record,0x198)==expected,"actual recovered configuration follows capability-backed local adapter");
    }
    fixture.states[0].Gamepad.wButtons=XINPUT_GAMEPAD_A; fixture.states[0].Gamepad.bLeftTrigger=200;
    check(dreadzone_poll_record(&host,0)==0 && record.f1C0==(0x40|1) && record.f1C4==(0x40|1) && callback.f1D8==1,"real poll/callback/decoder consume attached-state fixture");
    check(record.f100[10]==1.0f && record.f100[14]>0.78f && record.f100[14]<0.79f,"real recovered pressure decoding sees digital and trigger magnitudes");
    check(dreadzone_poll_record(&host,0)==0 && record.f1C4==0 && callback.f1D8==2,"held input does not invent a second press");
    fixture.states[0].Gamepad={};
    check(dreadzone_poll_record(&host,0)==0 && record.f1C0==0 && record.f1C8==(0x40|1) && callback.f1D8==3,"real decoder release and callback frame advance");
    check(fixture.feedbacks==0,"complete recovered polling does not actuate default backend");
    setup(); fixture.caps[0].Vibration={}; dlnative::ControllerWin no_motors(api);
    record={}; host.platform=no_motors.platform(); host.platform_context=no_motors.platform_context();
    check(dreadzone_poll_record(&host,0)==0 && word(record,0x198)==70,"analog capability detected without motors");
    check(dreadzone_poll_record(&host,0)==0 && word(record,0x198)==99,"unsupported motor config safely selects analog reads");
    check(dreadzone_poll_record(&host,0)==0 && word(record,0x1DC)==0x73,"fallback packet remains honest8-byte analog mode");
    setup(); fixture.caps[0].Gamepad.wButtons=0x0C00;
    fixture.caps[0].Gamepad.bLeftTrigger=fixture.caps[0].Gamepad.bRightTrigger=0;
    dlnative::ControllerWin reserved_caps(api);
    const auto *reserved=reserved_caps.platform(); void *reserved_user=reserved_caps.platform_context();
    const u8 alignment[6]={0,1,255,255,255,255};
    check(reserved->select_config(reserved_user,0,0,-1,0)==1 && reserved->set_alignment(reserved_user,0,0,alignment)==1,
        "motor configuration can be valid independently of pressure inputs");
    check(reserved->request_pressure(reserved_user,0,0)==0,"reserved capabilities bits never invent supported pressure channels");
}
}
int main() {
    conversion(); transport_and_configuration(); feedback_fixture(); recovered_record();
    std::printf("Windows XInput adapter and recovered single-record fixture: %u checks passed; no hardware vibration.\n",checks);
    return 0;
}
