#pragma once

#include <string>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include <d3d12.h>
#include <DirectXMath.h>
#include "wrl.h"
#pragma comment(lib, "d3d12.lib")

#include "Assertion.h"


class Texture
{
private:
    struct Pixel
    {
        std::byte R;
        std::byte G;
        std::byte B;
        std::byte A;
    };
    Microsoft::WRL::ComPtr<ID3D12Device8> m_Device;
    // D3D12_HEAP_PROPERTIES m_UploadHeapProp = {};
    // D3D12_RESOURCE_DESC m_UploadResourceDesc = {};
    // Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadResource;
    D3D12_HEAP_PROPERTIES m_HeapProp = {};
    D3D12_RESOURCE_DESC m_ResourceDesc = {};
    Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;

public:
    Texture(const Microsoft::WRL::ComPtr<ID3D12Device8> device, std::string file)
    {
        using namespace Microsoft::WRL;

        m_Device = device;
     
        auto [matrix, rect] = LoadFromFile(file);
        Dump(matrix, rect);

        // Resource確保
        m_HeapProp =
        {
            .Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_CUSTOM,
            .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
            .MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_L0,
            .CreationNodeMask = 0,
            .VisibleNodeMask = 0,
        };
        m_ResourceDesc =
        {
            .Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            .Alignment = 0,
            .Width = static_cast<UINT>(rect.Width),
            .Height = static_cast<UINT>(rect.Height),
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = { .Count = 1, .Quality = 0, },
            .Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE,
        };
        AssertOK(m_Device->CreateCommittedResource(&m_HeapProp, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &m_ResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_Resource)));
        // Resourceに書き込み
        AssertOK(m_Resource->WriteToSubresource(0, nullptr, matrix.data(), rect.Width * sizeof(*matrix.data()), matrix.size() * sizeof(*matrix.data())));

        
    }

    static std::tuple<std::vector<Pixel>, Gdiplus::Rect> LoadFromFile(std::string file)
    {
        using std::vector;
        using namespace Gdiplus;
        using namespace DirectX;

        ULONG_PTR gdipulsToken;
        GdiplusStartupInput gdipulsInput;
        GdiplusStartup(&gdipulsToken, &gdipulsInput, nullptr);

        auto bitmap = Bitmap::FromFile(MultiByteToWide(file.data()).data());
        auto rect = Gdiplus::Rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
        BitmapData data = {};
        bitmap->LockBits(&rect, ImageLockMode::ImageLockModeRead, bitmap->GetPixelFormat(), &data);
        auto buffer = reinterpret_cast<Pixel*>(data.Scan0);
        std::vector<Pixel> matrix;
        for (size_t i = 0; i < data.Height * data.Width; i++)
        {
            matrix.push_back(buffer[i]);
        }
        
        bitmap->UnlockBits(&data);

        GdiplusShutdown(gdipulsToken);
        return std::tuple(matrix, rect);
    }

    static void Dump(std::vector<Pixel> matrix, Gdiplus::Rect rect)
    {
        using namespace std;

        for (size_t i = 0; i < rect.Height; i += 32)
        {
            for (size_t j = 0; j < rect.Width; j += 16)
            {
                cout << (to_integer<unsigned char>(matrix[i * rect.Width + j].A) > 0x80 ? "#" : " ");
            }
            cout << endl;
        }
    }
};