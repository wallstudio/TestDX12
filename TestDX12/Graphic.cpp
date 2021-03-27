#include <iostream>
#include <vector>
#include <array>
#include <tuple>
#include <chrono>
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
    m_WindowHandle = window;
    AssertOK(CoInitialize(NULL));

    ComPtr<ID3D12Debug> debug;
    AssertOK(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
    debug->EnableDebugLayer();
    ComPtr<ID3D12Debug1> debugEx;
    AssertOK(debug->QueryInterface(IID_PPV_ARGS(&debugEx)));
    debugEx->SetEnableGPUBasedValidation(true);
    
    AssertOK(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_Factory)));
    for (auto adapter : GetAdapters(m_Factory.Get()))
    {
        DXGI_ADAPTER_DESC adapterDesc;
        AssertOK(adapter->GetDesc(&adapterDesc));
        std::cout << WideToMultiByte(adapterDesc.Description).data() << std::endl;
    }

    AssertOK(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device)));

    AssertOK(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));
    AssertOK(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
    AssertOK(m_CommandList->Close());
    D3D12_COMMAND_QUEUE_DESC commandQueueCreateDesc = {};
    commandQueueCreateDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
    commandQueueCreateDesc.NodeMask = 0;
    commandQueueCreateDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    commandQueueCreateDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
    AssertOK(m_Device->CreateCommandQueue(&commandQueueCreateDesc, IID_PPV_ARGS(&m_CommandQueue)));

    RECT windowRect = {};
    GetClientRect(m_WindowHandle, &windowRect);
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = windowRect.right - windowRect.left;
    swapChainDesc.Height = windowRect.bottom - windowRect.top;
    swapChainDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = false;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    ComPtr<IDXGISwapChain1> swapChain1;
    AssertOK(m_Factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_WindowHandle, &swapChainDesc, nullptr, nullptr, &swapChain1));
    AssertOK(swapChain1.As(&m_SwapChain));

    D3D12_DESCRIPTOR_HEAP_DESC swapChainRenderTargetsHeaDesc = {};
    swapChainRenderTargetsHeaDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    swapChainRenderTargetsHeaDesc.NumDescriptors = 2;
    swapChainRenderTargetsHeaDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    swapChainRenderTargetsHeaDesc.NodeMask = 0;
    AssertOK(m_Device->CreateDescriptorHeap(&swapChainRenderTargetsHeaDesc, IID_PPV_ARGS(&m_SwapChainRenderTargetsHeap)));

    D3D12_CPU_DESCRIPTOR_HANDLE swapChainRenderTargetDescriptorHandle = m_SwapChainRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < swapChainDesc.BufferCount; i++)
    {
        auto currentHandle = swapChainRenderTargetDescriptorHandle;
        swapChainRenderTargetDescriptorHandle.ptr += m_Device->GetDescriptorHandleIncrementSize(swapChainRenderTargetsHeaDesc.Type);
        ComPtr<ID3D12Resource> resource;
        AssertOK(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&resource)));
        m_Device->CreateRenderTargetView(resource.Get(), nullptr, currentHandle);
        if(nullptr == resource.Get()) throw std::exception("Failed create RenderTargetView");
        m_SwapChainRenderTargets.push_back({ currentHandle, resource });
    }
    
    AssertOK(m_Device->CreateFence(~0, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
}

Graphic::~Graphic()
{
    CoUninitialize();
}

UINT64 Graphic::Rendring()
{
    auto frameNumberForFence = m_Fence->GetCompletedValue() + 1;
    AssertOK(m_CommandAllocator->Reset());
    AssertOK(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));

    auto currentSwapChainRenderTarget = m_SwapChainRenderTargets[m_SwapChain->GetCurrentBackBufferIndex()];
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition = {};
    barrier.Transition.pResource = currentSwapChainRenderTarget.Resource.Get();
    barrier.Transition.Subresource = 0;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_CommandList->ResourceBarrier(1, &barrier);

    m_CommandList->OMSetRenderTargets(1, std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 1>({currentSwapChainRenderTarget.Handle}).data(), false, nullptr);
    m_CommandList->ClearRenderTargetView(currentSwapChainRenderTarget.Handle, std::array<FLOAT, 4>({ 1.0f, 0.0f, 0.0f, 1.0f }).data(), 0, nullptr);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT;
    m_CommandList->ResourceBarrier(1, &barrier);
    
    AssertOK(m_CommandList->Close());
    m_CommandQueue->ExecuteCommandLists(1, std::vector<ID3D12CommandList*>({ m_CommandList.Get() }).data());

    AssertOK(m_SwapChain->Present(DXGI_SWAP_EFFECT_SEQUENTIAL, 0 /* DXGI_PRESENT */));
    AssertOK(m_CommandQueue->Signal(m_Fence.Get(), frameNumberForFence));

    while (m_Fence->GetCompletedValue() != frameNumberForFence);
    return frameNumberForFence;
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
