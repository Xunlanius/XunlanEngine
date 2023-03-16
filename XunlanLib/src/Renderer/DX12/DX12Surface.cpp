#include "DX12Surface.h"
#include "Platforms/Window.h"
#include "Renderer/DX12/DX12Core.h"

namespace Xunlan::Graphics::DX12
{
    using namespace Microsoft::WRL;

    namespace
    {
        DXGI_FORMAT ToNonSRGBFormat(DXGI_FORMAT format)
        {
            return format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ? DXGI_FORMAT_R8G8B8A8_UNORM : format;
        }
    }

    DX12Surface::DX12Surface(ID windowID) : m_window(windowID) { assert(Window::GetHandle(windowID)); }

    void DX12Surface::CreateSwapChain(Factory* factory, ID3D12CommandQueue* cmdQueue, DXGI_FORMAT format)
    {
        assert(factory && cmdQueue);

        Release();

        if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_allowTearing, sizeof(m_allowTearing))))
        {
            m_presentFlag = DXGI_PRESENT_ALLOW_TEARING;
        }

        m_format = format;

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = Window::GetWidth(m_window);
        swapChainDesc.Height = Window::GetHeight(m_window);
        swapChainDesc.Format = ToNonSRGBFormat(m_format);
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = NUM_BACK_BUFFERS;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags = m_allowTearing > 0 ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        HWND hwnd = (HWND)Window::GetHandle(m_window);
        ComPtr<IDXGISwapChain1> swapChain;
        Check(factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &swapChainDesc, nullptr, nullptr, &swapChain));
        swapChain.As(&m_swapChain);
        swapChain.Reset();

        m_currBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

        for (uint32 i = 0; i < NUM_BACK_BUFFERS; ++i)
        {
            m_renderTargets[i].rtv = Core::GetRTVHeap().Allocate();
        }

        Finalize();
    }
    void DX12Surface::Present()
    {
        Check(m_swapChain->Present(0, m_presentFlag));
        m_currBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
    }
    void DX12Surface::Resize()
    {
        assert(m_swapChain);

        for (uint32 i = 0; i < NUM_BACK_BUFFERS; ++i)
        {
            m_renderTargets[i].resource.Reset();
        }

        uint32 flags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        Check(m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, flags));
        m_currBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

        Finalize();
    }

    void DX12Surface::Finalize()
    {
        // Create RTVs
        for (uint32 i = 0; i < NUM_BACK_BUFFERS; ++i)
        {
            RenderTarget& renderTarget = m_renderTargets[i];
            assert(!renderTarget.resource);

            Check(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTarget.resource)));
            NAME_OBJECT_INDEX(renderTarget.resource, L"Back Buffer", i);

            D3D12_RENDER_TARGET_VIEW_DESC desc = {};
            desc.Format = m_format;
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

            Core::GetDevice().CreateRenderTargetView(renderTarget.resource.Get(), &desc, renderTarget.rtv.handleCPU);
        }

        DXGI_SWAP_CHAIN_DESC desc = {};
        m_swapChain->GetDesc(&desc);
        const uint32 width = desc.BufferDesc.Width;
        const uint32 height = desc.BufferDesc.Height;
        assert(Window::GetWidth(m_window) == width && Window::GetHeight(m_window) == height);

        // 视口是后台缓冲区的一个矩形子区域
        m_viewport.TopLeftX = 0.0f;
        m_viewport.TopLeftY = 0.0f;
        m_viewport.Width = (float)(width);
        m_viewport.Height = (float)(height);
        m_viewport.MinDepth = 0.0f;
        m_viewport.MaxDepth = 1.0f;

        // 裁剪矩形矩形外的像素都将被剔除
        // 前两个为左上角坐标，后两个为右下角坐标
        m_scissorRect.left = 0;
        m_scissorRect.top = 0;
        m_scissorRect.right = width;
        m_scissorRect.bottom = height;
    }
    void DX12Surface::Release()
    {
        for (uint32 i = 0; i < NUM_BACK_BUFFERS; ++i)
        {
            RenderTarget& renderTarget = m_renderTargets[i];
            renderTarget.resource.Reset();
            Core::GetRTVHeap().Free(renderTarget.rtv);
        }

        m_swapChain.Reset();
    }
}