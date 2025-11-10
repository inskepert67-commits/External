#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"
#include "imgui_freetype.h"
#include <d3d11.h>
#include <tchar.h>

#include "background.h"

#include "roboto.h"
#include "icon.h"

#include <iostream>
#include <Windows.h>

#include <D3DX11tex.h>
#pragma comment(lib, "D3DX11.lib")

#define SNOW_LIMIT 60

#include "snow.hpp"

std::vector<Snowflake::Snowflake> snow;

ID3D11ShaderResourceView* bg = nullptr;

#include "imgui_settings.h"

HWND hwnd;
RECT rc;

static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

DWORD flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize ;
DWORD picker_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview;
int tabs = 0;

static float accent[4] = { 118 / 255.f, 130 / 255.f, 255 / 255.f, 255 / 255.f };

namespace fonts {

    ImFont* widgets = nullptr;
    ImFont* roboto = nullptr;
    ImFont* icon_big = nullptr;
    ImFont* icon = nullptr;
}

int main(int, char**)
{
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
    ::RegisterClassExW(&wc);
    hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, wc.hInstance, NULL);

    POINT mouse;

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  

    ImFontConfig cfg;
    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor;

    fonts::widgets = io.Fonts->AddFontFromMemoryTTF(&roboto_medium, sizeof roboto_medium, 15, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    fonts::roboto = io.Fonts->AddFontFromMemoryTTF(&roboto_medium, sizeof roboto_medium, 17, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    fonts::icon = io.Fonts->AddFontFromMemoryTTF(&icon, sizeof icon, 18, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    fonts::icon_big = io.Fonts->AddFontFromMemoryTTF(&icon, sizeof icon, 55, &cfg, io.Fonts->GetGlyphRangesCyrillic());

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


    Snowflake::CreateSnowFlakes(snow, SNOW_LIMIT, 5.f, 30.f, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), Snowflake::vec3(0.f, 0.005f), ImGui::GetColorU32(c::bg::circle));


    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();

        c::general_color = ImLerp(c::general_color, ImColor(accent[0], accent[1], accent[2], 1.f), ImGui::GetIO().DeltaTime * 8.f);

        static bool menu = true;
        static float menu_alpha = 0.f; /* */ static float menu_add; /* */ static bool active_menu = true;

        if (GetAsyncKeyState(VK_INSERT) & 0x01) {
            menu = !menu;

            if (menu) ImGui::SetNextWindowPos(ImVec2(GetSystemMetrics(SM_CXSCREEN) / 2 - s::bg::size.x / 2, GetSystemMetrics(SM_CYSCREEN) / 2 - s::bg::size.y / 2));

        }
        ImGui::GetStyle().WindowPadding = ImVec2(0, 0);
        ImGui::GetStyle().WindowBorderSize = 0;
        ImGui::GetStyle().ScrollbarSize = 7.f;

        ImGui::GetStyle().ItemSpacing = ImVec2(20, 20);

        D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };
        if (bg == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, background, sizeof(background), &info, pump, &bg, 0);

        ImGui::GetBackgroundDrawList()->AddImage(bg, ImVec2(0, 0), ImVec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));

        ImGui::SetNextWindowSizeConstraints(ImVec2(s::bg::size), ImGui::GetIO().DisplaySize);

        menu_alpha = ImClamp(menu_alpha + (4.f * ImGui::GetIO().DeltaTime * (menu == active_menu ? 1.f : -1.f)), 0.f, 1.f);

        if (menu_alpha == 0.f && menu_add == 0.f) active_menu = menu;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menu_alpha * ImGui::GetStyle().Alpha);

        if (active_menu) {

            ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)), ImGui::GetColorU32(c::bg::background_));

            GetWindowRect(hwnd, &rc);

            GetCursorPos(&mouse);

            Snowflake::Update(snow, Snowflake::vec3(mouse.x, mouse.y), Snowflake::vec3(rc.left, rc.top));


            ImGui::Begin("##0", nullptr, flags);
            {
                static bool menu_active = false;
                static char login[16], password[16];
                std::string login_success = "12345";

                ImGui::GetBackgroundDrawList()->AddRectFilled(ImGui::GetWindowPos() + ImVec2(0, 0), ImGui::GetWindowPos() + ImVec2(ImGui::GetContentRegionMax().x, ImGui::GetContentRegionMax().y), ImGui::GetColorU32(c::bg::background), s::bg::rounding);

                if (!menu_active) {

                    ImGui::SetCursorPos(ImVec2((s::bg::size.x / 2) - ( 315 ) - (ImGui::GetStyle().ItemSpacing.x / 2), (s::bg::size.y / 2) - (340 / 2)));

                    ImGui::BeginGroup();
                    {
                        ImGui::BeginChild("EXSPIRA", ImVec2(315, 340));
                        {
                            ImGui::InputTextEx("##0", "License", login, 16, ImVec2(280, 30), ImGuiInputTextFlags_None);

                            // static bool remember_me = false;
                            // ImGui::Checkbox1("Remember", "Save at the next login" , &remember_me);

                             //   ImGui::TextColored(ImColor(ImStyle::text::have_account), "I don't have an account?");
                             //   if (ImGui::IsItemClicked()) ShellExecute(NULL, "open", "YOUR LINK", NULL, NULL, SW_SHOW);

                            if (ImGui::Button("Auth", ImVec2(280, 30)) && login == login_success) menu_active = true;


                            ImGui::TextColored(ImColor(64, 64, 64, 255), "e-Mail: exspira@exspiracheats.com\nwww.exspiracheats.com");

                        }
                        ImGui::EndChild();
                    }
                    ImGui::EndGroup();

                    ImGui::SameLine();

                    ImGui::BeginGroup();
                    {
                        ImGui::BeginChild("INFORMATION", ImVec2(315, 340));
                        {

                            ImGui::TextSeparator(ImColor(64, 64, 64, 255), "KEY INFORMATION");

                            ImGui::TextColored(ImColor(64, 64, 64, 255), "LICENSE KEY: 0000-0000-0000-0000");

                            ImGui::TextSeparator(ImColor(64, 64, 64, 255), "INFORMATION");

                            ImGui::TextColored(ImColor(64, 64, 64, 255), "Announcements: Welcome To Demonic Software");


                            if (ImGui::Button("LOAD", ImVec2(280, 30)));

                        }
                        ImGui::EndChild();
                    }
                    ImGui::EndGroup();

                }else {

                    ImGui::PushFont(fonts::icon_big);
                    ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowPos() + ImVec2((90.f / 2) - (ImGui::CalcTextSize("K").x) / 2.f, (85.f / 2) - (ImGui::CalcTextSize("K").x) / 2.f), ImGui::GetColorU32(c::general_color), "K");
                    ImGui::PopFont();

                    ImGui::GetBackgroundDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(0, 90), ImGui::GetWindowPos() + ImVec2(90, 90), ImGui::GetColorU32(c::bg::border));

                    ImGui::SetCursorPosY(90);

                    ImGui::BeginGroup();
                    {
                        if (ImGui::Tabs(0 == tabs, "A", "ASSIST", ImVec2(90, 90))) tabs = 0;

                        if (ImGui::Tabs(1 == tabs, "B", "VISUALS", ImVec2(90, 90))) tabs = 1;

                        if (ImGui::Tabs(2 == tabs, "C", "MISC", ImVec2(90, 90))) tabs = 2;

                        if (ImGui::Tabs(3 == tabs, "D", "COLORS", ImVec2(90, 90))) tabs = 3;

                        if (ImGui::Tabs(4 == tabs, "E", "TRIAL", ImVec2(90, 90))) tabs = 4;
                    }
                    ImGui::EndGroup();

                    ImGui::GetBackgroundDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(90, 0), ImGui::GetWindowPos() + ImVec2(90, ImGui::GetContentRegionMax().y), ImGui::GetColorU32(c::bg::border));

                    ImGui::SetCursorPos(ImVec2(110, 20));

                    static float tab_alpha = 0.f; /* */ static float tab_add; /* */ static int active_tab = 0;

                    tab_alpha = ImClamp(tab_alpha + (4.f * ImGui::GetIO().DeltaTime * (tabs == active_tab ? 1.f : -1.f)), 0.f, 1.f);
                    tab_add = ImClamp(tab_add + (std::round(350.f) * ImGui::GetIO().DeltaTime * (tabs == active_tab ? 1.f : -1.f)), 0.f, 1.f);

                    if (tab_alpha == 0.f && tab_add == 0.f) active_tab = tabs;

                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * ImGui::GetStyle().Alpha);

                    if (active_tab == 1) {

                        ImGui::BeginGroup();
                        {

                            ImGui::BeginChild("GENERAL", ImVec2(285, ImGui::GetContentRegionMax().y - 35), true);
                            {

                                static bool check_0;
                                ImGui::Checkbox1("Enabled", "Visuals Enabled", &check_0);

                                static bool check_1;
                                ImGui::Checkbox1("Enemy only", "Visuals for enemies only", &check_1);

                                ImGui::TextSeparator(ImGui::GetColorU32(c::bg::text), "CHARACTER");

                                static bool check_2;
                                ImGui::Checkbox1("Box", "Draw player box", &check_2);

                                static bool check_3;
                                ImGui::Checkbox1("Box Outline", "Draw outline box", &check_3);

                                static bool check_4;
                                ImGui::Checkbox1("Skeleton", "Draw skeleton", &check_4);

                                static float slider_0 = 0.5f;
                                ImGui::SliderFloat("Skeleton Tickness", &slider_0, 0.5f, 1.5f, "%.2fPPI");

                                static int slider_1 = 1500;
                                ImGui::SliderInt("Maximum Distance", &slider_1, 0, 2500, "%d feets");

                                static int select = 0;
                                const char* items[3]{ "One", "Two", "Three" };
                                ImGui::Combo("Player Info", "Select info to display", &select, items, IM_ARRAYSIZE(items), 3);

                                ImGui::SetCursorPosY(1000);
                            }
                            ImGui::EndChild();

                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();

                        ImGui::BeginGroup();
                        {

                            ImGui::BeginChild("MISC", ImVec2(285, ImGui::GetContentRegionMax().y - 35), true);
                            {
                                //   static bool check_5;
                                //   ImGui::Checkbox1("Checkbox", "Hello checkbox :)", &check_5);

                                static bool multi_num[5] = { true, true, true, true, false };
                                const char* multi_items[5] = { "One", "Two", "Three", "Four", "Five" };
                                ImGui::MultiCombo("MultiCombo", "Hello multicombo :)", multi_num, multi_items, 5);

                                if (ImGui::Button("Button", ImVec2(ImGui::GetContentRegionMax().x - ImGui::GetStyle().WindowPadding.x, 30.f)));

                                static int key0 = 0;
                                ImGui::Keybind("Keybind", "Hello keybind :)", &key0);

                            }
                            ImGui::EndChild();

                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();

                        ImGui::BeginGroup();
                        {

                            ImGui::BeginChild("SETTINGS", ImVec2(285, ImGui::GetContentRegionMax().y - 35), true);
                            {

                                ImGui::ColorEdit4("GUI", "Accentuated interface color", accent, picker_flags | ImGuiColorEditFlags_NoAlpha);

                                static char input[25] = { "" };
                                ImGui::InputTextEx("##0", "Input...", input, 25, ImVec2(ImGui::GetContentRegionMax().x - ImGui::GetStyle().WindowPadding.x, 30.f), ImGuiInputTextFlags_None);
                            }
                            ImGui::EndChild();

                        }
                        ImGui::EndGroup();

                    }

                    ImGui::PopStyleVar();

                }

            }
            ImGui::End();

        }

        ImGui::PopStyleVar();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
