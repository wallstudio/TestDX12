#pragma once

#include <vector>
#include <array>
using namespace std;

#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "wrl.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
using namespace Microsoft::WRL;
using namespace DirectX;

#include "StringUtility.h"


class Mesh
{
private:
    ComPtr<ID3D12Device8> m_Device;
    UINT m_Size;
    D3D12_HEAP_PROPERTIES m_HeapProp = {};
    D3D12_RESOURCE_DESC m_ResourceDesc = {};
    ComPtr<ID3D12Resource> m_Resource;
    D3D12_VERTEX_BUFFER_VIEW m_View = {};
    vector<D3D12_INPUT_ELEMENT_DESC> m_InputElementDescs;
public:
    const UINT Size() { return m_Size; }
    const D3D12_VERTEX_BUFFER_VIEW *const View() { return &m_View; }
    const D3D12_INPUT_ELEMENT_DESC* InputElements() { return m_InputElementDescs.data(); }
    const D3D_PRIMITIVE_TOPOLOGY Topology() { return D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; }
public:
    Mesh(const ComPtr<ID3D12Device8> device, vector<XMFLOAT3> vertecies)
    {
        m_Device = device;
        
        m_Size = static_cast<UINT>(vertecies.size());

        // Resource確保
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
            .Width = m_Size * sizeof(XMFLOAT3),
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN,
            .SampleDesc = { .Count = 1, .Quality = 0, },
            .Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE,
        };
        AssertOK(m_Device->CreateCommittedResource(&m_HeapProp, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &m_ResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_Resource)));

        // Resourceに書き込み
        void *verteciesMapping;
        AssertOK(m_Resource->Map(0, nullptr, &verteciesMapping));
        std::memcpy(verteciesMapping, vertecies.data(), vertecies.size() * sizeof(XMFLOAT3));
        m_Resource->Unmap(0, nullptr);

        // View & レイアウト
        m_View =
        {
            .BufferLocation = m_Resource->GetGPUVirtualAddress(),
            .SizeInBytes = m_Size * sizeof(XMFLOAT3),
            .StrideInBytes = sizeof(XMFLOAT3),
        };
        m_InputElementDescs =
        {
            D3D12_INPUT_ELEMENT_DESC {
                .SemanticName = "POSITION",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = 0,
                .InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0,
            },
        };
    }
};