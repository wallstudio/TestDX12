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

public:
    Texture(const Microsoft::WRL::ComPtr<ID3D12Device8> device, std::string file)
    {
        m_Device = device;
     
        auto matrix = LoadFromFile(file);
        Dump(matrix);
    }

    static std::vector<std::vector<Pixel>> LoadFromFile(std::string file)
    {
        using std::vector;
        using namespace Gdiplus;
        using namespace DirectX;

        auto matrix = vector<vector<Pixel>>();
        ULONG_PTR gdipulsToken;
        GdiplusStartupInput gdipulsInput;
        GdiplusStartup(&gdipulsToken, &gdipulsInput, nullptr);
        {
            auto bitmap = Bitmap::FromFile(MultiByteToWide(file.data()).data());
            auto rect = Gdiplus::Rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
            BitmapData data = {};
            bitmap->LockBits(&rect, ImageLockMode::ImageLockModeRead, bitmap->GetPixelFormat(), &data);
            auto buffer = reinterpret_cast<Pixel*>(data.Scan0);
            for (size_t i = 0; i < data.Height; i++)
            {
                auto row = vector<Pixel>();
                for (size_t j = 0; j < data.Width; j++)
                {
                    row.push_back(buffer[data.Width * i + j]);
                }
                matrix.push_back(row);
            }
            bitmap->UnlockBits(&data);
        }
        GdiplusShutdown(gdipulsToken);
        return matrix;
    }

    static void Dump(std::vector<std::vector<Pixel>> matrix)
    {
        using namespace std;
        
        for (size_t i = 0; i < matrix.size(); i += 32)
        {
            for (size_t j = 0; j < matrix[i].size(); j += 16)
            {
                cout << (to_integer<unsigned char>(matrix[i][j].A) > 0x80 ? "#" : " ");
            }
            cout << endl;
        }
    }
};