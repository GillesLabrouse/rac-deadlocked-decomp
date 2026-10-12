#include "graphics_completion_win.h"
#include <wrl/client.h>
#include <chrono>

namespace dlnative {

HRESULT complete_and_reset_graphics(ID3D11DeviceContext* context, DWORD timeout_ms) {
    if (!context || context->GetType() != D3D11_DEVICE_CONTEXT_IMMEDIATE || timeout_ms > 30000)
        return E_INVALIDARG;
    if (!timeout_ms) return HRESULT_FROM_WIN32(ERROR_TIMEOUT);

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    context->GetDevice(&device);
    auto result = device->GetDeviceRemovedReason();
    if (FAILED(result)) return result;
    const D3D11_QUERY_DESC desc{D3D11_QUERY_EVENT, 0};
    Microsoft::WRL::ComPtr<ID3D11Query> completion;
    result = device->CreateQuery(&desc, &completion);
    if (FAILED(result)) return result;
    context->End(completion.Get());
    context->Flush();
    for (;;) {
        if (std::chrono::steady_clock::now() >= deadline)
            return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
        BOOL completed = FALSE;
        result = context->GetData(completion.Get(), &completed, sizeof(completed),
                                  D3D11_ASYNC_GETDATA_DONOTFLUSH);
        if (FAILED(result)) return result;
        if (result == S_OK && completed) {
            context->ClearState();
            return S_OK;
        }
        Sleep(1);
    }
}

}
