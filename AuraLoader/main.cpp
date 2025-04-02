#include <Windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <vector>
#include <ctime>
#include <cstdlib>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

struct Particle {
    float x, y, speed;
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool g_ExitApplication = false;
std::vector<Particle> particles;

LRESULT CALLBACK window_procedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    if (msg == WM_DESTROY || msg == WM_QUIT) {
        g_ExitApplication = true;
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    srand(static_cast<unsigned>(time(nullptr)));

    WNDCLASSEX wc = { sizeof(wc), CS_CLASSDC, window_procedure, 0L, 0L, hInstance, nullptr, nullptr, nullptr, nullptr, L"SnowWindow", nullptr };
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        wc.lpszClassName,
        L"Snow FX",
        WS_POPUP,
        100, 100, 800, 600,
        nullptr, nullptr, hInstance, nullptr
    );

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.OutputWindow = hwnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    D3D_FEATURE_LEVEL featureLevel;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* renderTarget = nullptr;

    D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &context
    );

    ID3D11Texture2D* backBuffer = nullptr;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    device->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
    backBuffer->Release();

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);
    ImGui::StyleColorsDark();

    const int screenWidth = 800;
    const int screenHeight = 600;

    const int particleCount = 150;
    for (int i = 0; i < particleCount; ++i) {
        particles.push_back({
            static_cast<float>(rand() % screenWidth),
            static_cast<float>(rand() % screenHeight),
            static_cast<float>(1 + rand() % 3)
            });
    }

    while (!g_ExitApplication) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        for (auto& p : particles) {
            p.y += p.speed;
            if (p.y > screenHeight)
                p.y = 0.0f;

            drawList->AddCircleFilled(ImVec2(p.x, p.y), 2.5f, IM_COL32(255, 255, 255, 200));
        }

        ImGui::Render();
        const float clearColor[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
        context->OMSetRenderTargets(1, &renderTarget, nullptr);
        context->ClearRenderTargetView(renderTarget, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        swapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    if (renderTarget) renderTarget->Release();
    if (swapChain) swapChain->Release();
    if (context) context->Release();
    if (device) device->Release();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, hInstance);

    return 0;
}
