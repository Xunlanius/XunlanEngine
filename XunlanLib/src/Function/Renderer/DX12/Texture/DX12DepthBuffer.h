#pragma once

#include "src/Function/Renderer/Abstract/Texture.h"
#include "../DX12Common.h"
#include "../DX12DescriptorHeap.h"

namespace Xunlan::DX12
{
    class DX12DepthBuffer final : public DepthBuffer
    {
    public:

        DX12DepthBuffer(uint32 width, uint32 height);
        virtual ~DX12DepthBuffer();

    public:

        ID3D12Resource* GetResource() const { return m_resource.Get(); }
        DescriptorHandle GetDSV() const { return m_dsv; }
        DescriptorHandle GetSRV() const { return m_srv; }
        DXGI_FORMAT GetDXFormat() const { return m_dxFormat; }
        virtual uint32 GetHeapIndex() const override { return m_srv.index; }

        virtual void Resize(uint32 width, uint32 height) override;

        static constexpr D3D12_RESOURCE_STATES DS_INIT_STATE = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

    private:

        void CreateDepth(uint32 width, uint32 height);
        void ClearDepth();

        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
        DescriptorHandle m_dsv;
        DescriptorHandle m_srv;

        DXGI_FORMAT m_dxFormat = DXGI_FORMAT_UNKNOWN;
    };
}