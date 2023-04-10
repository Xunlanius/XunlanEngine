#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/Texture.h"
#include "DX12Resource.h"

namespace Xunlan::DX12
{
    class DX12RenderTarget final : public RenderTarget
    {
    public:

        DX12RenderTarget(uint32 width, uint32 height, RenderTargetUsage usage);
        virtual ~DX12RenderTarget();

    public:

        static Ref<DX12RenderTarget> Create(uint32 width, uint32 height, RenderTargetUsage usage);

        ID3D12Resource* GetRT() const { return m_rtTexture.Get(); }
        ID3D12Resource* GetDS() const { return m_dsTexture.Get(); }
        DescriptorHandle GetRTV() const { return m_rtv; }
        DescriptorHandle GetDSV() const { return m_dsv; }
        DescriptorHandle GetRenderTargetSRV() const { assert(m_usage == RenderTargetUsage::DEFAULT); return m_rtSRV; }
        DescriptorHandle GetDepthStencilSRV() const { assert(m_usage == RenderTargetUsage::SHADOW_MAP); return m_dsSRV; }
        DXGI_FORMAT GetRTFormat() const { return m_currRTFormat; }
        DXGI_FORMAT GetDSFormat() const { return m_currDSFormat; }

        virtual void Resize(uint32 width, uint32 height) override;
        virtual uint32 GetRenderTargetIndex() const override { return m_rtSRV.index; }
        virtual uint32 GetDepthStencilIndex() const override { return m_dsSRV.index; }

        static constexpr D3D12_RESOURCE_STATES RT_INIT_STATE = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        static constexpr D3D12_RESOURCE_STATES DS_INIT_STATE = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

    private:

        void Create(uint32 width, uint32 height);
        void Clear();

        void CreateRenderTarget(uint32 width, uint32 height);
        void CreateDepthStencil(uint32 width, uint32 height);
        void CreateDepth(uint32 width, uint32 height);

        void ClearRenderTarget();
        void ClearDepthStencil();

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> m_rtTexture;
        DescriptorHandle m_rtv = {};
        DescriptorHandle m_rtSRV = {};

        Microsoft::WRL::ComPtr<ID3D12Resource> m_dsTexture;
        DescriptorHandle m_dsv = {};
        DescriptorHandle m_dsSRV = {};

        DXGI_FORMAT m_currRTFormat = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT m_currDSFormat = DXGI_FORMAT_UNKNOWN;
    };
}