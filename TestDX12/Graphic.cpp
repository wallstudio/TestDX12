#include <iostream>
#include <vector>
#include "Graphic.h"

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

		auto message = std::wstring(adapterDesc.Description);
        auto multiByteMessage = std::vector<char>(message.size() * 2 + 1);
		ZeroMemory(multiByteMessage.data(), multiByteMessage.size());
		WideCharToMultiByte(CP_UTF8, 0, message.data(), message.size(), multiByteMessage.data(), multiByteMessage.size(), NULL, NULL);
        std::cout << multiByteMessage.data() << std::endl;
    }
    
}

Graphic::~Graphic()
{
    CoUninitialize();
}
