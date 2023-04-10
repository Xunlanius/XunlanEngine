#include "DX12Surface.h"
#include "src/Function/Renderer/WindowSystem.h"
#include "DX12RHI.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    DX12Surface::DX12Surface(HWND hwnd, uint32 width, uint32 height)
        : m_handle(hwnd), m_width(width), m_height(height)
    {
        SetViewport(0, 0, width, height);
    }

    void DX12Surface::CreateSwapChain(Factory* factory, Device& device, ID3D12CommandQueue* cmdQueue, DX12DescriptorHeap& rtvHeap, DXGI_FORMAT format)
    {
        assert(factory && cmdQueue);

        Release();

        Check(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_allowTearing, sizeof(m_allowTearing)));

        m_format = format;

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = m_width;
        swapChainDesc.Height = m_height;
        swapChainDesc.Format = m_format;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = NUM_BACK_BUFFERS;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        ComPtr<IDXGISwapChain1> swapChain;
        Check(factory->CreateSwapChainForHwnd(cmdQueue, m_handle, &swapChainDesc, nullptr, nullptr, &swapChain));
        swapChain.As(&m_swapChain);
        swapChain.Reset();

        m_currBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

        for (DescriptorHandle& rtv : m_rtvs)
        {
            rtv = rtvHeap.Allocate();
        }

        CreateRTVs(device);
    }
    void DX12Surface::Present()
    {
        Check(m_swapChain->Present(0, m_allowTearing ? DXGI_PRESENT_ALLOW_TEARING : 0));
        m_currBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
    }
    void DX12Surface::SetViewport(uint32 x, uint32 y, uint32 width, uint32 height)
    {
        m_viewport.TopLeftX = (float)x;
        m_viewport.TopLeftY = (float)y;
        m_viewport.Width = (float)(width == 0 ? m_width : width);
        m_viewport.Height = (float)(height == 0 ? m_height : height);
        m_viewport.MinDepth = 0.0f;
        m_viewport.MaxDepth = 1.0f;

        m_scissorRect.left = x;
        m_scissorRect.top = y;
        m_scissorRect.right = width == 0 ? m_width : width;
        m_scissorRect.bottom = height == 0 ? m_height : height;
    }
    void DX12Surface::Resize(uint32 width, uint32 height)
    {
        assert(m_swapChain);

        m_width = width;
        m_height = height;
        SetViewport(0, 0, width, height);

        Check(m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0));
        m_currBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

        CreateRTVs(DX12RHI::Instance().GetDevice());
    }

    ComPtr<ID3D12Resource> DX12Surface::GetBackBuffer() const
    {
        ComPtr<ID3D12Resource> backBuffer;
        Check(m_swapChain->GetBuffer(m_currBackBufferIndex, IID_PPV_ARGS(&backBuffer)));
        return backBuffer;
    }

    void DX12Surface::CreateRTVs(Device& device)
    {
        // Create RTVs
        for (uint32 i = 0; i < NUM_BACK_BUFFERS; ++i)
        {
            ComPtr<ID3D12Resource> backBuffer;
            Check(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
            NAME_OBJECT_INDEX(backBuffer, L"Back Buffer", i);

            D3D12_RENDER_TARGET_VIEW_DESC desc = {};
            desc.Format = m_format;
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

            device.CreateRenderTargetView(backBuffer.Get(), &desc, m_rtvs[i].handleCPU);
        }
    }
    void DX12Surface::Release()
    {
        if (!m_swapChain) return;

        DX12RHI& rhi = DX12RHI::Instance();

        for (DescriptorHandle& rtv : m_rtvs)
        {
            rhi.GetRTVHeap().Free(rtv);
        }

        m_swapChain.Reset();
    }
}