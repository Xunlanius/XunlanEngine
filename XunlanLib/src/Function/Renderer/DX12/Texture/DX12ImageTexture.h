#pragma once

#include "src/Function/Renderer/Abstract/Texture.h"
#include "../DX12Common.h"
#include "../DX12DescriptorHeap.h"

namespace Xunlan::DX12
{
    class DX12ImageTexture final : public ImageTexture
    {
    public:

        DX12ImageTexture(
            CRef<RawTexture> rawTexture,
            Microsoft::WRL::ComPtr<ID3D12Resource> texture,
            const DescriptorHandle& handle)
            : ImageTexture(rawTexture), m_texture(texture), m_handle(handle) {}
        virtual ~DX12ImageTexture();

    public:

        static Ref<ImageTexture> Create(CRef<RawTexture> rawTexture);

        ID3D12Resource* GetTexture() const { return m_texture.Get(); }
        D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return m_handle.handleCPU; }
        virtual uint32 GetHeapIndex() const override { return m_handle.index; }

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
        DescriptorHandle m_handle;
    };
}