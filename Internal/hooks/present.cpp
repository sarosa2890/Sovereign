#include "hooks.h"
#include <d3d11.h>
#include <dxgi.h>
#include <cmath>
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_dx11.h"
#include "../imgui/backends/imgui_impl_win32.h"
#include "../minhook/include/MinHook.h"
#include "../features/features.h"
#include "../sdk/config.h"
#include "../sdk/structs.h"
#include <iostream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present_t oPresent = nullptr;

HWND window = nullptr;
ID3D11Device* pDevice = nullptr;
ID3D11DeviceContext* pContext = nullptr;
ID3D11RenderTargetView* mainRenderTargetView = nullptr;
WNDPROC oWndProc = nullptr;

bool init = false;
bool showMenu = true;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN && wParam == VK_INSERT) {
        showMenu = !showMenu;
        return true;
    }

    if (showMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
        return true;
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

void SetupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowPadding = ImVec2(15, 15);
    style.WindowRounding = 10.0f;
    style.FramePadding = ImVec2(8, 6);
    style.FrameRounding = 5.0f;
    style.ItemSpacing = ImVec2(10, 10);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 5.0f;
    style.ChildRounding = 8.0f;
    style.TabRounding = 5.0f;

    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.08f, 0.11f, 0.80f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.24f, 0.22f, 0.27f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.18f, 0.23f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.25f, 0.32f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.56f, 0.27f, 0.97f, 1.00f); // Purple Accent
    colors[ImGuiCol_SliderGrab] = ImVec4(0.56f, 0.27f, 0.97f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.76f, 0.47f, 0.98f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.16f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.36f, 0.17f, 0.77f, 0.80f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.27f, 0.97f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.16f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.36f, 0.17f, 0.77f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.56f, 0.27f, 0.97f, 1.00f);
    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.39f, 0.10f, 0.71f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.56f, 0.27f, 0.97f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.56f, 0.27f, 0.97f, 1.00f);
}

void Draw3DCube(ImVec2 center, float size, float time, ImDrawList* draw_list) {
    Vector3 vertices[8] = {
        {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
        {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
    };

    ImVec2 proj[8];
    for (int i = 0; i < 8; i++) {
        float x = vertices[i].x;
        float y = vertices[i].y;
        float z = vertices[i].z;

        // Rotate Y
        float nx = x * cosf(time) - z * sinf(time);
        float nz = x * sinf(time) + z * cosf(time);
        x = nx; z = nz;

        // Rotate X
        float ny = y * cosf(time * 0.5f) - z * sinf(time * 0.5f);
        nz = y * sinf(time * 0.5f) + z * cosf(time * 0.5f);
        y = ny; z = nz;

        // Project
        float distance = 3.0f;
        float z_proj = 1.0f / (distance - z);
        float fov = 150.0f;
        
        proj[i].x = center.x + (x * fov * z_proj) * (size / 100.0f);
        proj[i].y = center.y + (y * fov * z_proj) * (size / 100.0f);
    }

    int edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    ImU32 color = ImColor::HSV(fmodf(time * 0.2f, 1.0f), 0.8f, 0.8f);

    for (int i = 0; i < 12; i++) {
        draw_list->AddLine(proj[edges[i][0]], proj[edges[i][1]], color, 2.0f);
    }
}

HRESULT STDMETHODCALLTYPE hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!init) {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
            pDevice->GetImmediateContext(&pContext);
            window = FindWindowA("SDL_app", "Counter-Strike 2");
            if (!window) {
                DXGI_SWAP_CHAIN_DESC sd;
                pSwapChain->GetDesc(&sd);
                window = sd.OutputWindow;
            }

            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
            pBackBuffer->Release();

            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            
            SetupImGuiStyle();

            ImGui_ImplWin32_Init(window);
            ImGui_ImplDX11_Init(pDevice, pContext);

            init = true;
        }
        else {
            return oPresent(pSwapChain, SyncInterval, Flags);
        }
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    float time = (float)ImGui::GetTime();

    if (showMenu) {
        ImGui::SetNextWindowSize(ImVec2(650, 450), ImGuiCond_FirstUseEver);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        
        ImGui::Begin("SOVEREIGN INTERNAL", &showMenu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        
        ImVec2 p = ImGui::GetCursorScreenPos();
        float w = ImGui::GetWindowWidth();
        
        ImU32 col1 = ImColor::HSV(fmodf(time * 0.2f, 1.0f), 0.8f, 0.8f);
        ImU32 col2 = ImColor::HSV(fmodf(time * 0.2f + 0.5f, 1.0f), 0.8f, 0.8f);
        
        // Animated RGB Top Bar
        ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(p.x - 15, p.y - 15), ImVec2(p.x + w + 15, p.y - 12), col1, col2, col2, col1);
        
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.97f, 1.0f), "SOVEREIGN");
        ImGui::SameLine();
        ImGui::TextDisabled("| PREMIUM INTERNAL");
        ImGui::Dummy(ImVec2(0, 10));

        // Setup Layout
        ImGui::Columns(2, "Layout", false);
        ImGui::SetColumnWidth(0, 150);

        // Left Column (Tabs)
        static int activeTab = 0;
        if (ImGui::Button("Visuals", ImVec2(130, 40))) activeTab = 0;
        if (ImGui::Button("Aimbot", ImVec2(130, 40))) activeTab = 1;
        if (ImGui::Button("Misc", ImVec2(130, 40))) activeTab = 2;
        if (ImGui::Button("Config", ImVec2(130, 40))) activeTab = 3;
        
        // 3D Cube below tabs
        ImGui::Dummy(ImVec2(0, 80));
        ImVec2 center = ImGui::GetCursorScreenPos();
        center.x += 65; center.y += 20;
        Draw3DCube(center, 40.0f, time, ImGui::GetWindowDrawList());

        ImGui::NextColumn();

        // Right Column (Content)
        ImGui::BeginChild("Content", ImVec2(0, 0), true);
        
        if (activeTab == 0) {
            ImGui::Text("ESP Features");
            ImGui::Separator();
            ImGui::Checkbox("Enable ESP Master", &config::esp);
            if (config::esp) {
                ImGui::Indent();
                ImGui::Checkbox("Draw Bounding Boxes", &config::esp_boxes);
                ImGui::Checkbox("Draw Player Skeletons", &config::esp_skeletons);
                ImGui::Checkbox("Draw Player Names", &config::esp_names);
                ImGui::Checkbox("Draw Weapon Name", &config::esp_weapons);
                ImGui::Checkbox("Draw View Camera", &config::esp_camera);
                ImGui::Checkbox("Draw Reload State", &config::esp_reload);
                ImGui::Unindent();
            }
        }
        else if (activeTab == 1) {
            ImGui::Text("Aimbot Assistance");
            ImGui::Separator();
            ImGui::Checkbox("Enable Aimbot", &config::aimbot);
            if (config::aimbot) {
                ImGui::Checkbox("Visibility Check", &config::aim_visible_check);
                ImGui::Checkbox("Draw FOV Circle", &config::draw_fov);
                ImGui::SliderFloat("FOV Radius", &config::aim_fov, 1.0f, 180.0f, "%.1f deg");
                ImGui::SliderFloat("Smooth Speed", &config::aim_smooth, 1.0f, 20.0f, "%.1f");
            }
            ImGui::Dummy(ImVec2(0, 10));
            ImGui::Text("Triggerbot");
            ImGui::Separator();
            ImGui::Checkbox("Enable Auto-Fire", &config::triggerbot);
        }
        else if (activeTab == 2) {
            ImGui::Text("Miscellaneous");
            ImGui::Separator();
            ImGui::TextDisabled("Bhop removed for stability.");
            ImGui::TextDisabled("More features coming soon...");
        }
        else if (activeTab == 3) {
            ImGui::Text("Configuration");
            ImGui::Separator();
            if (ImGui::Button("Save CFG Deeply", ImVec2(150, 30))) { config::Save(); }
            if (ImGui::Button("Load CFG", ImVec2(150, 30))) { config::Load(); }
            ImGui::Dummy(ImVec2(0, 20));
            ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.97f, 1.0f), "made by sarosa2840");
        }

        ImGui::EndChild();

        ImGui::Columns(1);
        ImGui::End();
        ImGui::PopStyleVar();
    }

    if (config::esp || (config::draw_fov && config::aimbot)) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);
        
        if (config::draw_fov && config::aimbot) {
            float radius = tanf(config::aim_fov * (3.1415926535f / 180.0f) / 2.0f) / tanf(90.0f * (3.1415926535f / 180.0f) / 2.0f) * (io.DisplaySize.x / 2.0f);
            ImGui::GetWindowDrawList()->AddCircle(ImVec2(io.DisplaySize.x / 2.0f, io.DisplaySize.y / 2.0f), radius, IM_COL32(255, 255, 255, 150), 64, 1.0f);
        }

        if (config::esp) {
            features::RenderESP();
        }
        
        ImGui::End();
    }

    ImGui::Render();

    // Preserve the original RenderTarget
    ID3D11RenderTargetView* oRTV = nullptr;
    pContext->OMGetRenderTargets(1, &oRTV, NULL);

    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    if (oRTV) {
        pContext->OMSetRenderTargets(1, &oRTV, NULL);
        oRTV->Release();
    }

    return oPresent(pSwapChain, SyncInterval, Flags);
}
