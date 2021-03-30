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
    ComPtr<ID3D12Resource> m_Resource;
    UINT m_Size;
    D3D12_HEAP_PROPERTIES m_HeapProp = {};
    D3D12_RESOURCE_DESC m_ResourceDesc = {};
    D3D12_VERTEX_BUFFER_VIEW m_View = {};
public:
    UINT Size() { return m_Size; }
    D3D12_VERTEX_BUFFER_VIEW *const View() { return &m_View; }
    D3D_PRIMITIVE_TOPOLOGY Topology() { return D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; }
public:
    Mesh(const ComPtr<ID3D12Device8> device, vector<XMFLOAT3> vertecies)
    {
        m_Device = device;
        
        m_Size = static_cast<UINT>(vertecies.size());

        m_HeapProp.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
        m_HeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        m_HeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
        m_HeapProp.CreationNodeMask = 0;
        m_HeapProp.VisibleNodeMask = 0;

        m_ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
        m_ResourceDesc.Alignment = 0;
        m_ResourceDesc.Width = m_Size * sizeof(XMFLOAT3);
        m_ResourceDesc.Height = 1;
        m_ResourceDesc.DepthOrArraySize = 1;
        m_ResourceDesc.MipLevels = 1;
        m_ResourceDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
        m_ResourceDesc.SampleDesc = {};
        m_ResourceDesc.SampleDesc.Count = 1;
        m_ResourceDesc.SampleDesc.Quality = 0;
        m_ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        m_ResourceDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

        AssertOK(m_Device->CreateCommittedResource(&m_HeapProp, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &m_ResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_Resource)));
        void *verteciesMapping;
        AssertOK(m_Resource->Map(0, nullptr, &verteciesMapping));
        memcpy(verteciesMapping, vertecies.data(), m_Size * sizeof(XMFLOAT3));
        m_Resource->Unmap(0, nullptr);
        
        m_View.BufferLocation = m_Resource->GetGPUVirtualAddress();
        m_View.SizeInBytes = m_Size * sizeof(XMFLOAT3);
        m_View.StrideInBytes = sizeof(XMFLOAT3);
    }
};