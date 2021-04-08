#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "SwapChainRenderTargets.h"
#include "Texture.h"

class Graphic
{
private:
    HWND m_WindowHandle;
    Microsoft::WRL::ComPtr<ID3D12Device8> m_Device;
    Microsoft::WRL::ComPtr<IDXGIFactory7> m_Factory;

    std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_CommandAllocators;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    
    std::shared_ptr<SwapChainRenderTargets> m_SwapChainRenderTarget;
    std::shared_ptr<Mesh> m_Mesh;
    std::shared_ptr<Shader> m_VertexShader;
    std::shared_ptr<Shader> m_PixelShader;
    std::shared_ptr<Texture> m_Texture;

    UINT64 m_FrameIndex = ~0;

public:
    Graphic(HWND window)
    {
        using namespace std;
        using namespace Microsoft::WRL;

        m_WindowHandle = window;
        AssertOK(CoInitialize(NULL));
        EnableDebug();
        
        // Device
        AssertOK(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_Factory)));
        AssertOK(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device)));

        // Command
        D3D12_COMMAND_QUEUE_DESC commandQueueCreateDesc =
        {
            .Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            .Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0,
        };
        AssertOK(m_Device->CreateCommandQueue(&commandQueueCreateDesc, IID_PPV_ARGS(&m_CommandQueue)));
        for (size_t i = 0; i < 2; i++)
        {
            ComPtr<ID3D12CommandAllocator> commandAllocator;
            AssertOK(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
            m_CommandAllocators.push_back(commandAllocator);
        }
        
        // Resources
        m_SwapChainRenderTarget.reset(new SwapChainRenderTargets(m_WindowHandle, m_Device, m_Factory, m_CommandQueue));
                m_Mesh.reset(new Mesh(
            m_Device,
            vector<VERTEX>
            {
                { .Postion { -0.8f, -0.8f, +0.0f, +1.0f }, .Texcord { -0.8f, -0.8f, }, .Color { 1.0f, 1.0f, 1.0f, 1.0f}, },
                { .Postion { -0.8f, +0.8f, +0.0f, +1.0f }, .Texcord { -0.8f, +0.8f, }, .Color { 1.0f, 0.0f, 1.0f, 1.0f}, },
                { .Postion { +0.8f, -0.8f, +0.0f, +1.0f }, .Texcord { +0.8f, -0.8f, }, .Color { 1.0f, 1.0f, 0.0f, 1.0f}, },
                { .Postion { +0.8f, +0.8f, +0.0f, +1.0f }, .Texcord { +0.8f, +0.8f, }, .Color { 1.0f, 1.0f, 1.0f, 1.0f}, },
            },
            vector<USHORT>{ 0, 1, 2, 2, 1, 3, }));
        m_VertexShader.reset(new Shader("vs_5_0", "vs", ifstream("Shader.hlsl")));
        m_PixelShader.reset(new Shader("ps_5_0", "ps", ifstream("Shader.hlsl")));
        m_Texture.reset(new Texture(m_Device, "Texture.png"));
    }

    ~Graphic()
    {
        CoUninitialize();
    }

    UINT64 Rendring()
    {
        using namespace std;
        using namespace Microsoft::WRL;
        using namespace DirectX;

        m_FrameIndex++;

        auto commandAllocator = m_CommandAllocators[m_SwapChainRenderTarget->CurrentIndex()];
        ComPtr<ID3D12GraphicsCommandList> commandList;
        AssertOK(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
        
        // IA
        commandList->IASetVertexBuffers(0, 1, m_Mesh->VertexView());
        commandList->IASetIndexBuffer(m_Mesh->IndexView());
        commandList->IASetPrimitiveTopology(m_Mesh->Topology());
        
        // RS
        RECT windowRect = {};
        GetClientRect(m_WindowHandle, &windowRect);
        D3D12_VIEWPORT viewPort = {};
        viewPort.TopLeftX = 0;
        viewPort.TopLeftY = 0;
        viewPort.Width = static_cast<FLOAT>(windowRect.right - windowRect.left);
        viewPort.Height = static_cast<FLOAT>(windowRect.bottom - windowRect.top);
        viewPort.MinDepth = 1.0f;
        viewPort.MaxDepth = 1.1f;
        commandList->RSSetViewports(1, &viewPort);
        D3D12_RECT scissor = {
            .left = 0,
            .top = 0,
            .right = windowRect.right - windowRect.left,
            .bottom = windowRect.bottom - windowRect.top,
        };
        commandList->RSSetScissorRects(1, &scissor);

        // Signatures
        ComPtr<ID3D12RootSignature> rootSignature;
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        ComPtr<ID3DBlob> rootSignatureBlob, rootSignatureSerializerError;
        try
        {
            AssertOK(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSignatureBlob, &rootSignatureSerializerError));
        }
        catch(exception ex)
        {
            if(rootSignatureSerializerError.Get() != nullptr) throw exception(reinterpret_cast<char *>(rootSignatureSerializerError->GetBufferPointer()));
            throw;
        }
        AssertOK(m_Device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
        commandList->SetGraphicsRootSignature(rootSignature.Get());

        // StateObject
        ID3D12PipelineState* piplineState;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC piplineStateDesc =
        {
            .pRootSignature = rootSignature.Get(),
            .VS = { m_VertexShader->GetBufferPointer(), m_VertexShader->GetBufferSize() },
            .PS = { m_PixelShader->GetBufferPointer(), m_PixelShader->GetBufferSize() },
            .DS = {},
            .HS = {},
            .GS = {},
            .StreamOutput = {},
            .BlendState =
            {
                .AlphaToCoverageEnable = false,
                .IndependentBlendEnable = false,
                .RenderTarget =
                {
                    D3D12_RENDER_TARGET_BLEND_DESC
                    {
                        .BlendEnable = false,
                        .LogicOpEnable = false,
                        .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE::D3D12_COLOR_WRITE_ENABLE_ALL,
                    }
                },
            },
            .SampleMask = D3D12_DEFAULT_SAMPLE_MASK,
            .RasterizerState = 
            {
                .FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID,
                .CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE,
                .FrontCounterClockwise = true,
                .DepthClipEnable = true,
            },
            .DepthStencilState = {},
            .InputLayout = { .pInputElementDescs = m_Mesh->InputElements()->data(), .NumElements = static_cast<UINT>(m_Mesh->InputElements()->size()) },
            .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE::D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
            .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .NumRenderTargets = 1,
            .RTVFormats = { DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM },
            .DSVFormat = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN,
            .SampleDesc = { .Count = 1, .Quality = 0 },
            .NodeMask = 0,
            .CachedPSO = {},
            .Flags = D3D12_PIPELINE_STATE_FLAGS::D3D12_PIPELINE_STATE_FLAG_NONE,
        };
        AssertOK(m_Device->CreateGraphicsPipelineState(&piplineStateDesc, IID_PPV_ARGS(&piplineState)));
        commandList->SetPipelineState(piplineState);

        // Rendering
        m_SwapChainRenderTarget->ChangeBarrier(commandList, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto color = XMFLOAT4(1.0f * (m_FrameIndex % 256) / 256.0f, 0.0f, 0.0f, 1.0f);
        commandList->ClearRenderTargetView(m_SwapChainRenderTarget->DiscriptorHandle(), reinterpret_cast<FLOAT*>(&color), 0, nullptr);
        auto renderTargets = vector { m_SwapChainRenderTarget->DiscriptorHandle() };
        commandList->OMSetRenderTargets(1, renderTargets.data(), false, nullptr);
        commandList->DrawIndexedInstanced(m_Mesh->Size(), 1, 0, 0, 0);
        m_SwapChainRenderTarget->ChangeBarrier(commandList, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT);
        
        // Execute
        AssertOK(commandList->Close());
        m_CommandQueue->ExecuteCommandLists(1, vector<ID3D12CommandList*>({ commandList.Get() }).data());
        AssertOK(m_SwapChainRenderTarget->SwapChain()->Present(DXGI_SWAP_EFFECT_SEQUENTIAL, 0 /* DXGI_PRESENT */));

        // Wait
        ComPtr<ID3D12Fence> fence;
        AssertOK(m_Device->CreateFence(0, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
        AssertOK(m_CommandQueue->Signal(fence.Get(), 1));
        if (fence->GetCompletedValue() != 0)
        {
            auto renderFinishEvent = CreateEvent(nullptr, false, false, nullptr);
            fence->SetEventOnCompletion(1, renderFinishEvent);
            WaitForSingleObject(renderFinishEvent, INFINITE);
            CloseHandle(renderFinishEvent);
        }
        return m_FrameIndex;
    }

private:
    void EnableDebug()
    {
        using namespace Microsoft::WRL;

        ComPtr<ID3D12Debug1> debug;
        AssertOK(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
        debug->EnableDebugLayer();
        debug->SetEnableGPUBasedValidation(true);
    }

    static std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>> GetAdapters(IDXGIFactory7 *factory)
    {
        using namespace std;
        using namespace Microsoft::WRL;

        auto adapters = vector<ComPtr<IDXGIAdapter>>();
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
};