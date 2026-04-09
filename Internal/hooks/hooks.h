#pragma once
#include <d3d11.h>
#include <dxgi.h>

typedef HRESULT(STDMETHODCALLTYPE* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
extern Present_t oPresent;

HRESULT STDMETHODCALLTYPE hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
