#pragma once

#include "Renderer/DX12/DX12Common.h"

namespace Xunlan::Graphics::DX12
{
    class DX12Surface final
    {
    public:

        explicit DX12Surface(ID windowID);
        DX12Surface(const DX12Surface& rhs) = delete;
        DX12Surface& operator =(const DX12Surface& rhs) = delete;
        ~DX12Surface() { Release(); }

    public:

        void CreateSwapChain(Factory* factory, ID3D12CommandQueue* cmdQueue, DXGI_FORMAT format = DEFAULT_RENDER_TARGET_FORMAT);

        /// <summary>
        /// Present the back buffer and update the index
        /// </summary>
        void Present();

        void Resize();

        uint32 GetWidth() const { return (uint32)m_viewport.Width; }
        uint32 GetHeight() const { return (uint32)m_viewport.Height; }
        ID3D12Resource* GetBackBuffer() const { return m_renderTargets[m_currBackBufferIndex].resource.Get(); }
        D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_renderTargets[m_currBackBufferIndex].rtv.handleCPU; }
        const D3D12_VIEWPORT& GetViewport() const { return m_viewport; }
        const D3D12_RECT& GetScissorRect() const { return m_scissorRect; }

        static constexpr uint32 NUM_BACK_BUFFERS = 3;
        static constexpr DXGI_FORMAT DEFAULT_RENDER_TARGET_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    private:

        void Finalize();
        void Release();

        struct RenderTarget
        {
            Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
            DescriptorHandle rtv = {};
        };

        ID m_window = INVALID_ID;

        Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain = nullptr;
        DXGI_FORMAT m_format = DEFAULT_RENDER_TARGET_FORMAT;
        uint32 m_currBackBufferIndex = 0;
        RenderTarget m_renderTargets[NUM_BACK_BUFFERS] = {};

        D3D12_VIEWPORT m_viewport = {};
        D3D12_RECT m_scissorRect = {};

        uint32 m_allowTearing = 0;
        uint32 m_presentFlag = 0;
    };
}