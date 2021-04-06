#pragma once

#include <vector>
#include <array>
using namespace std;

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "wrl.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
using namespace Microsoft::WRL;
using namespace DirectX;

#include "StringUtility.h"


class Shader
{
private:
    ComPtr<ID3DBlob> m_Shader;
public:
    const LPVOID GetBufferPointer() { return m_Shader->GetBufferPointer(); } 
    const SIZE_T GetBufferSize() { return m_Shader->GetBufferSize(); }
public:
    Shader(string type, string code)
    {
        ComPtr<ID3DBlob> error;
        const auto result = D3DCompile(
            code.data(), code.size(), "VS", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "main", type.data(), D3DCOMPILE_DEBUG|D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_Shader, &error);
        if(error != nullptr)
        {
            throw exception(reinterpret_cast<char *>(error->GetBufferPointer()));
        }
        AssertOK(result);
    }
};