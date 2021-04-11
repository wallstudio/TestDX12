#pragma once

#include <vector>
#include <fstream>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "wrl.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Assertion.h"


class Shader
{
private:
    Microsoft::WRL::ComPtr<ID3DBlob> m_Shader;
public:
    const LPVOID GetBufferPointer() { return m_Shader->GetBufferPointer(); } 
    const SIZE_T GetBufferSize() { return m_Shader->GetBufferSize(); }
public:
    Shader(std::string targetModel, std::string entrypoint, std::ifstream file)
    {
        using namespace std;
        using namespace Microsoft::WRL;

        auto buff = stringstream();
        buff << file.rdbuf();
        const auto code = string(buff.str());

        ComPtr<ID3DBlob> error;
        const auto result = D3DCompile(
            code.data(), code.size(),
            entrypoint.data(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entrypoint.data(), targetModel.data(), D3DCOMPILE_DEBUG|D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_Shader, &error);
        if(error != nullptr)
        {
            auto message = string(reinterpret_cast<char *>(error->GetBufferPointer()));
            cout << message << endl;
            throw exception(message.data());
        }
        AssertOK(result);
    }
};