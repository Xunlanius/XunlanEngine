#pragma once

#include "src/Function/Renderer/RenderCommon.h"
#include "DX12Common.h"
#include "DX12Resource.h"
#include <memory>

namespace Xunlan::DX12
{
    class DX12Surface final
    {
    public:

        explicit DX12Surface(HWND hwnd, uint32 width, uint32 height);
        DISABLE_COPY(DX12Surface)
        DISABLE_MOVE(DX12Surface)
        ~DX12Surface() { Release(); }

    public:

        void CreateSwapChain(Factory* factory, Device& device, ID3D12CommandQueue* cmdQueue, DX12DescriptorHeap& rtvHeap, DXGI_FORMAT format = DEFAULT_RENDER_TARGET_FORMAT);

        void Present();

        uint32 GetWidth() const { return m_width; }
        uint32 GetHeight() const { return m_height; }
        const D3D12_VIEWPORT& GetViewport() const { return m_viewport; }
        const D3D12_RECT& GetScissorRect() const { return m_scissorRect; }
        DXGI_FORMAT GetFormat() const { return m_format; }

        void SetViewport(uint32 x, uint32 y, uint32 width, uint32 height);
        void Resize(uint32 width, uint32 height);

        Microsoft::WRL::ComPtr<ID3D12Resource> GetBackBuffer() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_rtvs[m_currBackBufferIndex].handleCPU; }

        static constexpr uint32 NUM_BACK_BUFFERS = 3;

    private:

        void CreateRTVs(Device& device);
        void Release();

        HWND m_handle;

        Microsoft::WRL::ComPtr<SwapChain> m_swapChain;

        static constexpr DXGI_FORMAT DEFAULT_RENDER_TARGET_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
        DXGI_FORMAT m_format = DEFAULT_RENDER_TARGET_FORMAT;

        uint32 m_currBackBufferIndex = 0;
        DescriptorHandle m_rtvs[NUM_BACK_BUFFERS] = {};

        uint32 m_width;
        uint32 m_height;

        D3D12_VIEWPORT m_viewport = {};
        D3D12_RECT m_scissorRect = {};

        uint32 m_allowTearing = 0;
    };
}