#include <iostream>
#include <vector>
#include "Graphic.h"
#include "StringUtility.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include "wrl.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;

Graphic::Graphic(HWND window)
{
    AssertOK(CoInitialize(NULL));
    
    AssertOK(CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory)));
    for (auto adapter : GetAdapters(m_Factory.Get()))
    {
        DXGI_ADAPTER_DESC adapterDesc;
        AssertOK(adapter->GetDesc(&adapterDesc));
        std::cout << WideToMultiByte(adapterDesc.Description).data() << std::endl;
    }

    AssertOK(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device)));
}

Graphic::~Graphic()
{
    CoUninitialize();
}

std::vector<ComPtr<IDXGIAdapter>> Graphic::GetAdapters(IDXGIFactory7 *factory)
{
    auto adapters = std::vector<ComPtr<IDXGIAdapter>>();
    for (auto i = 0; ; i++)
    {
        ComPtr<IDXGIAdapter> adapter;
        auto result = factory->EnumAdapters(i, &adapter);
        if(result == DXGI_ERROR_NOT_FOUND) break;
        AssertOK(result);
        adapters.push_back(adapter);
    }
    return adapters;
}
