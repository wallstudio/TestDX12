#pragma once

#include <Windows.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include "wrl.h"
#pragma comment(lib, "d3d12.lib")

#include "Assertion.h"


class MVP
{
private:
    Microsoft::WRL::ComPtr<ID3D12Device8> m_Device;
    D3D12_HEAP_PROPERTIES m_HeapProp = {};
    D3D12_RESOURCE_DESC m_ResourceDesc = {};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
    D3D12_CONSTANT_BUFFER_VIEW_DESC m_View;

public:
    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeap() { return m_DescriptorHeap; }
    const D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHandle() { return m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart(); }

    MVP(const Microsoft::WRL::ComPtr<ID3D12Device8> device, HWND windowHandle)
    {
        using namespace Microsoft::WRL;
        using namespace DirectX;

        m_Device = device;
    
        RECT windowRect;
        GetClientRect(windowHandle, &windowRect);
        auto width = windowRect.right - windowRect.left;
        auto height = windowRect.bottom - windowRect.top;

        auto model = XMMatrixIdentity();
        auto camera = XMMatrixIdentity();
        // 2D ((0, 0) -> (-1, +1); (w, h) -> (+1, -1))
        auto project = XMMatrixScaling(1.0f / width, -1.0f / height, 1.0f) * XMMatrixTranslation(-1.0f, +1.0f, 0);
        // 3D
        // auto project = XMMatrixIdentity();
        auto mvp = model * camera * project;
        auto alignedSize = static_cast<UINT>(std::ceil(sizeof(XMMATRIX) / 256.0f)) * 256;

        m_HeapProp =
        {
            .Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD,
            .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            .MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN,
            .CreationNodeMask = 0,
            .VisibleNodeMask = 0,
        };
        m_ResourceDesc =
        {
            .Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = alignedSize,
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN,
            .SampleDesc = { .Count = 1, .Quality = 0, },
            .Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE,
        };
        AssertOK(m_Device->CreateCommittedResource(&m_HeapProp, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &m_ResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_Resource)));
        void* mapping;
        AssertOK(m_Resource->Map(0, nullptr, &mapping));
        std::memcpy(mapping, &mvp, alignedSize);
        m_Resource->Unmap(0, nullptr);

        // Descriptor
        D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc =
        {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, // CBV
            .NumDescriptors = 1,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
            .NodeMask = 0,
        };
        AssertOK(m_Device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_DescriptorHeap)));
        m_View =
        {
            .BufferLocation = m_Resource->GetGPUVirtualAddress(),
            .SizeInBytes = alignedSize,
        };
        m_Device->CreateConstantBufferView(&m_View, m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    }

};
