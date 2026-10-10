#ifndef DEADLOCKED_CONTROLLER_WIN_H
#define DEADLOCKED_CONTROLLER_WIN_H
#ifndef _WIN32
#error This backend requires Windows XInput 1.4.
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <Xinput.h>
#include <array>
extern "C" {
#include "dreadzone_input_poll.h"
}

namespace dlnative {
/* Operating-system seam for deterministic fixtures. Production always uses
 * XInputGetState/GetCapabilities/SetState from XInput 1.4, never Sony RPC.
 * Microsoft API contracts:
 * https://learn.microsoft.com/windows/win32/api/xinput/ns-xinput-xinput_gamepad
 * https://learn.microsoft.com/windows/win32/api/xinput/nf-xinput-xinputgetcapabilities
 * https://learn.microsoft.com/windows/win32/api/xinput/nf-xinput-xinputsetstate
 */
struct ControllerWinApi {
    DWORD (WINAPI *get_state)(DWORD,XINPUT_STATE *);
    DWORD (WINAPI *get_capabilities)(DWORD,DWORD,XINPUT_CAPABILITIES *);
    DWORD (WINAPI *set_state)(DWORD,XINPUT_VIBRATION *);
};
struct ControllerWinSnapshot {
    DWORD state_error=ERROR_DEVICE_NOT_CONNECTED;
    DWORD capabilities_error=ERROR_DEVICE_NOT_CONNECTED;
    DWORD feedback_error=ERROR_ACCESS_DENIED;
    XINPUT_STATE state{};
    XINPUT_CAPABILITIES capabilities{};
    std::array<u8,32> packet{};
    bool connected=false;
    bool adapter_ready=false;
    bool pressure_enabled=false;
};

class ControllerWin {
public:
    explicit ControllerWin(bool allow_feedback=false);
    ControllerWin(const ControllerWinApi &,bool allow_feedback=false);
    const DreadzonePollPlatform *platform() const;
    void *platform_context() { return this; }
    bool snapshot(u32 index,ControllerWinSnapshot &);
    static int user_index(s32 port,s32 slot); /* index=2*slot+port; both 0..1. */
    /* Deliberate host packet adapter for the recovered decoder: active-low
     * buttons, RX/RY/LX/LY bytes, optional 12 pressure bytes. These bytes are
     * not a claim that Windows implements a DS2/SCE transport.
     * Sticks use the entire signed16 range without adding a second deadzone:
     * the recovered decoder already has its own 48-byte deadzone. Y is flipped.
     * A/B/X/Y -> Cross/Circle/Square/Triangle, shoulders -> L1/R1,
     * Back/Start -> Select/Start, LT/RT -> L2/R2 when > Microsoft threshold30.
     * Pressure order: Right/Left/Up/Down, Triangle/Circle/Cross/Square,
     * L1/R1/L2/R2. Digital pressure is explicitly synthesized as 0/255;
     * only L2/R2 retain the independent XInput trigger magnitudes.
     */
    static void encode_packet(const XINPUT_GAMEPAD &,bool analog,bool pressure,u8 *packet32);
private:
    struct Slot {
        ControllerWinSnapshot sample{};
        bool analog=false,selected=false,aligned=false,pressure=false,completed_request=false;
        std::array<u8,6> alignment{{255,255,255,255,255,255}};
    };
    ControllerWinApi api_;
    bool allow_feedback_;
    std::array<Slot,4> slots_{};
    Slot *refresh(int);
    Slot *device(s32,s32);
    static bool analog_capable(const Slot &);
    static bool pressure_capable(const Slot &);
    static void update_packet(Slot &);
    static s32 query_state(void *,s32,s32);
    static s32 read_packet(void *,s32,s32,u8 *);
    static s32 query_modes(void *,s32,s32,s32,s32);
    static s32 request_mode(void *,s32,s32,s32,s32);
    static s32 request_state(void *,s32,s32);
    static s32 select_config(void *,s32,s32,s32,s32);
    static s32 set_alignment(void *,s32,s32,const u8 *);
    static s32 request_pressure(void *,s32,s32);
    static s32 query_pressure(void *,s32,s32);
    static s32 port_hint(void *,s32);
    static s32 feedback(void *,s32,s32,const u8 *);
};
}
#endif
