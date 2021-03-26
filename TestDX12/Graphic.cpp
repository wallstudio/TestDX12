#include <iostream>
#include <vector>
#include "Graphic.h"
#include "StringUtility.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include "wrl.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define AssertOK(result) do { if(FAILED(result))throw new std::exception("F" #result); } while(false)

using namespace Microsoft::WRL;

Graphic::Graphic(HWND window)
{
    AssertOK(CoInitialize(NULL));
    
    AssertOK(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device)));
    AssertOK(CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory)));

    auto adapters = std::vector<ComPtr<IDXGIAdapter>>();
    for (auto i = 0; ; i++)
    {
        ComPtr<IDXGIAdapter> adapter;
        auto result = m_Factory->EnumAdapters(i, &adapter);
        if(result == DXGI_ERROR_NOT_FOUND) break;
        AssertOK(result);
        adapters.push_back(adapter);
    }
    for (auto adapter : adapters)
    {
        DXGI_ADAPTER_DESC adapterDesc;
        AssertOK(adapter->GetDesc(&adapterDesc));
        std::cout << WideToMultiByte(adapterDesc.Description).data() << std::endl;
    }
}

Graphic::~Graphic()
{
    CoUninitialize();
}
