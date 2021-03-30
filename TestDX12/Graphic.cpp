#include <iostream>
#include <vector>
#include <array>
#include <tuple>
#include <chrono>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "wrl.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
using namespace Microsoft::WRL;
using namespace DirectX;

#include "Graphic.h"

Graphic::Graphic(HWND window)
{
    m_WindowHandle = window;
    AssertOK(CoInitialize(NULL));

    ComPtr<ID3D12Debug> debug;
    AssertOK(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
    debug->EnableDebugLayer();
    ComPtr<ID3D12Debug1> debugEx;
    AssertOK(debug->QueryInterface(IID_PPV_ARGS(&debugEx)));
    debugEx->SetEnableGPUBasedValidation(true);
    
    AssertOK(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_Factory)));
    for (auto adapter : GetAdapters(m_Factory.Get()))
    {
        DXGI_ADAPTER_DESC adapterDesc;
        AssertOK(adapter->GetDesc(&adapterDesc));
        std::cout << WideToMultiByte(adapterDesc.Description).data() << std::endl;
    }

    AssertOK(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device)));

    AssertOK(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));
    AssertOK(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
    AssertOK(m_CommandList->Close());
    D3D12_COMMAND_QUEUE_DESC commandQueueCreateDesc = {};
    commandQueueCreateDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
    commandQueueCreateDesc.NodeMask = 0;
    commandQueueCreateDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    commandQueueCreateDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
    AssertOK(m_Device->CreateCommandQueue(&commandQueueCreateDesc, IID_PPV_ARGS(&m_CommandQueue)));

    RECT windowRect = {};
    GetClientRect(m_WindowHandle, &windowRect);
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = windowRect.right - windowRect.left;
    swapChainDesc.Height = windowRect.bottom - windowRect.top;
    swapChainDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = false;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    ComPtr<IDXGISwapChain1> swapChain1;
    AssertOK(m_Factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_WindowHandle, &swapChainDesc, nullptr, nullptr, &swapChain1));
    AssertOK(swapChain1.As(&m_SwapChain));

    D3D12_DESCRIPTOR_HEAP_DESC swapChainRenderTargetsHeaDesc = {};
    swapChainRenderTargetsHeaDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    swapChainRenderTargetsHeaDesc.NumDescriptors = 2;
    swapChainRenderTargetsHeaDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    swapChainRenderTargetsHeaDesc.NodeMask = 0;
    AssertOK(m_Device->CreateDescriptorHeap(&swapChainRenderTargetsHeaDesc, IID_PPV_ARGS(&m_SwapChainRenderTargetsHeap)));

    D3D12_CPU_DESCRIPTOR_HANDLE swapChainRenderTargetDescriptorHandle = m_SwapChainRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < swapChainDesc.BufferCount; i++)
    {
        auto currentHandle = swapChainRenderTargetDescriptorHandle;
        swapChainRenderTargetDescriptorHandle.ptr += m_Device->GetDescriptorHandleIncrementSize(swapChainRenderTargetsHeaDesc.Type);
        ComPtr<ID3D12Resource> resource;
        AssertOK(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&resource)));
        m_Device->CreateRenderTargetView(resource.Get(), nullptr, currentHandle);
        if(nullptr == resource.Get()) throw std::exception("Failed create RenderTargetView");
        m_SwapChainRenderTargets.push_back({ currentHandle, resource });
    }
    
    AssertOK(m_Device->CreateFence(~0, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));

    m_Mesh.reset(new Mesh(m_Device, std::vector<XMFLOAT3>(
    {
        { -1.0f, -1.0f, +0.0f },
        { -1.0f, +1.0f, +0.0f },
        { +1.0f, -1.0f, +0.0f },
    })));
    
    m_VertexShader.reset(new Shader("vs_5_0", "float4 main(float4 pos : POSITION) : SV_POSITION { return pos; }"));
    m_PixelShader.reset(new Shader("ps_5_0", "float4 main(float4 pos : SV_POSITION) : SV_TARGET { return float4(1,1,1,1); }"));
}

Graphic::~Graphic()
{
    CoUninitialize();
}

UINT64 Graphic::Rendring()
{
    auto frameNumberForFence = m_Fence->GetCompletedValue() + 1;
    AssertOK(m_CommandAllocator->Reset());
    AssertOK(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));

    const auto currentSwapChainRenderTarget = m_SwapChainRenderTargets[m_SwapChain->GetCurrentBackBufferIndex()];
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition = {};
    barrier.Transition.pResource = currentSwapChainRenderTarget.Resource.Get();
    barrier.Transition.Subresource = 0;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_CommandList->ResourceBarrier(1, &barrier);

    m_CommandList->OMSetRenderTargets(1, std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 1>({currentSwapChainRenderTarget.Handle}).data(), false, nullptr);
    m_CommandList->ClearRenderTargetView(currentSwapChainRenderTarget.Handle, std::array<FLOAT, 4>({ 1.0f * (frameNumberForFence % 256) / 256.0f, 0.0f, 0.0f, 1.0f }).data(), 0, nullptr);

    RECT windowRect = {};
    GetClientRect(m_WindowHandle, &windowRect);
    D3D12_VIEWPORT viewPort = {};
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = windowRect.right - windowRect.left;
    viewPort.Height = windowRect.bottom - windowRect.top;
    viewPort.MinDepth = 1.0f;
    viewPort.MaxDepth = 1.1f;
    m_CommandList->RSSetViewports(1, &viewPort);
    D3D12_RECT scissor = {};
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = windowRect.right - windowRect.left;
    scissor.bottom = windowRect.bottom - windowRect.top;
    m_CommandList->RSSetScissorRects(1, &scissor);

    m_CommandList->IASetVertexBuffers(0, 1, m_Mesh->View());
    m_CommandList->IASetPrimitiveTopology(m_Mesh->Topology());

    

    ComPtr<ID3D12RootSignature> rootSignature;
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    ComPtr<ID3DBlob> rootSignatureBlob, rootSignatureSerializerError;
    try
    {
        AssertOK(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSignatureBlob, &rootSignatureSerializerError));
    }
    catch(std::exception ex)
    {
        if(rootSignatureSerializerError.Get() != nullptr) throw std::exception(reinterpret_cast<char *>(rootSignatureSerializerError->GetBufferPointer()));
        throw;
    }
    AssertOK(m_Device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

    ComPtr<ID3D12PipelineState> piplineState;
    piplineStates.push_back(piplineState); // TODO: 適当なタイミングで解放
    
    D3D12_GRAPHICS_PIPELINE_STATE_DESC piplineStateDesc = {};
    piplineStateDesc.pRootSignature = rootSignature.Get();
    piplineStateDesc.VS = { m_VertexShader->GetBufferPointer(), m_VertexShader->GetBufferSize() };
    piplineStateDesc.PS = { m_PixelShader->GetBufferPointer(), m_PixelShader->GetBufferSize() };
    piplineStateDesc.DS = {};
    piplineStateDesc.HS = {};
    piplineStateDesc.GS = {};
    piplineStateDesc.StreamOutput = {};
    piplineStateDesc.BlendState = {};
    piplineStateDesc.BlendState.AlphaToCoverageEnable = false;
    piplineStateDesc.BlendState.IndependentBlendEnable = false;
    piplineStateDesc.BlendState.RenderTarget[0].BlendEnable = false;
    piplineStateDesc.BlendState.RenderTarget[0].LogicOpEnable = false;
    piplineStateDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE::D3D12_COLOR_WRITE_ENABLE_ALL;
    piplineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    piplineStateDesc.RasterizerState = {};
    piplineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
    piplineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
    piplineStateDesc.RasterizerState.FrontCounterClockwise = true;
    piplineStateDesc.RasterizerState.DepthClipEnable = true;
    piplineStateDesc.DepthStencilState = {};
    piplineStateDesc.InputLayout = {};
    piplineStateDesc.InputLayout.pInputElementDescs = m_Mesh->InputElements();
    piplineStateDesc.InputLayout.NumElements = 1;
    piplineStateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE::D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    piplineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    piplineStateDesc.NumRenderTargets = 1;
    piplineStateDesc.RTVFormats[0] = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    piplineStateDesc.DSVFormat = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
    piplineStateDesc.SampleDesc = { 1, 0 };
    piplineStateDesc.NodeMask = 0;
    piplineStateDesc.CachedPSO = {};
    piplineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAGS::D3D12_PIPELINE_STATE_FLAG_NONE;
    AssertOK(m_Device->CreateGraphicsPipelineState(&piplineStateDesc, IID_PPV_ARGS(&piplineState)));
    m_CommandList->SetPipelineState(piplineState.Get());
    m_CommandList->SetGraphicsRootSignature(rootSignature.Get());
    m_CommandList->DrawInstanced(m_Mesh->Size(), 1, 0, 0);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT;
    m_CommandList->ResourceBarrier(1, &barrier);
    
    AssertOK(m_CommandList->Close());
    m_CommandQueue->ExecuteCommandLists(1, std::vector<ID3D12CommandList*>({ m_CommandList.Get() }).data());

    AssertOK(m_SwapChain->Present(DXGI_SWAP_EFFECT_SEQUENTIAL, 0 /* DXGI_PRESENT */));
    AssertOK(m_CommandQueue->Signal(m_Fence.Get(), frameNumberForFence));

    if (m_Fence->GetCompletedValue() != frameNumberForFence)
    {
        auto renderFinishEvent = CreateEvent(nullptr, false, false, nullptr);
        m_Fence->SetEventOnCompletion(frameNumberForFence, renderFinishEvent);
        WaitForSingleObject(renderFinishEvent, INFINITE);
        CloseHandle(renderFinishEvent);
    }
    return frameNumberForFence;
}

std::vector<ComPtr<IDXGIAdapter>> Graphic::GetAdapters(IDXGIFactory7 *factory)
{
    auto adapters = std::vector<ComPtr<IDXGIAdapter>>();
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
