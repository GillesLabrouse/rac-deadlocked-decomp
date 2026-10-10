#include "controller_win.h"
#include <cstring>

namespace dlnative {
namespace {
const ControllerWinApi windows_api={XInputGetState,XInputGetCapabilities,XInputSetState};
u8 stick_byte(SHORT axis) { return static_cast<u8>((static_cast<s32>(axis)+32768)/256); }
u8 stick_y_byte(SHORT axis) {
    s32 inverted=-static_cast<s32>(axis);
    if(inverted>32767) inverted=32767;
    if(inverted<-32768) inverted=-32768;
    return static_cast<u8>((inverted+32768)/256);
}
u16 pressed_mask(const XINPUT_GAMEPAD &g) {
    const WORD b=g.wButtons;
    u16 mask=0;
    if(b&XINPUT_GAMEPAD_BACK) mask|=0x0100;
    if(b&XINPUT_GAMEPAD_LEFT_THUMB) mask|=0x0200;
    if(b&XINPUT_GAMEPAD_RIGHT_THUMB) mask|=0x0400;
    if(b&XINPUT_GAMEPAD_START) mask|=0x0800;
    if(b&XINPUT_GAMEPAD_DPAD_UP) mask|=0x1000;
    if(b&XINPUT_GAMEPAD_DPAD_RIGHT) mask|=0x2000;
    if(b&XINPUT_GAMEPAD_DPAD_DOWN) mask|=0x4000;
    if(b&XINPUT_GAMEPAD_DPAD_LEFT) mask|=0x8000;
    if(g.bLeftTrigger>XINPUT_GAMEPAD_TRIGGER_THRESHOLD) mask|=0x0001;
    if(g.bRightTrigger>XINPUT_GAMEPAD_TRIGGER_THRESHOLD) mask|=0x0002;
    if(b&XINPUT_GAMEPAD_LEFT_SHOULDER) mask|=0x0004;
    if(b&XINPUT_GAMEPAD_RIGHT_SHOULDER) mask|=0x0008;
    if(b&XINPUT_GAMEPAD_Y) mask|=0x0010;
    if(b&XINPUT_GAMEPAD_B) mask|=0x0020;
    if(b&XINPUT_GAMEPAD_A) mask|=0x0040;
    if(b&XINPUT_GAMEPAD_X) mask|=0x0080;
    return mask;
}
}
ControllerWin::ControllerWin(bool allow_feedback):ControllerWin(windows_api,allow_feedback) {}
ControllerWin::ControllerWin(const ControllerWinApi &api,bool allow_feedback):api_(api),allow_feedback_(allow_feedback) {}
int ControllerWin::user_index(s32 port,s32 slot) {
    return port>=0 && port<2 && slot>=0 && slot<2 ? slot*2+port : -1;
}
bool ControllerWin::analog_capable(const Slot &slot) {
    const auto &g=slot.sample.capabilities.Gamepad;
    return g.sThumbLX!=0 || g.sThumbLY!=0 || g.sThumbRX!=0 || g.sThumbRY!=0;
}
bool ControllerWin::pressure_capable(const Slot &slot) {
    const auto &g=slot.sample.capabilities.Gamepad;
    return (g.wButtons&0xF3FF)!=0 || g.bLeftTrigger!=0 || g.bRightTrigger!=0;
}
void ControllerWin::encode_packet(const XINPUT_GAMEPAD &g,bool analog,bool pressure,u8 *packet) {
    if(!packet) return;
    std::memset(packet,0,32);
    const u16 mask=pressed_mask(g);
    packet[1]=pressure && analog ? 0x79 : analog ? 0x73 : 0x41;
    packet[2]=static_cast<u8>((mask^0xFFFFU)>>8);
    packet[3]=static_cast<u8>(mask^0xFFFFU);
    packet[4]=analog ? stick_byte(g.sThumbRX) : 128;
    packet[5]=analog ? stick_y_byte(g.sThumbRY) : 128;
    packet[6]=analog ? stick_byte(g.sThumbLX) : 128;
    packet[7]=analog ? stick_y_byte(g.sThumbLY) : 128;
    if(pressure && analog) {
        const u16 digital[10]={0x2000,0x8000,0x1000,0x4000,0x10,0x20,0x40,0x80,4,8};
        for(u32 i=0;i<10;i++) packet[8+i]=(mask&digital[i]) ? 255 : 0;
        packet[18]=g.bLeftTrigger; packet[19]=g.bRightTrigger;
    }
}
void ControllerWin::update_packet(Slot &slot) {
    XINPUT_GAMEPAD gamepad=slot.sample.state.Gamepad;
    const auto &caps=slot.sample.capabilities.Gamepad;
    gamepad.wButtons&=caps.wButtons;
    if(caps.sThumbLX==0) gamepad.sThumbLX=0;
    if(caps.sThumbLY==0) gamepad.sThumbLY=0;
    if(caps.sThumbRX==0) gamepad.sThumbRX=0;
    if(caps.sThumbRY==0) gamepad.sThumbRY=0;
    if(caps.bLeftTrigger==0) gamepad.bLeftTrigger=0;
    if(caps.bRightTrigger==0) gamepad.bRightTrigger=0;
    encode_packet(gamepad,slot.analog,slot.pressure,slot.sample.packet.data());
    slot.sample.pressure_enabled=slot.pressure;
}
ControllerWin::Slot *ControllerWin::refresh(int index) {
    if(index<0 || index>=4) return nullptr;
    Slot &slot=slots_[static_cast<u32>(index)];
    XINPUT_STATE state{};
    XINPUT_CAPABILITIES capabilities{};
    const DWORD state_error=api_.get_state ? api_.get_state(static_cast<DWORD>(index),&state) : ERROR_PROC_NOT_FOUND;
    const DWORD caps_error=api_.get_capabilities ? api_.get_capabilities(static_cast<DWORD>(index),0,&capabilities) : ERROR_PROC_NOT_FOUND;
    const bool ready=state_error==ERROR_SUCCESS && caps_error==ERROR_SUCCESS && capabilities.Type==XINPUT_DEVTYPE_GAMEPAD;
    const bool changed=std::memcmp(&slot.sample.capabilities,&capabilities,sizeof(capabilities))!=0;
    if(!ready || !slot.sample.adapter_ready || changed) {
        slot.analog=false; slot.selected=false; slot.aligned=false; slot.pressure=false; slot.completed_request=false;
        slot.alignment.fill(255);
    }
    const bool initialize_analog=ready && (!slot.sample.adapter_ready || changed);
    slot.sample.state_error=state_error; slot.sample.capabilities_error=caps_error;
    slot.sample.state=state; slot.sample.capabilities=capabilities;
    slot.sample.connected=state_error==ERROR_SUCCESS; slot.sample.adapter_ready=ready;
    if(initialize_analog) slot.analog=analog_capable(slot);
    if(ready) update_packet(slot);
    else {
        slot.sample.packet.fill(0); slot.sample.pressure_enabled=false;
        slot.sample.feedback_error=state_error!=ERROR_SUCCESS ? state_error : caps_error!=ERROR_SUCCESS ? caps_error : ERROR_NOT_SUPPORTED;
    }
    return ready ? &slot : nullptr;
}
ControllerWin::Slot *ControllerWin::device(s32 port,s32 slot) { return refresh(user_index(port,slot)); }
bool ControllerWin::snapshot(u32 index,ControllerWinSnapshot &snapshot) {
    if(index>=4) { snapshot={}; snapshot.state_error=snapshot.capabilities_error=ERROR_BAD_ARGUMENTS; return false; }
    refresh(static_cast<int>(index));
    snapshot=slots_[index].sample;
    return snapshot.adapter_ready;
}
const DreadzonePollPlatform *ControllerWin::platform() const {
    static const DreadzonePollPlatform platform={query_state,read_packet,query_modes,request_mode,
        request_state,select_config,set_alignment,request_pressure,query_pressure,port_hint,feedback,nullptr};
    return &platform;
}
s32 ControllerWin::query_state(void *user,s32 port,s32 slot) {
    return user && static_cast<ControllerWin *>(user)->device(port,slot) ? 6 : 0;
}
s32 ControllerWin::read_packet(void *user,s32 port,s32 slot,u8 *packet) {
    if(!user || !packet) return 0;
    Slot *device=static_cast<ControllerWin *>(user)->device(port,slot);
    if(!device) return 0;
    std::memcpy(packet,device->sample.packet.data(),32);
    return 1;
}
s32 ControllerWin::query_modes(void *user,s32 port,s32 slot,s32 selector,s32 index) {
    if(!user || index!=0 || (selector!=1 && selector!=2)) return 0;
    Slot *device=static_cast<ControllerWin *>(user)->device(port,slot);
    if(!device) return 0;
    if(selector==1) return device->analog ? 7 : 4;
    return analog_capable(*device) ? 7 : 0;
}
s32 ControllerWin::request_mode(void *user,s32 port,s32 slot,s32 mode,s32 lock) {
    if(!user || mode!=1 || lock!=3) return 0;
    Slot *device=static_cast<ControllerWin *>(user)->device(port,slot);
    if(!device || !analog_capable(*device)) return 0;
    device->analog=true; device->completed_request=true; update_packet(*device);
    return 1;
}
s32 ControllerWin::request_state(void *user,s32 port,s32 slot) {
    if(!user) return -1;
    Slot *device=static_cast<ControllerWin *>(user)->device(port,slot);
    return device && device->completed_request ? 0 : -1;
}
s32 ControllerWin::select_config(void *user,s32 port,s32 slot,s32 selection,s32 mode) {
    if(!user || selection!=-1 || mode!=0) return 0;
    Slot *device=static_cast<ControllerWin *>(user)->device(port,slot);
    if(!device || !analog_capable(*device) || device->sample.capabilities.Vibration.wLeftMotorSpeed==0
        || device->sample.capabilities.Vibration.wRightMotorSpeed==0) return 0;
    /* No Sony configuration command exists here: this selects a local adapter
     * configuration only after verifying the device capabilities. Without both
     * motors, state70 falls back to ordinary analog packet reads (state99). */
    device->selected=true; device->completed_request=true;
    return 1;
}
s32 ControllerWin::set_alignment(void *user,s32 port,s32 slot,const u8 *alignment) {
    if(!user || !alignment) return 0;
    Slot *device=static_cast<ControllerWin *>(user)->device(port,slot);
    if(!device || !device->selected) return 0;
    bool roles[2]={false,false};
    for(u32 i=0;i<6;i++) {
        const u8 role=alignment[i];
        if(role==255) continue;
        if(role>1 || roles[role]) return 0;
        roles[role]=true;
        if(role==0 && device->sample.capabilities.Vibration.wRightMotorSpeed==0) return 0;
        if(role==1 && device->sample.capabilities.Vibration.wLeftMotorSpeed==0) return 0;
    }
    std::memcpy(device->alignment.data(),alignment,6);
    device->aligned=true; device->completed_request=true;
    return 1;
}
s32 ControllerWin::request_pressure(void *user,s32 port,s32 slot) {
    if(!user) return 0;
    Slot *device=static_cast<ControllerWin *>(user)->device(port,slot);
    if(!device || !device->selected || !device->aligned || !pressure_capable(*device)) return 0;
    /* XInput has no pressure-mode switch. Enable the declared synthetic
     * digital/analog-trigger channels in our packet adapter, synchronously. */
    device->pressure=true; device->completed_request=true; update_packet(*device);
    return 1;
}
s32 ControllerWin::query_pressure(void *user,s32 port,s32 slot) {
    if(!user) return 0;
    Slot *device=static_cast<ControllerWin *>(user)->device(port,slot);
    return device && device->pressure && pressure_capable(*device) ? 1 : 0;
}
s32 ControllerWin::port_hint(void *user,s32 port) {
    if(!user || port<0 || port>=2) return 0;
    auto *backend=static_cast<ControllerWin *>(user);
    /* Host hint means a usable XInput binding exists on this logical port. */
    const bool first=backend->device(port,0)!=nullptr;
    const bool second=backend->device(port,1)!=nullptr;
    return first || second ? 1 : 0;
}
s32 ControllerWin::feedback(void *user,s32 port,s32 slot,const u8 *bytes) {
    if(!user || !bytes) return 0;
    auto *backend=static_cast<ControllerWin *>(user);
    Slot *device=backend->device(port,slot);
    if(!device) return 0;
    if(!backend->allow_feedback_ || !device->aligned || !backend->api_.set_state) {
        device->sample.feedback_error=ERROR_ACCESS_DENIED; return 0;
    }
    XINPUT_VIBRATION vibration{};
    for(u32 i=0;i<6;i++) {
        if(device->alignment[i]==0) vibration.wRightMotorSpeed=bytes[i]!=0 ? 65535 : 0;
        if(device->alignment[i]==1) vibration.wLeftMotorSpeed=static_cast<WORD>(bytes[i]*257U);
    }
    /* Adapter role0 -> binary high-frequency motor; role1 -> byte-scaled
     * low-frequency motor. XInput's motor ranges are both 0..65535. */
    device->sample.feedback_error=backend->api_.set_state(static_cast<DWORD>(user_index(port,slot)),&vibration);
    return device->sample.feedback_error==ERROR_SUCCESS ? 1 : 0;
}
}
