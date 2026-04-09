#include "hooks/hooks.h"
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "minhook/include/MinHook.h"
#include "imgui/imgui.h"
#include "sdk/offsets.h"
#include "sdk/config.h"
#include "features/features.h"
#include <iostream>

DWORD WINAPI MainThread(LPVOID lpReserved) {
    // Allocate a debug console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "[+] Injection started!" << std::endl;

    // Create a dummy window for device creation
    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_CLASSDC, DefWindowProcA, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "DX11Dummy", NULL };
    RegisterClassExA(&wc);
    HWND dummyWindow = CreateWindowA("DX11Dummy", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

    if (!dummyWindow) {
        std::cout << "[-] Failed to create dummy window." << std::endl;
        return FALSE;
    }

    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;
    IDXGISwapChain* pSwapChain = nullptr;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    DXGI_SWAP_CHAIN_DESC sd = { 0 };
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = dummyWindow;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    std::cout << "[+] Creating dummy device..." << std::endl;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, NULL, &pContext);
    
    if (FAILED(hr)) {
        std::cout << "[-] Failed to create D3D11 Device and SwapChain." << std::endl;
        DestroyWindow(dummyWindow);
        UnregisterClassA("DX11Dummy", wc.hInstance);
        return FALSE;
    }

    void** pVTable = *(void***)pSwapChain;
    void* pPresentTarget = pVTable[8];
    std::cout << "[+] Found Present at: " << pPresentTarget << std::endl;

    pDevice->Release();
    pContext->Release();
    pSwapChain->Release();
    DestroyWindow(dummyWindow);
    UnregisterClassA("DX11Dummy", wc.hInstance);

    if (MH_Initialize() != MH_OK) {
        std::cout << "[-] MinHook initialization failed." << std::endl;
        return FALSE;
    }

    if (MH_CreateHook(pPresentTarget, &hkPresent, (LPVOID*)&oPresent) != MH_OK) {
        std::cout << "[-] Failed to create Present hook." << std::endl;
        return FALSE;
    }

    MH_STATUS status = MH_EnableHook(pPresentTarget);
    if (status != MH_OK) {
        std::cout << "[-] Failed to enable Present hook. Error code: " << (int)status << std::endl;
        return FALSE;
    }

    std::cout << "[+] Hook enabled! Menu should render now." << std::endl;

    while (!(GetAsyncKeyState(VK_END) & 0x8000)) {
        if (config::aimbot)
            features::RunAimbot();
        
        if (config::triggerbot)
            features::RunTriggerbot();
            
        features::RunMisc();
            
        Sleep(1);
    }

    std::cout << "[+] Unloading..." << std::endl;
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread((HMODULE)lpReserved, 0);
    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, hModule, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
