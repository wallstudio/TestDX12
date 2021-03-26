#pragma once

#include <Windows.h>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include "wrl.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;

class Graphic
{
private:
    ComPtr<ID3D12Device8> m_Device;
    ComPtr<IDXGIFactory7> m_Factory;
    ComPtr<IDXGISwapChain4> m_SwapChain;
public:
    Graphic(HWND window);
    ~Graphic();
private:
    static std::vector<ComPtr<IDXGIAdapter>> GetAdapters(IDXGIFactory7 *factory);
};