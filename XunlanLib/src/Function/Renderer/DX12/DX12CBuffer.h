#pragma once

#include "DX12Common.h"
#include "src/Function/Renderer/Abstract/CBuffer.h"

namespace Xunlan::DX12
{
    class DX12CBuffer final : public CBuffer
    {
    public:

        explicit DX12CBuffer(size_t size);
        virtual ~DX12CBuffer() override;

    public:

        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return m_buffer->GetGPUVirtualAddress(); }

    private:

        virtual void* GetData() const override { return m_dst; }

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
        void* m_dst = nullptr;
    };
}