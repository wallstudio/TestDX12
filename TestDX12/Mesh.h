#pragma once

#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "wrl.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Assertion.h"

struct VERTEX
{
    DirectX::XMFLOAT4 Postion;
    DirectX::XMFLOAT2 Texcord;
    DirectX::XMFLOAT4 Normal;
    DirectX::XMFLOAT3 Tangent;
    DirectX::XMFLOAT4 Color;
};

class Mesh
{
private:
    Microsoft::WRL::ComPtr<ID3D12Device8> m_Device;
    UINT m_Size;
    D3D12_HEAP_PROPERTIES m_HeapProp = {};
    D3D12_RESOURCE_DESC m_VertexResourceDesc = {};
    D3D12_RESOURCE_DESC m_IndexResourceDesc = {};
    Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexResource;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexResource;
    D3D12_VERTEX_BUFFER_VIEW m_VertexView = {};
    D3D12_INDEX_BUFFER_VIEW m_IndexView = {};
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputElementDescs;

public:
    const UINT Size() { return m_Size; }
    const D3D12_VERTEX_BUFFER_VIEW *const VertexView() { return &m_VertexView; }
    const D3D12_INDEX_BUFFER_VIEW *const IndexView() { return &m_IndexView; }
    const std::vector<D3D12_INPUT_ELEMENT_DESC> *const InputElements() { return &m_InputElementDescs; }
    const D3D_PRIMITIVE_TOPOLOGY Topology() { return D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; }

public:
    Mesh(const Microsoft::WRL::ComPtr<ID3D12Device8> device, std::vector<VERTEX> vertecies, std::vector<USHORT> indecies)
    {
        m_Device = device;
        
        m_Size = static_cast<UINT>(indecies.size());

        // Resource確保
        m_HeapProp =
        {
            .Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD,
            .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            .MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN,
            .CreationNodeMask = 0,
            .VisibleNodeMask = 0,
        };
        m_VertexResourceDesc =
        {
            .Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = vertecies.size() * sizeof(*vertecies.data()),
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN,
            .SampleDesc = { .Count = 1, .Quality = 0, },
            .Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE,
        };
        AssertOK(m_Device->CreateCommittedResource(&m_HeapProp, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &m_VertexResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexResource)));
        m_IndexResourceDesc =
        {
            .Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = indecies.size() * sizeof(*indecies.data()),
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN,
            .SampleDesc = { .Count = 1, .Quality = 0, },
            .Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE,
        };
        AssertOK(m_Device->CreateCommittedResource(&m_HeapProp, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &m_IndexResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_IndexResource)));
        

        // Resourceに書き込み
        void *vertexMapping;
        AssertOK(m_VertexResource->Map(0, nullptr, &vertexMapping));
        std::memcpy(vertexMapping, vertecies.data(), vertecies.size() * sizeof(*vertecies.data()));
        m_VertexResource->Unmap(0, nullptr);
        void *indexMapping;
        AssertOK(m_IndexResource->Map(0, nullptr, &indexMapping));
        std::memcpy(indexMapping, indecies.data(), indecies.size() * sizeof(*indecies.data()));
        m_IndexResource->Unmap(0, nullptr);

        // View
        m_VertexView =
        {
            .BufferLocation = m_VertexResource->GetGPUVirtualAddress(),
            .SizeInBytes = static_cast<UINT>(vertecies.size()) * sizeof(*vertecies.data()),
            .StrideInBytes = sizeof(*vertecies.data()),
        };
        m_IndexView =
        {
            .BufferLocation = m_IndexResource->GetGPUVirtualAddress(),
            .SizeInBytes = static_cast<UINT>(indecies.size()) * sizeof(*indecies.data()),
            .Format = DXGI_FORMAT::DXGI_FORMAT_R16_UINT,
        };

        // レイアウト
        UINT offset = 0;
        using std::tuple;
        for(auto [semantic, format, size] :
        {
            tuple("POSITION", DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(VERTEX::Postion)),
            tuple("TEXCOORD", DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, sizeof(VERTEX::Texcord)),
            tuple("NORMAL", DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, sizeof(VERTEX::Normal)),
            tuple("TANGENT", DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, sizeof(VERTEX::Tangent)),
            tuple("COLOR", DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(VERTEX::Color)),
        })
        {
            m_InputElementDescs.push_back(D3D12_INPUT_ELEMENT_DESC
            {
                .SemanticName = semantic,
                .SemanticIndex = 0,
                .Format = format,
                .InputSlot = 0,
                .AlignedByteOffset = offset,
                .InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0,
            });
            offset += static_cast<UINT>(size);
        }
    }
};