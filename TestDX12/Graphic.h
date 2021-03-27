#pragma once

#include <Windows.h>
#include <vector>
#include <string>
#include <exception>

#include <d3d12.h>
#include <dxgi1_6.h>
#include "wrl.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;

#define AssertOK(operation) \
    do { \
        const auto __result = operation; \
        if(FAILED(__result)) { \
            auto __message = std::string("Failed\n\n" #operation "\n\n"); \
            __message += ToMultiByte(ToTString(__result).data()); \
            throw std::exception(__message.data()); \
        } \
    } while(false)

struct Resouce
{
    D3D12_CPU_DESCRIPTOR_HANDLE Handle;
    ComPtr<ID3D12Resource> Resource;
};

class Graphic
{
private:
    HWND m_WindowHandle;
    ComPtr<ID3D12Device8> m_Device;
    ComPtr<IDXGIFactory7> m_Factory;

    ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    ComPtr<ID3D12CommandQueue> m_CommandQueue;
    ComPtr<ID3D12Fence> m_Fence;
    
    ComPtr<IDXGISwapChain3> m_SwapChain;
    ComPtr<ID3D12DescriptorHeap> m_SwapChainRenderTargetsHeap;
    std::vector<Resouce> m_SwapChainRenderTargets = std::vector<Resouce>();
public:
    Graphic(HWND window);
    ~Graphic();
    UINT64 Rendring();
private:
    static std::vector<ComPtr<IDXGIAdapter>> GetAdapters(IDXGIFactory7 *factory);
};