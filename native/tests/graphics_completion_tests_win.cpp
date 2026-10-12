#include "graphics_completion_win.h"
#include <wrl/client.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <chrono>

using Microsoft::WRL::ComPtr;

static void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

static void checked(HRESULT result, const char* message) {
    require(SUCCEEDED(result), message);
}

static bool bound_to(ID3D11DeviceContext* context, ID3D11RenderTargetView* expected) {
    ComPtr<ID3D11RenderTargetView> view;
    context->OMGetRenderTargets(1, &view, nullptr);
    return view.Get() == expected;
}

int main() {
    try {
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;
        const D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_0;
        checked(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, &level, 1,
                                 D3D11_SDK_VERSION, &device, nullptr, &context), "Create real WARP device");
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = 8; desc.Height = 8; desc.MipLevels = 1; desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count = 1;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET;
        ComPtr<ID3D11Texture2D> surface, staging;
        ComPtr<ID3D11RenderTargetView> target;
        checked(device->CreateTexture2D(&desc, nullptr, &surface), "Create render surface");
        checked(device->CreateRenderTargetView(surface.Get(), nullptr, &target), "Create target view");
        desc.Usage = D3D11_USAGE_STAGING; desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        checked(device->CreateTexture2D(&desc, nullptr, &staging), "Create readback");
        auto* view = target.Get();
        context->OMSetRenderTargets(1, &view, nullptr);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        require(bound_to(context.Get(), view), "Expected initial target binding");

        ComPtr<ID3D11DeviceContext> deferred;
        checked(device->CreateDeferredContext(0, &deferred), "Create deferred context");
        require(dlnative::complete_and_reset_graphics(nullptr, 2000) == E_INVALIDARG, "Reject null context");
        require(dlnative::complete_and_reset_graphics(deferred.Get(), 2000) == E_INVALIDARG, "Reject deferred context");
        require(dlnative::complete_and_reset_graphics(context.Get(), 30001) == E_INVALIDARG, "Reject excessive budget");
        const auto begin = std::chrono::steady_clock::now();
        require(dlnative::complete_and_reset_graphics(context.Get(), 0) == HRESULT_FROM_WIN32(ERROR_TIMEOUT),
                "Zero budget must refuse");
        require(std::chrono::steady_clock::now() - begin < std::chrono::seconds(1), "Zero budget should return promptly");
        require(bound_to(context.Get(), view), "Refusal must retain target binding");
        D3D11_PRIMITIVE_TOPOLOGY topology{};
        context->IAGetPrimitiveTopology(&topology);
        require(topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, "Refusal must retain topology");

        for (int pass = 0; pass < 2; ++pass) {
            context->OMSetRenderTargets(1, &view, nullptr);
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            const float color[] = {pass ? 0.f : 1.f, pass ? 1.f : 0.f, 0.f, 1.f};
            context->ClearRenderTargetView(view, color);
            context->CopyResource(staging.Get(), surface.Get());
            checked(dlnative::complete_and_reset_graphics(context.Get(), 2000), "Complete actual queued clear/copy");
            require(bound_to(context.Get(), nullptr), "Success must clear target binding");
            context->IAGetPrimitiveTopology(&topology);
            require(topology == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED, "Success must clear topology");
            D3D11_MAPPED_SUBRESOURCE mapped{};
            checked(context->Map(staging.Get(), 0, D3D11_MAP_READ, D3D11_MAP_FLAG_DO_NOT_WAIT, &mapped),
                    "Readback must already be ready without another wait");
            bool valid = true;
            for (unsigned row = 0; row < 8; ++row) {
                const auto* pixels = static_cast<const std::uint8_t*>(mapped.pData) + row * mapped.RowPitch;
                for (unsigned col = 0; col < 8; ++col) {
                    const auto* pixel = pixels + col * 4;
                    valid = valid && pixel[0] == (pass ? 0 : 255) && pixel[1] == (pass ? 255 : 0)
                                  && pixel[2] == 0 && pixel[3] == 255;
                }
            }
            context->Unmap(staging.Get(), 0);
            require(valid, "Queued GPU writes must be visible in every readback pixel");
        }
        std::cout << "PASS: real WARP completion/readback twice, reset bindings, bounded refusal and invalid contexts\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
