#pragma once

#include <Windows.h>
#include <vector>
#include <string>
#include <exception>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "wrl.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
using namespace Microsoft::WRL;
using namespace DirectX;

#include "StringUtility.h"
#include "Mesh.h"


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

    std::shared_ptr<Mesh> m_Mesh;

    std::vector<ComPtr<ID3D12PipelineState>> piplineStates;
public:
    Graphic(HWND window);
    ~Graphic();
    UINT64 Rendring();
private:
    static std::vector<ComPtr<IDXGIAdapter>> GetAdapters(IDXGIFactory7 *factory);
};