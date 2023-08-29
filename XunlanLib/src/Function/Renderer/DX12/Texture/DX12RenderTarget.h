#pragma once

#include "src/Function/Renderer/Abstract/Texture.h"
#include "../DX12Common.h"
#include "../DX12DescriptorHeap.h"

namespace Xunlan::DX12
{
    class DX12RenderTarget final : public RenderTarget
    {
    public:

        DX12RenderTarget(uint32 width, uint32 height, TextureFormat format);
        virtual ~DX12RenderTarget();

    public:

        ID3D12Resource* GetResource() const { return m_resource.Get(); }
        DescriptorHandle GetRTV() const { return m_rtv; }
        DescriptorHandle GetSRV() const { return m_srv; }
        DXGI_FORMAT GetDXFormat() const { return m_dxFormat; }

        virtual void Resize(uint32 width, uint32 height) override;
        virtual uint32 GetHeapIndex() const override { return m_srv.index; }

        static constexpr D3D12_RESOURCE_STATES RT_INIT_STATE = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    private:

        void CreateRT(uint32 width, uint32 height);
        void ClearRT();

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
        DescriptorHandle m_rtv;
        DescriptorHandle m_srv;

        DXGI_FORMAT m_dxFormat = DXGI_FORMAT_UNKNOWN;
    };
}