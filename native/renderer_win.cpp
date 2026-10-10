#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wincodec.h>
#include <wrl/client.h>

#include "scene.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace dlnative;

namespace {
void checked(HRESULT result, const char* action) {
    if (FAILED(result)) {
        std::ostringstream message;
        message << action << " failed (HRESULT 0x" << std::hex << unsigned(result) << ')';
        throw std::runtime_error(message.str());
    }
}

const char* shader_source = R"(
cbuffer Camera : register(b0) { row_major float4x4 view_projection; };
Texture2D image : register(t0);
SamplerState image_sampler : register(s0);
struct Input { float3 position:POSITION; float2 uv:TEXCOORD; float4 color:COLOR; };
struct Output { float4 position:SV_POSITION; float2 uv:TEXCOORD; float4 color:COLOR; };
Output vs_main(Input input) {
    Output output;
    output.position = mul(float4(input.position, 1), view_projection);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}
float4 ps_main(Output input) : SV_TARGET {
    float4 value = image.Sample(image_sampler, input.uv) * input.color;
    clip(value.a - 0.05);
    return float4(value.rgb, 1);
}
)";

struct Camera {
    XMFLOAT3 position;
    float yaw;
    float pitch;
    XMVECTOR forward() const {
        return XMVectorSet(std::cos(yaw)*std::cos(pitch), std::sin(yaw)*std::cos(pitch), std::sin(pitch), 0);
    }
};

struct CaptureStats { std::size_t drawn; std::size_t red; };

Camera frame_camera(const Scene& scene) {
    const auto& b = scene.bounds;
    const float span = std::max({b[3]-b[0], b[4]-b[1], b[5]-b[2], 1.f});
    Camera camera{{(b[0]+b[3])*0.5f, b[1]-span*0.65f, b[5]+span*0.2f}, 0, 0};
    XMFLOAT3 target{(b[0]+b[3])*0.5f, (b[1]+b[4])*0.5f, (b[2]+b[5])*0.5f};
    const float dx = target.x-camera.position.x, dy = target.y-camera.position.y;
    camera.yaw = std::atan2(dy, dx);
    camera.pitch = std::atan2(target.z-camera.position.z, std::hypot(dx, dy));
    return camera;
}

class Renderer {
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swap_chain;
    ComPtr<ID3D11Texture2D> target;
    ComPtr<ID3D11RenderTargetView> target_view;
    ComPtr<ID3D11DepthStencilView> depth_view;
    ComPtr<ID3D11Buffer> geometry;
    ComPtr<ID3D11Buffer> camera_buffer;
    ComPtr<ID3D11VertexShader> vertex_shader;
    ComPtr<ID3D11PixelShader> pixel_shader;
    ComPtr<ID3D11InputLayout> layout;
    ComPtr<ID3D11SamplerState> sampler;
    ComPtr<ID3D11RasterizerState> rasterizer;
    ComPtr<IWICImagingFactory> imaging;
    std::vector<ComPtr<ID3D11ShaderResourceView>> textures;
    std::vector<DrawBatch> batches;
    unsigned width, height;
    bool software;

    ComPtr<ID3DBlob> compile(const char* entry, const char* profile) {
        ComPtr<ID3DBlob> bytecode, errors;
        auto result = D3DCompile(shader_source, std::strlen(shader_source), nullptr, nullptr, nullptr,
                                entry, profile, D3DCOMPILE_ENABLE_STRICTNESS, 0, &bytecode, &errors);
        if (FAILED(result) && errors)
            throw std::runtime_error(std::string(static_cast<char*>(errors->GetBufferPointer()), errors->GetBufferSize()));
        checked(result, "Compile host shader");
        return bytecode;
    }

    ComPtr<ID3D11ShaderResourceView> upload_texture(const std::vector<std::uint8_t>& bytes) {
        unsigned w = 1, h = 1;
        std::vector<std::uint8_t> pixels{255,255,255,255};
        if (!bytes.empty()) {
            ComPtr<IWICStream> stream;
            ComPtr<IWICBitmapDecoder> decoder;
            ComPtr<IWICBitmapFrameDecode> frame;
            ComPtr<IWICFormatConverter> converter;
            checked(imaging->CreateStream(&stream), "Create PNG input stream");
            checked(stream->InitializeFromMemory(const_cast<BYTE*>(bytes.data()), static_cast<DWORD>(bytes.size())), "Read PNG bytes");
            checked(imaging->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnLoad, &decoder), "Decode PNG");
            checked(decoder->GetFrame(0, &frame), "Read PNG frame");
            checked(frame->GetSize(&w, &h), "Read PNG dimensions");
            if (!w || !h || w > 8192 || h > 8192 || std::uint64_t(w)*h > 16777216)
                throw std::runtime_error("Unsupported scene texture dimensions");
            checked(imaging->CreateFormatConverter(&converter), "Create PNG converter");
            checked(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone,
                                          nullptr, 0, WICBitmapPaletteTypeCustom), "Convert PNG pixels");
            pixels.resize(std::size_t(w)*h*4);
            checked(converter->CopyPixels(nullptr, w*4, static_cast<UINT>(pixels.size()), pixels.data()), "Copy PNG pixels");
        }
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = w; desc.Height = h; desc.MipLevels = 1; desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_IMMUTABLE; desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA data{pixels.data(), w*4, 0};
        ComPtr<ID3D11Texture2D> texture;
        ComPtr<ID3D11ShaderResourceView> view;
        checked(device->CreateTexture2D(&desc, &data, &texture), "Upload scene texture");
        checked(device->CreateShaderResourceView(texture.Get(), nullptr, &view), "Create scene texture view");
        return view;
    }

public:
    Renderer(const Scene& scene, unsigned w, unsigned h, bool use_software, HWND window=nullptr)
        : batches(scene.batches), width(w), height(h), software(use_software) {
        checked(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                 IID_PPV_ARGS(&imaging)), "Create image codec factory");
        const D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_0;
        auto create = [&](D3D_DRIVER_TYPE driver) {
            return D3D11CreateDevice(nullptr, driver, nullptr, 0, &level, 1, D3D11_SDK_VERSION,
                                    &device, nullptr, &context);
        };
        auto result = create(software ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE);
        if (FAILED(result) && !software) {
            device.Reset(); context.Reset(); software = true;
            result = create(D3D_DRIVER_TYPE_WARP);
        }
        checked(result, "Create Direct3D 11 device");
        if (window) {
            ComPtr<IDXGIDevice> dxgi_device;
            ComPtr<IDXGIAdapter> adapter;
            ComPtr<IDXGIFactory> factory;
            checked(device.As(&dxgi_device), "Find graphics interface");
            checked(dxgi_device->GetAdapter(&adapter), "Find graphics adapter");
            checked(adapter->GetParent(IID_PPV_ARGS(&factory)), "Find graphics factory");
            DXGI_SWAP_CHAIN_DESC swap{};
            swap.BufferDesc.Width = width; swap.BufferDesc.Height = height;
            swap.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swap.SampleDesc.Count = 1; swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swap.BufferCount = 1; swap.OutputWindow = window; swap.Windowed = TRUE;
            swap.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            checked(factory->CreateSwapChain(device.Get(), &swap, &swap_chain), "Create scene window surface");
            checked(swap_chain->GetBuffer(0, IID_PPV_ARGS(&target)), "Get scene window surface");
        } else {
            D3D11_TEXTURE2D_DESC desc{};
            desc.Width=width; desc.Height=height; desc.MipLevels=1; desc.ArraySize=1;
            desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count=1;
            desc.BindFlags=D3D11_BIND_RENDER_TARGET;
            checked(device->CreateTexture2D(&desc, nullptr, &target), "Create capture surface");
        }
        checked(device->CreateRenderTargetView(target.Get(), nullptr, &target_view), "Create capture target");
        D3D11_TEXTURE2D_DESC depth{};
        depth.Width=width; depth.Height=height; depth.MipLevels=1; depth.ArraySize=1;
        depth.Format=DXGI_FORMAT_D32_FLOAT; depth.SampleDesc.Count=1; depth.BindFlags=D3D11_BIND_DEPTH_STENCIL;
        ComPtr<ID3D11Texture2D> depth_texture;
        checked(device->CreateTexture2D(&depth, nullptr, &depth_texture), "Create scene depth buffer");
        checked(device->CreateDepthStencilView(depth_texture.Get(), nullptr, &depth_view), "Create scene depth view");
        auto vs = compile("vs_main", "vs_5_0"), ps = compile("ps_main", "ps_5_0");
        checked(device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &vertex_shader), "Create vertex shader");
        checked(device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &pixel_shader), "Create pixel shader");
        const D3D11_INPUT_ELEMENT_DESC elements[] = {
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA,0}
        };
        checked(device->CreateInputLayout(elements, 3, vs->GetBufferPointer(), vs->GetBufferSize(), &layout), "Create scene vertex layout");
        D3D11_BUFFER_DESC buffer{};
        buffer.ByteWidth=static_cast<UINT>(scene.vertices.size()*sizeof(Vertex));
        buffer.Usage=D3D11_USAGE_IMMUTABLE; buffer.BindFlags=D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA data{scene.vertices.data(), 0, 0};
        checked(device->CreateBuffer(&buffer, &data, &geometry), "Upload scene vertices");
        buffer.ByteWidth=sizeof(XMFLOAT4X4); buffer.Usage=D3D11_USAGE_DEFAULT; buffer.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
        checked(device->CreateBuffer(&buffer, nullptr, &camera_buffer), "Create scene camera buffer");
        D3D11_SAMPLER_DESC sampling{};
        sampling.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampling.AddressU=sampling.AddressV=sampling.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;
        sampling.MaxLOD=D3D11_FLOAT32_MAX;
        sampling.ComparisonFunc=D3D11_COMPARISON_ALWAYS; sampling.MaxAnisotropy=1;
        checked(device->CreateSamplerState(&sampling, &sampler), "Create texture sampling state");
        D3D11_RASTERIZER_DESC raster{};
        raster.FillMode=D3D11_FILL_SOLID; raster.CullMode=D3D11_CULL_NONE; raster.DepthClipEnable=TRUE;
        checked(device->CreateRasterizerState(&raster, &rasterizer), "Create rasterization state");
        textures.push_back(upload_texture({}));
        for (const auto& png : scene.pngs) textures.push_back(upload_texture(png));
    }

    void render(const Camera& camera) {
        const float background[] = {0.03f,0.045f,0.08f,1};
        context->ClearRenderTargetView(target_view.Get(), background);
        context->ClearDepthStencilView(depth_view.Get(), D3D11_CLEAR_DEPTH, 1, 0);
        auto* view = target_view.Get();
        context->OMSetRenderTargets(1, &view, depth_view.Get());
        D3D11_VIEWPORT viewport{0,0,float(width),float(height),0,1};
        context->RSSetViewports(1, &viewport); context->RSSetState(rasterizer.Get());
        XMFLOAT4X4 transform;
        const auto view_matrix = XMMatrixLookToRH(XMLoadFloat3(&camera.position), camera.forward(), XMVectorSet(0,0,1,0));
        const auto projection = XMMatrixPerspectiveFovRH(XM_PIDIV4, float(width)/height, 0.1f, 20000.f);
        XMStoreFloat4x4(&transform, view_matrix*projection);
        context->UpdateSubresource(camera_buffer.Get(), 0, nullptr, &transform, 0, 0);
        auto* constant = camera_buffer.Get(); context->VSSetConstantBuffers(0,1,&constant);
        context->IASetInputLayout(layout.Get());
        auto* vertices = geometry.Get(); UINT stride=sizeof(Vertex), offset=0;
        context->IASetVertexBuffers(0,1,&vertices,&stride,&offset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(vertex_shader.Get(),nullptr,0);
        context->PSSetShader(pixel_shader.Get(),nullptr,0);
        auto* sampling = sampler.Get(); context->PSSetSamplers(0,1,&sampling);
        for (const auto& batch : batches) {
            auto* texture = textures[batch.texture].Get();
            context->PSSetShaderResources(0,1,&texture);
            context->Draw(batch.count,batch.first);
        }
        if (swap_chain) checked(swap_chain->Present(1,0), "Present scene frame");
    }

    CaptureStats capture(const std::filesystem::path& path) {
        D3D11_TEXTURE2D_DESC desc;
        target->GetDesc(&desc); desc.Usage=D3D11_USAGE_STAGING; desc.BindFlags=0;
        desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ; desc.MiscFlags=0;
        ComPtr<ID3D11Texture2D> staging;
        checked(device->CreateTexture2D(&desc,nullptr,&staging), "Create capture readback");
        context->CopyResource(staging.Get(),target.Get());
        D3D11_MAPPED_SUBRESOURCE mapped;
        checked(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped), "Read rendered frame");
        std::vector<BYTE> pixels(std::size_t(width)*height*4);
        for (unsigned row=0;row<height;row++)
            std::memcpy(pixels.data()+std::size_t(row)*width*4,
                        static_cast<BYTE*>(mapped.pData)+std::size_t(row)*mapped.RowPitch,width*4);
        context->Unmap(staging.Get(),0);
        CaptureStats stats{};
        for (std::size_t i=0;i<pixels.size();i+=4) {
            if (std::abs(int(pixels[i])-8)>1 || std::abs(int(pixels[i+1])-11)>1 || std::abs(int(pixels[i+2])-20)>1)
                stats.drawn++;
            if (pixels[i]>200 && pixels[i+1]<10 && pixels[i+2]<10) stats.red++;
        }
        ComPtr<IWICStream> stream;
        ComPtr<IWICBitmapEncoder> encoder;
        ComPtr<IWICBitmapFrameEncode> frame;
        if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
        checked(imaging->CreateStream(&stream), "Create capture output stream");
        checked(stream->InitializeFromFilename(path.c_str(),GENERIC_WRITE), "Open capture output");
        checked(imaging->CreateEncoder(GUID_ContainerFormatPng,nullptr,&encoder), "Create PNG capture encoder");
        checked(encoder->Initialize(stream.Get(),WICBitmapEncoderNoCache), "Initialize PNG capture encoder");
        checked(encoder->CreateNewFrame(&frame,nullptr), "Create PNG capture frame");
        checked(frame->Initialize(nullptr), "Initialize PNG capture frame");
        checked(frame->SetSize(width,height), "Set PNG capture dimensions");
        WICPixelFormatGUID format=GUID_WICPixelFormat32bppBGRA;
        checked(frame->SetPixelFormat(&format), "Set PNG capture format");
        if (format!=GUID_WICPixelFormat32bppBGRA) throw std::runtime_error("PNG encoder changed pixel format");
        for (std::size_t i=0;i<pixels.size();i+=4) std::swap(pixels[i],pixels[i+2]);
        checked(frame->WritePixels(height,width*4,static_cast<UINT>(pixels.size()),pixels.data()), "Encode rendered pixels");
        checked(frame->Commit(), "Commit capture frame"); checked(encoder->Commit(), "Commit capture PNG");
        return stats;
    }
    bool uses_software() const { return software; }
};

LRESULT CALLBACK window_events(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message==WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcW(window,message,wparam,lparam);
}

Scene test_triangle() {
    Scene scene;
    scene.bounds={-1,0,-1,1,0,1};
    scene.vertices={{{-1,0,-1},{0,0},{1,0,0,1}},{{1,0,-1},{0,0},{1,0,0,1}},{{0,0,1},{0,0},{1,0,0,1}}};
    scene.batches={{0,3,0}};
    return scene;
}
}

int wmain(int argc,wchar_t** argv) {
    try {
        std::filesystem::path input, output;
        bool software=false, self_test=false, explicit_camera=false;
        unsigned width=1280,height=720;
        Camera camera{};
        for (int i=1;i<argc;i++) {
            const std::wstring arg=argv[i];
            if (arg==L"--capture" && i+1<argc) output=argv[++i];
            else if (arg==L"--software") software=true;
            else if (arg==L"--self-test") self_test=true;
            else if (arg==L"--camera" && i+5<argc) {
                camera.position.x=std::stof(argv[++i]); camera.position.y=std::stof(argv[++i]); camera.position.z=std::stof(argv[++i]);
                camera.yaw=std::stof(argv[++i]); camera.pitch=std::stof(argv[++i]); explicit_camera=true;
            } else if (arg==L"--help") {
                std::cout << "Usage: deadlocked_native_scene_viewer scene.dlscene [--capture frame.png] [--software]\n"
                             "       [--camera x y z yaw_radians pitch_radians]\n"
                             "Controls: WASD move, Q/E vertical, arrows look, Shift faster, Esc exit.\n"
                             "This renders extracted terrain; game logic and campaign are not implemented.\n";
                return 0;
            } else if (!arg.empty() && arg[0]!=L'-' && input.empty()) input=arg;
            else throw std::runtime_error("Invalid command line; use --help");
        }
        if (!self_test && input.empty()) throw std::runtime_error("Supply a prepared scene file; use --help");
        if (self_test && output.empty()) throw std::runtime_error("Render self-test requires --capture");
        auto scene=self_test ? test_triangle() : load_scene(input);
        if (!explicit_camera) camera=self_test ? Camera{{0,-3,0},XM_PIDIV2,0} : frame_camera(scene);
        if (!std::isfinite(camera.position.x) || !std::isfinite(camera.position.y) || !std::isfinite(camera.position.z) ||
            !std::isfinite(camera.yaw) || !std::isfinite(camera.pitch) || std::abs(camera.pitch)>=XM_PIDIV2)
            throw std::runtime_error("Camera must be finite with pitch between -pi/2 and pi/2");
        checked(CoInitializeEx(nullptr,COINIT_MULTITHREADED), "Initialize image codecs");
        struct ComScope { ~ComScope() { CoUninitialize(); } } com_scope;
        HWND window=nullptr;
        if (output.empty()) {
            WNDCLASSW cls{}; cls.lpfnWndProc=window_events; cls.hInstance=GetModuleHandleW(nullptr);
            cls.lpszClassName=L"DeadlockedNativeScene"; cls.hCursor=LoadCursor(nullptr,IDC_ARROW);
            if (!RegisterClassW(&cls)) throw std::runtime_error("Cannot register native scene window");
            const DWORD style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
            RECT rectangle{0,0,LONG(width),LONG(height)}; AdjustWindowRect(&rectangle,style,FALSE);
            window=CreateWindowW(cls.lpszClassName,L"Deadlocked native terrain | WASD move | arrows look | Esc exit",
                                 style,CW_USEDEFAULT,CW_USEDEFAULT,rectangle.right-rectangle.left,rectangle.bottom-rectangle.top,
                                 nullptr,nullptr,cls.hInstance,nullptr);
            if (!window) throw std::runtime_error("Cannot create native scene window");
            ShowWindow(window,SW_SHOW);
        }
        Renderer renderer(scene,width,height,software,window);
        std::cout << "Native terrain: " << scene.vertices.size()/3 << " triangles, " << scene.pngs.size()
                  << " textures, " << scene.batches.size() << " draw batches; "
                  << (renderer.uses_software() ? "WARP" : "hardware") << " Direct3D 11.\n";
        if (!output.empty()) {
            renderer.render(camera);
            const auto pixels=renderer.capture(output);
            std::cout << "Captured " << pixels.drawn << " non-background pixels.\n";
            if (self_test && pixels.red < 10000) throw std::runtime_error("Render self-test did not draw the red triangle");
            std::ofstream report(output.wstring()+L".json");
            report << "{\n  \"kind\": \"native-terrain-capture\",\n  \"triangles\": " << scene.vertices.size()/3
                   << ",\n  \"textures\": " << scene.pngs.size() << ",\n  \"draw_batches\": " << scene.batches.size()
                   << ",\n  \"non_background_pixels\": " << pixels.drawn << ",\n  \"software_renderer\": "
                   << (renderer.uses_software()?"true":"false") << ",\n  \"game_logic_running\": false,\n"
                   << "  \"camera\": [" << camera.position.x << ',' << camera.position.y << ',' << camera.position.z
                   << ',' << camera.yaw << ',' << camera.pitch << "]\n}\n";
            if (!report) throw std::runtime_error("Cannot save capture report");
            return 0;
        }
        auto previous=std::chrono::steady_clock::now();
        bool running=true;
        while (running) {
            MSG event;
            while (PeekMessageW(&event,nullptr,0,0,PM_REMOVE)) {
                if (event.message==WM_QUIT) running=false;
                TranslateMessage(&event); DispatchMessageW(&event);
            }
            if (!running) break;
            auto now=std::chrono::steady_clock::now();
            const float dt=std::min(std::chrono::duration<float>(now-previous).count(),0.1f); previous=now;
            if (GetForegroundWindow()==window) {
                auto key=[](int code) { return (GetAsyncKeyState(code)&0x8000)!=0; };
                if (key(VK_ESCAPE)) { DestroyWindow(window); continue; }
                camera.yaw+=(float(key(VK_LEFT))-float(key(VK_RIGHT)))*dt;
                camera.pitch=std::clamp(camera.pitch+(float(key(VK_UP))-float(key(VK_DOWN)))*dt,-1.5f,1.5f);
                const float speed=dt*(key(VK_SHIFT)?150.f:30.f);
                auto forward=camera.forward();
                auto right=XMVector3Normalize(XMVector3Cross(forward,XMVectorSet(0,0,1,0)));
                auto position=XMLoadFloat3(&camera.position);
                position+=forward*((float(key('W'))-float(key('S')))*speed);
                position+=right*((float(key('D'))-float(key('A')))*speed);
                position+=XMVectorSet(0,0,(float(key('E'))-float(key('Q')))*speed,0);
                XMStoreFloat3(&camera.position,position);
            }
            renderer.render(camera);
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
