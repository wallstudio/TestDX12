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

class SwapChainRenderTargets
{
private:
    ComPtr<IDXGISwapChain3> m_SwapChain;
    ComPtr<ID3D12DescriptorHeap> m_SwapChainRenderTargetsHeap;
    vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_DiscriptorHandles;
    vector<ComPtr<ID3D12Resource>> m_Resources;

public:
    const ComPtr<IDXGISwapChain3> SwapChain() { return m_SwapChain; }
    const UINT CurrentIndex() { return m_SwapChain->GetCurrentBackBufferIndex(); }
    const D3D12_CPU_DESCRIPTOR_HANDLE DiscriptorHandle() { return DiscriptorHandle(CurrentIndex()); }
    const D3D12_CPU_DESCRIPTOR_HANDLE DiscriptorHandle(UINT index) { return m_DiscriptorHandles[index]; }
    const ComPtr<ID3D12Resource> Resouce() { return Resouce(CurrentIndex()); }
    const ComPtr<ID3D12Resource> Resouce(UINT index) { return m_Resources[index]; }

    SwapChainRenderTargets(HWND windowHandle, ComPtr<ID3D12Device8> device, ComPtr<IDXGIFactory7> factory, ComPtr<ID3D12CommandQueue> commandQueue)
    {
        // SwapChain（Resourceの確保）
        RECT windowRect = {};
        GetClientRect(windowHandle, &windowRect);
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc =
        {
            .Width = static_cast<UINT>(windowRect.right - windowRect.left),
            .Height = static_cast<UINT>(windowRect.bottom - windowRect.top),
            .Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
            .Stereo = false,
            .SampleDesc = {.Count = 1, .Quality = 0, },
            .BufferUsage = DXGI_USAGE_BACK_BUFFER,
            .BufferCount = 2,
            .Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED,
            .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
        };
        ComPtr<IDXGISwapChain1> swapChain1;
        AssertOK(factory->CreateSwapChainForHwnd(commandQueue.Get(), windowHandle, &swapChainDesc, nullptr, nullptr, &swapChain1));
        AssertOK(swapChain1.As(&m_SwapChain));

        // Discriptor書き込み先のHeap確保
        D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc =
        {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = 2,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = 0,
        };
        AssertOK(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_SwapChainRenderTargetsHeap)));

        // Discriptor（View）書き込み
        D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandleCursor = m_SwapChainRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < swapChainDesc.BufferCount; i++)
        {
            ComPtr<ID3D12Resource> resource;
            AssertOK(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&resource)));
            m_Resources.push_back(resource);

            D3D12_RENDER_TARGET_VIEW_DESC* defaultView = nullptr;
            device->CreateRenderTargetView(resource.Get(), defaultView, descriptorHandleCursor);
            m_DiscriptorHandles.push_back(descriptorHandleCursor);

            descriptorHandleCursor.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // Seek to next
        }
    }

    void ChangeBarrier(ComPtr<ID3D12GraphicsCommandList> commands, D3D12_RESOURCE_STATES prevState, D3D12_RESOURCE_STATES nextState) { ChangeBarrier(commands, prevState, nextState, CurrentIndex()); }
    void ChangeBarrier(ComPtr<ID3D12GraphicsCommandList> commands, D3D12_RESOURCE_STATES prevState, D3D12_RESOURCE_STATES nextState, UINT index)
    {
        D3D12_RESOURCE_BARRIER barrier =
        {
            .Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition =
            {
                .pResource = Resouce(index).Get(),
                .Subresource = 0,
                .StateBefore = prevState,
                .StateAfter = nextState,
            },
        };
        commands->ResourceBarrier(1, &barrier);
    }

};
