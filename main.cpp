// Dear ImGui: standalone example application for Windows API + DirectX 11
#include "logic.h"
#include <random>
#include <ctime>
#include <vector>
#include <string>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// 实例化逻辑类（全局静态变量，确保在 GUI 循环中持续存在）
static std::vector<Passenger> g_passengers;
static TravelAgency g_agency;
static TicketOffice g_office;
static bool g_isSystemInitialized = false;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int main(int, char**)
{
    // DPI Awareness
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TravelSimClass", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"旅行模拟系统 - GUI控制台", WS_OVERLAPPEDWINDOW, 100, 100, (int)(1280 * main_scale), (int)(800 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // 加载中文字体（确保 msyh.ttc 在 exe 旁边）
    io.Fonts->AddFontFromFileTTF("msyh.ttc", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // --- 核心逻辑 UI 窗口 ---
        {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
            ImGui::Begin(u8"旅行模拟系统主控台");

            // 1. 初始化按钮
            if (ImGui::Button(u8"生成/重置模拟数据", ImVec2(220, 40))) {
                std::mt19937 gen(static_cast<unsigned>(std::time(nullptr)));
                std::uniform_int_distribution<> distCount(15, 30); 
                std::uniform_int_distribution<> distDest(0, 2);
                std::uniform_int_distribution<> distLug(1, 3);
                std::uniform_int_distribution<> distJoin(0, 1);
                std::string namePool = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

                int passengerCount = distCount(gen);
                g_passengers.clear();
                g_passengers.resize(passengerCount);

                // 初始化旅行团
                for (int i = 0; i < 5; i++) {
                    if (g_agency.teams[i]) delete g_agency.teams[i];
                    g_agency.teams[i] = new Team((destination)distDest(gen), 0);
                }

                // 分配旅客
                for (int i = 0; i < passengerCount; i++) {
                    destination d = (destination)distDest(gen);
                    int lc = distLug(gen);
                    bool wantJoin = (distJoin(gen) == 1);
                    g_passengers[i].init(namePool[i % namePool.size()], lc, d, wantJoin);

                    if (wantJoin) {
                        bool enrolled = false;
                        for (int t = 0; t < 5; t++) {
                            if (g_agency.teams[t]->dest == g_passengers[i].dest && g_agency.teams[t]->passengercount < 6) {
                                g_agency.teams[t]->tp[g_agency.teams[t]->passengercount++] = &g_passengers[i];
                                g_passengers[i].jointeam = true;
                                g_passengers[i].teamnum = t + 1;
                                enrolled = true;
                                break;
                            }
                        }
                        if (!enrolled) g_passengers[i].jointeam = false;
                    }
                }
                g_isSystemInitialized = true;
            }

            if (g_isSystemInitialized) {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0, 1, 0, 1), u8"当前状态：已生成 %d 位旅客数据", (int)g_passengers.size());

                if (ImGui::BeginTabBar("DataTabs")) {
                    if (ImGui::BeginTabItem(u8"乘客名单")) {
                        if (ImGui::BeginTable("Table1", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 300))) {
                            ImGui::TableSetupColumn(u8"姓名");
                            ImGui::TableSetupColumn(u8"目的地");
                            ImGui::TableSetupColumn(u8"行李数");
                            ImGui::TableSetupColumn(u8"出行方式");
                            ImGui::TableHeadersRow();

                            for (auto& p : g_passengers) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0); ImGui::Text("%s", p.idname.c_str());
                                ImGui::TableSetColumnIndex(1); ImGui::Text("%s", desti(p.dest).c_str());
                                ImGui::TableSetColumnIndex(2); ImGui::Text("%d", p.luggagecount);
                                ImGui::TableSetColumnIndex(3); 
                                if (p.jointeam) ImGui::TextColored(ImVec4(1,1,0,1), u8"团 %d", p.teamnum);
                                else ImGui::Text(u8"自由行");
                            }
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem(u8"旅行社概况")) {
                        ImGui::Text(u8"当前团组分配情况：");
                        for (int i = 0; i < 5; i++) {
                            ImGui::BulletText(u8"旅行团 %d -> 目的地: [%s] | 当前人数: %d/6", 
                                i + 1, desti(g_agency.teams[i]->dest).c_str(), g_agency.teams[i]->passengercount);
                        }
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            } else {
                ImGui::Text(u8"请点击按钮初始化系统...");
            }
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); 
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// ----------------- D3D11 助手函数 -----------------
bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK) return false;
    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pd3dDeviceContext) g_pd3dDeviceContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) g_mainRenderTargetView->Release();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
        case WM_SIZE: if (wParam != SIZE_MINIMIZED) { g_ResizeWidth = (UINT)LOWORD(lParam); g_ResizeHeight = (UINT)HIWORD(lParam); } return 0;
        case WM_SYSCOMMAND: if ((wParam & 0xfff0) == SC_KEYMENU) return 0; break;
        case WM_DESTROY: ::PostQuitMessage(0); return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}